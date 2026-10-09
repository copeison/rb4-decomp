#include "render/shaders/RndShaderMgr.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cmath>
#include <cstdint>

#include "math/color/Color.h"
#include "math/vector/Vector3i.h"
#include "os/memory/MemMgr.h"
#include "utl/containers/Std.h"
#include "utl/text/Symbol.h"
#include "render/system/RndConfig.h"
#include "render/system/RndCapabilities.h"
#include "render/system/RndDevice.h"
#include "render/textures/render_data_format.h"
#include "render/textures/RndTextureArray1D.h"
#include "render/textures/RndPixelCanvas.h"
#include "render/textures/RndPixelData.h"
#include "utl/text/Str.h"
#include "render/shaders/RndShader.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/buffers/RndCShaderClearBuffer.h"
#include "render/buffers/RndCShaderCopyBuffer.h"
#include "render/debug/RndCShaderRenderTestCompute.h"
#include "render/debug/RndShaderDisplayShadingMode.h"
#include "render/debug/RndShaderDisplaySphereMap.h"
#include "render/debug/RndShaderDisplayTextureCube.h"
#include "render/debug/RndShaderRenderTestSimple.h"
#include "render/debug/RndShaderTestPattern.h"
#include "render/depth/RndCShaderCalcDepthRange.h"
#include "render/depth/RndCShaderLinearizeDepth.h"
#include "render/depth/RndShaderLinearizeDepth.h"
#include "render/distance_fields/RndCShaderSignedDistance.h"
#include "render/distance_fields/RndCShaderSignedDistanceClassify.h"
#include "render/lighting/ambient_occlusion/RndCShaderSSAOGen.h"
#include "render/lighting/volumetric/RndCShaderVScatAccumScattering.h"
#include "render/lighting/volumetric/RndCShaderVScatCalcDensityInscattering.h"
#include "render/lighting/volumetric/RndCShaderVScatDeferred.h"
#include "render/masking/RndShaderRefineSceneMask.h"
#include "render/masking/RndShaderStencilSceneMask.h"
#include "render/postprocessing/antialiasing/RndCShaderCMAAEdgeDetect.h"
#include "render/postprocessing/antialiasing/RndCShaderCMAAEdgePrune.h"
#include "render/postprocessing/antialiasing/RndCShaderCMAAFinalProcess.h"
#include "render/postprocessing/antialiasing/RndCShaderCMAAShapeFit.h"
#include "render/postprocessing/antialiasing/RndShaderFXAA.h"
#include "render/postprocessing/bloom/RndShaderBloom.h"
#include "render/postprocessing/blur/RndCShaderBlurClassify.h"
#include "render/postprocessing/blur/RndShaderBlur.h"
#include "render/postprocessing/depth_of_field/RndCShaderDOFDiscBlur.h"
#include "render/postprocessing/depth_of_field/RndShaderDOFSprite.h"
#include "render/postprocessing/downsample/RndShaderDownsample.h"
#include "render/postprocessing/output/RndShaderOutputConversion.h"
#include "render/video/RndShaderBinkConvert.h"
#include "render/shaders/RndShaderBasic.h"
#include "render/shaders/RndShaderError.h"

using namespace rb4;

namespace {

constexpr std::ptrdiff_t kSecondaryShaderDirtyOffset = -395;
constexpr std::size_t kCurrentPlatformConfigIndex = 7;
constexpr std::uint32_t kAsyncComputeFeature = 0x10;

struct RenderManagedObjectDispatch {
    void* reserved_0;
    void (*release_dynamic)(void* object);
};

struct RenderManagedObject {
    RenderManagedObjectDispatch* dispatch;
};

RndShaderLink* create_list_sentinel() {
    auto* node = new RndShaderLink;
    node->mNext = node;
    node->mPrev = node;
    return node;
}

void release_list_sentinel(RndShaderLink*& node) {
    if (node == nullptr) {
        return;
    }

    node->mNext->mPrev = node->mPrev;
    node->mPrev->mNext = node->mNext;
    delete node;
    node = nullptr;
}

void release_dynamic_resource(void*& storage) {
    auto* resource = static_cast<RenderManagedObject*>(storage);
    if (resource != nullptr) {
        resource->dispatch->release_dynamic(resource);
        storage = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x645F20.
float sample_function_table(
    std::uint32_t function_index,
    float input) {
    float output = 0.0F;
    switch (function_index) {
    case 0:
        output = 1.0F - input;
        break;
    case 1: {
        const auto denominator = 1.25F * input + 0.25F;
        output = 0.0642857179F / (denominator * denominator) -
            0.0285714306F;
        break;
    }
    case 2: {
        constexpr float kMinimum = 0.006737947F;
        const auto exponential = 1.0F / std::exp(5.0F * input);
        output = (exponential - kMinimum) / (1.0F - kMinimum);
        break;
    }
    case 3: {
        constexpr float kMinimum = 0.006692851F;
        constexpr float kMaximum = 0.993307173F;
        const auto sigmoid =
            1.0F / (std::exp((input - 0.5F) * 10.0F) + 1.0F);
        output = (sigmoid - kMinimum) / (kMaximum - kMinimum);
        break;
    }
    default:
        break;
    }
    return std::max(0.0F, std::min(1.0F, output));
}

struct ShaderConstantDefinition {
    const char* name;
    std::int32_t value;
};

template <std::size_t Count>
void add_shader_constant_group(
    RndShaderFixedDefines& registry,
    const char* comment,
    const ShaderConstantDefinition (&definitions)[Count]) {
    registry.AddComment(comment);
    for (const auto& definition : definitions) {
        registry.Add(Symbol(definition.name), definition.value);
    }
}

template <typename T>
T* NewShader() {
    auto* shader = new T;
    shader->_Register();
    return shader;
}

template <typename T>
void Release(T*& object) {
    if (object != nullptr) {
        delete object;
        object = nullptr;
    }
}

bool supports_async_compute() {
    const auto& platform = TheRndDevice()->mCapabilities[kCurrentPlatformConfigIndex];
    return (platform.mFeatureFlags & kAsyncComputeFeature) != 0;
}

}  // namespace

// Reconstructed from eboot.elf at 0x63F180. Constant offsets start at -1 and
// the configurations and shaders at null.
RndShaderMgr::RndShaderMgr()
    : mGlobalDefInfos{},
      mPreInitialized(false),
      mInitialized(false),
      mSceneCBuffer(nullptr),
      mTime(-1),
      mSmoothnessDecay(-1),
      mSGraphTransInfos(-1),
      mSceneGlobalFloats(-1),
      mSceneGlobalColors(-1),
      mTiledLightingParams(-1),
      mFogParams(-1),
      mVolumetricParams0(-1),
      mVolumetricParams1(-1),
      mRenderTargetCBuffer(nullptr),
      mTargetDimensions(-1),
      mCameraCBuffer(nullptr),
      mCameraNearFarParams(-1),
      mCameraMiscParams(-1),
      mCameraViewExtents(-1),
      mUnknown216(-1),
      mUnknown224(-1),
      mCameraRTSlicedData(-1),
      mClipPlanesCBuffer(nullptr),
      mClipPlanes(-1),
      mSkeletonCBuffer(nullptr),
      mSkeletonBoneXfms(-1),
      mMiscDrawStateCBuffer(nullptr),
      mEnvironIndex(-1),
      mSolidColor(-1),
      mOcclusionQueryCBuffer(nullptr),
      mOcclusionQueryCoverageParams(-1),
      mDebugCBuffer(nullptr),
      mDebugModes(-1),
      mDebugColor(-1),
      mBatchInfo(-1),
      mPreviewNodeIndex(-1),
      mTransientCBuffers{},
      mFixedDefines(nullptr),
      mDefinesGroup(nullptr),
      mFunctionTable(nullptr),
      mErrorShader(nullptr),
      mBasicShader(nullptr),
      mBinkConvertShader(nullptr),
      mBloomShader(nullptr),
      mBlurShader(nullptr),
      mFXAAShader(nullptr),
      mDOFSpriteShader(nullptr),
      mDisplayShadingModeShader(nullptr),
      mDisplaySphereMapShader(nullptr),
      mDisplayTextureCubeShader(nullptr),
      mDownsampleShader(nullptr),
      mLinearizeDepthShader(nullptr),
      mOutputConversionShader(nullptr),
      mRefineSceneMaskShader(nullptr),
      mStencilSceneMaskShader(nullptr),
      mTestPatternShader(nullptr),
      mUnknown528(nullptr),
      mBlurClassifyCShader(nullptr),
      mCalcDepthRangeCShader(nullptr),
      mClearBufferCShader(nullptr),
      mCopyBufferCShader(nullptr),
      mDOFDiscBlurCShader(nullptr),
      mVScatCalcDensityInscatteringCShader(nullptr),
      mVScatAccumScatteringCShader(nullptr),
      mVScatDeferredCShader(nullptr),
      mSSAOGenCShader(nullptr),
      mCMAAEdgeDetectCShader(nullptr),
      mCMAAEdgePruneCShader(nullptr),
      mCMAAShapeFitCShader(nullptr),
      mCMAAFinalProcessCShader(nullptr),
      mLinearizeDepthCShader(nullptr),
      mSignedDistanceCShader(nullptr),
      mSignedDistanceClassifyCShader(nullptr),
      mRenderTestSimpleShader(nullptr),
      mRenderTestComputeCShader(nullptr),
      mFixedDefinesChecksum(0) {
    mIncludeChecksums = new RndShaderIncludeChecksums{};
    mShaders = create_list_sentinel();
    mShaderGraphs = create_list_sentinel();
}

// Reconstructed from eboot.elf at 0x640BF0.
void RndShaderMgr::_InitDefinesGroups() {
    auto* parameters = new RndShaderDefinesGroup;
    mDefinesGroup = parameters;

    struct BindingDefinition {
        const char* name;
        std::uint32_t first_value;
        std::uint32_t last_value;
        std::size_t registry_index;
    };
    constexpr BindingDefinition kBindings[] = {
        {"HX_BT709_TO_BT2020", 0, 2, 0},
        {"HX_NUM_RT_SLICES", 0, 7, 0},
        {"HX_SHADING_MODE", 0, 19, 0},
        {"HX_GEO_TYPE", 0, 2, 1},
    };

    for (std::size_t index = 0; index < 4; ++index) {
        const auto& definition = kBindings[index];
        const Symbol name(definition.name);
        mGlobalDefInfos[index] =
            parameters->mDefines[definition.registry_index].Add(
                name,
                static_cast<int>(definition.first_value),
                static_cast<int>(definition.last_value));
    }
}

// Reconstructed from eboot.elf at 0x63F920.
void RndShaderMgr::_InitFixedDefines() {
    auto* registry = new RndShaderFixedDefines;
    mFixedDefines = registry;

    constexpr ShaderConstantDefinition kMiscConstants[] = {
        {"HX_MAX_BONES", 256},
        {"HX_MAX_CLIP_PLANES", 4},
        {"HX_MAX_TEXARRAY_SIZE", 2048},
        {"HX_VIEWPROJ_OFFSET", 0},
        {"HX_CAMXFM_OFFSET", 4},
        {"HX_CAMXFMINV_OFFSET", 7},
        {"HX_CAM_SIZE_PER_RT_SLICE", 10},
    };
    constexpr ShaderConstantDefinition kProgramTypes[] = {
        {"HX_PROGRAM_TYPE_VERTEX", 0},
        {"HX_PROGRAM_TYPE_HULL", 1},
        {"HX_PROGRAM_TYPE_DOMAIN", 2},
        {"HX_PROGRAM_TYPE_GEOMETRY", 3},
        {"HX_PROGRAM_TYPE_PIXEL", 4},
        {"HX_PROGRAM_TYPE_COMPUTE", 5},
    };
    constexpr ShaderConstantDefinition kGeometryTypes[] = {
        {"HX_GEO_UNSKINNED_MESH", 0},
        {"HX_GEO_SKINNED_MESH", 1},
    };
    constexpr ShaderConstantDefinition kStereoEyes[] = {
        {"HX_STEREO_EYE_LEFT", 0},
        {"HX_STEREO_EYE_RIGHT", 1},
    };
    constexpr ShaderConstantDefinition kBillboardTypes[] = {
        {"HX_BILLBOARD_NONE", 0},
        {"HX_BILLBOARD_CAMERA_XYZ", 1},
        {"HX_BILLBOARD_CAMERA_XY", 2},
        {"HX_BILLBOARD_CAMERA_KEEPZ", 3},
    };
    constexpr ShaderConstantDefinition kShadingModes[] = {
        {"HX_SHADING_MODE_STANDARD", 0},
        {"HX_SHADING_MODE_STANDARD_FOG", 1},
        {"HX_SHADING_MODE_STANDARD_VSCAT", 2},
        {"HX_SHADING_MODE_DEPTH_ONLY", 3},
        {"HX_SHADING_MODE_SOLID_COLOR", 4},
        {"HX_SHADING_MODE_DEFERRED_NORMALS_AND_ZFILL", 5},
        {"HX_SHADING_MODE_DEFERRED_UNLIT", 6},
        {"HX_SHADING_MODE_DEFERRED_UNLIT_AND_ZFILL", 7},
        {"HX_SHADING_MODE_DEFERRED_LIT", 8},
        {"HX_SHADING_MODE_DEFERRED_LIT_AND_ZFILL", 9},
        {"HX_SHADING_MODE_DEFERRED_LIT_EMISSIVE", 10},
        {"HX_SHADING_MODE_DEFERRED_LIT_EMISSIVE_AND_ZFILL", 11},
        {"HX_SHADING_MODE_DEFERRED_DECAL_TRANSPARENT", 12},
        {"HX_SHADING_MODE_FWD_LIT_OPAQUE", 13},
        {"HX_SHADING_MODE_SCENE_MASK", 14},
        {"HX_SHADING_MODE_IMPOSTOR_MAPS", 15},
        {"HX_SHADING_MODE_FAST_CHEAP", 16},
        {"HX_SHADING_MODE_WIREFRAME", 17},
        {"HX_SHADING_MODE_DEBUG_MISC", 18},
        {"HX_SHADING_MODE_FIRST_DEBUG", 16},
    };
    constexpr ShaderConstantDefinition kShaderDebugModes[] = {
        {"HX_SHADER_DEBUG_MODE_UNLIT", 2},
        {"HX_SHADER_DEBUG_MODE_OVERDRAW", 3},
        {"HX_SHADER_DEBUG_MODE_BATCHES", 4},
        {"HX_SHADER_DEBUG_MODE_BATCH_SIZE", 5},
        {"HX_SHADER_DEBUG_MODE_LIGHTING_ONLY", 6},
        {"HX_SHADER_DEBUG_MODE_LIT_DIFFUSE", 7},
        {"HX_SHADER_DEBUG_MODE_LIT_SPECULAR", 8},
        {"HX_SHADER_DEBUG_MODE_LIT_DIRECT", 9},
        {"HX_SHADER_DEBUG_MODE_LIT_DIRECT_DIFFUSE", 10},
        {"HX_SHADER_DEBUG_MODE_LIT_DIRECT_SPECULAR", 11},
        {"HX_SHADER_DEBUG_MODE_LIT_INDIRECT", 12},
        {"HX_SHADER_DEBUG_MODE_LIT_INDIRECT_DIFFUSE", 13},
        {"HX_SHADER_DEBUG_MODE_LIT_INDIRECT_SPECULAR", 14},
        {"HX_SHADER_DEBUG_MODE_NO_NEGLIGHTS", 15},
        {"HX_SHADER_DEBUG_MODE_LIGHTING_OVERDRAW", 16},
        {"HX_SHADER_DEBUG_MODE_LIGHT_PROBE_OVERDRAW", 17},
        {"HX_SHADER_DEBUG_MODE_VERTEX_COLOR", 18},
        {"HX_SHADER_DEBUG_MODE_VERTEX_ALPHA", 19},
        {"HX_SHADER_DEBUG_MODE_VERTEX_NORMAL", 20},
        {"HX_SHADER_DEBUG_MODE_VERTEX_TANGENT", 21},
        {"HX_SHADER_DEBUG_MODE_VERTEX_BITANGENT", 22},
        {"HX_SHADER_DEBUG_MODE_PIXEL_NORMAL", 23},
        {"HX_SHADER_DEBUG_MODE_UV0", 24},
        {"HX_SHADER_DEBUG_MODE_UV1", 25},
        {"HX_SHADER_DEBUG_MODE_MATERIAL_LIGHTING_PATH", 26},
        {"HX_SHADER_DEBUG_MODE_MATERIAL_COLOR", 27},
        {"HX_SHADER_DEBUG_MODE_MATERIAL_ALPHA", 28},
        {"HX_SHADER_DEBUG_MODE_MATERIAL_SMOOTHNESS", 29},
        {"HX_SHADER_DEBUG_MODE_MATERIAL_METALLICITY", 30},
        {"HX_SHADER_DEBUG_MODE_MATERIAL_EMISSIVE", 31},
    };
    constexpr ShaderConstantDefinition kCubeFaces[] = {
        {"HX_CUBE_FACE_RIGHT", 0},
        {"HX_CUBE_FACE_LEFT", 1},
        {"HX_CUBE_FACE_TOP", 2},
        {"HX_CUBE_FACE_BOTTOM", 3},
        {"HX_CUBE_FACE_FRONT", 4},
        {"HX_CUBE_FACE_BACK", 5},
    };
    constexpr ShaderConstantDefinition kFrustumPlanes[] = {
        {"HX_FRUSTUM_PLANE_FRONT", 0},
        {"HX_FRUSTUM_PLANE_BACK", 1},
        {"HX_FRUSTUM_PLANE_LEFT", 2},
        {"HX_FRUSTUM_PLANE_RIGHT", 3},
        {"HX_FRUSTUM_PLANE_TOP", 4},
        {"HX_FRUSTUM_PLANE_BOTTOM", 5},
    };
    constexpr ShaderConstantDefinition kFrustumCorners[] = {
        {"HX_FRUSTUM_CORNER_FRONT_LEFT_TOP", 0},
        {"HX_FRUSTUM_CORNER_FRONT_RIGHT_TOP", 1},
        {"HX_FRUSTUM_CORNER_FRONT_LEFT_BOTTOM", 2},
        {"HX_FRUSTUM_CORNER_FRONT_RIGHT_BOTTOM", 3},
        {"HX_FRUSTUM_CORNER_BACK_LEFT_TOP", 4},
        {"HX_FRUSTUM_CORNER_BACK_RIGHT_TOP", 5},
        {"HX_FRUSTUM_CORNER_BACK_LEFT_BOTTOM", 6},
        {"HX_FRUSTUM_CORNER_BACK_RIGHT_BOTTOM", 7},
    };
    constexpr ShaderConstantDefinition kLightTypes[] = {
        {"HX_LIGHT_TYPE_POINT", 0},
        {"HX_LIGHT_TYPE_SPOT", 1},
        {"HX_LIGHT_TYPE_DIRECTIONAL", 2},
        {"HX_NUM_LIGHT_TYPES", 3},
        {"HX_NUM_BOUNDED_LIGHT_TYPES", 2},
        {"HX_ILLUM_POSITIVE", 0},
        {"HX_ILLUM_POSITIVE_DIFFUSE", 1},
        {"HX_ILLUM_POSITIVE_SPECULAR", 2},
        {"HX_ILLUM_NEGATIVE", 3},
        {"HX_LIGHT_COOKIE_STATIC", 0},
        {"HX_LIGHT_COOKIE_RENDERED", 1},
    };
    constexpr ShaderConstantDefinition kLightingPaths[] = {
        {"HX_LIGHTING_PATH_UNLIT", 0},
        {"HX_LIGHTING_PATH_DEFERRED_LIT", 1},
        {"HX_LIGHTING_PATH_DEFERRED_EMISSIVE", 2},
        {"HX_LIGHTING_PATH_FWD_LIT_STANDARD", 3},
        {"HX_LIGHTING_PATH_FWD_LIT_SUBSURFACE", 4},
        {"HX_LIGHTING_PATH_FWD_LIT_SKIN", 5},
        {"HX_LIGHTING_PATH_FWD_LIT_HAIR", 6},
    };
    constexpr ShaderConstantDefinition kCommonStructSizes[] = {
        {"HX_SIZEOF_PLANE", 16},
        {"HX_SIZEOF_SPHERE", 16},
        {"HX_SIZEOF_CSLIGHTIDRANGE", 32},
        {"HX_SIZEOF_CSLIGHTPOINT", 208},
        {"HX_SIZEOF_CSLIGHTSPOT", 352},
        {"HX_SIZEOF_CSLIGHTDIRECTIONAL", 112},
        {"HX_SIZEOF_CSLIGHTPROBE", 96},
    };

    add_shader_constant_group(*registry, "misc constants", kMiscConstants);
    add_shader_constant_group(*registry, "program types", kProgramTypes);
    add_shader_constant_group(*registry, "geometry types", kGeometryTypes);
    add_shader_constant_group(*registry, "stereo eyes", kStereoEyes);
    add_shader_constant_group(
        *registry, "billboarding types", kBillboardTypes);
    add_shader_constant_group(*registry, "shading modes", kShadingModes);
    add_shader_constant_group(
        *registry, "shader debug modes", kShaderDebugModes);
    add_shader_constant_group(*registry, "cube faces", kCubeFaces);
    add_shader_constant_group(*registry, "frustum planes", kFrustumPlanes);
    add_shader_constant_group(*registry, "frustum corners", kFrustumCorners);
    add_shader_constant_group(*registry, "light types", kLightTypes);
    add_shader_constant_group(*registry, "lighting paths", kLightingPaths);
    add_shader_constant_group(
        *registry, "common struct sizes", kCommonStructSizes);
}

// Reconstructed from eboot.elf at 0x640D60.
void RndShaderMgr::_CreateCBufferConfigs() {

    mSceneCBuffer = new RndShaderCBufferConfig("Scene", 0, 9, 5);
    auto& scene = *mSceneCBuffer;
    mTime = scene.AddConstant(kShaderNumericFloat4, "gTime");
    mSmoothnessDecay = scene.AddConstant(kShaderNumericFloat, "gSmoothnessDecay");
    mSGraphTransInfos = scene.AddConstantArray(kShaderNumericFloat3x4, 4, "gSGraphTransInfos");
    mSceneGlobalFloats = scene.AddConstantArray(kShaderNumericFloat4, 1, "gSceneGlobalFloats");
    mSceneGlobalColors = scene.AddConstantArray(kShaderNumericFloat4, 4, "gSceneGlobalColors");
    mTiledLightingParams = scene.AddConstant(kShaderNumericFloat3, "gTiledLightingParams");
    mFogParams = scene.AddConstant(kShaderNumericFloat3, "gFogParams");
    mVolumetricParams0 = scene.AddConstant(kShaderNumericFloat3, "gVolumetricParams0");
    mVolumetricParams1 = scene.AddConstant(kShaderNumericFloat2, "gVolumetricParams1");

    mRenderTargetCBuffer = new RndShaderCBufferConfig("RenderTarget", 1, 28, 80);
    mTargetDimensions = mRenderTargetCBuffer->AddConstant(kShaderNumericFloat2, "gTargetDimensions");

    mCameraCBuffer = new RndShaderCBufferConfig("Camera", 2, 29, 40);
    auto& camera = *mCameraCBuffer;
    mCameraNearFarParams = camera.AddConstant(kShaderNumericFloat4, "gCameraNearFarParams");
    mCameraMiscParams = camera.AddConstant(kShaderNumericFloat4, "gCameraMiscParams");
    mCameraViewExtents = camera.AddConstantArray(kShaderNumericFloat4, 4, "gCameraViewExtents");
    mCameraRTSlicedData =
        camera.AddRTSlicedConstantArray(kShaderNumericFloat4, 10, "gCameraRTSlicedData");

    mClipPlanesCBuffer = new RndShaderCBufferConfig("ClipPlanes", 3, 1, 10);
    mClipPlanes = mClipPlanesCBuffer->AddConstantArray(kShaderNumericFloat4, 4, "gClipPlanes");

    mSkeletonCBuffer = new RndShaderCBufferConfig("Skeleton", 4, 1, 1);
    mSkeletonBoneXfms =
        mSkeletonCBuffer->AddConstantArray(kShaderNumericFloat3x4, 256, "gSkeletonBoneXfms");

    mMiscDrawStateCBuffer = new RndShaderCBufferConfig("MiscDrawState", 5, 8, 10);
    mEnvironIndex = mMiscDrawStateCBuffer->AddConstant(kShaderNumericFloat, "gEnvironIndex");
    mSolidColor = mMiscDrawStateCBuffer->AddConstant(kShaderNumericFloat4, "gSolidColor");

    mOcclusionQueryCBuffer = new RndShaderCBufferConfig("OcclusionQuery", 6, 9, 10);
    mOcclusionQueryCoverageParams = mOcclusionQueryCBuffer->AddConstant(kShaderNumericFloat2, "gOcclusionQueryCoverageParams");

    mDebugCBuffer = new RndShaderCBufferConfig("Debug", 7, 24, 10);
    auto& debug = *mDebugCBuffer;
    mDebugModes = debug.AddConstant(kShaderNumericFloat2, "gDebugModes");
    mDebugColor = debug.AddConstant(kShaderNumericFloat4, "gDebugColor");
    mBatchInfo = debug.AddConstant(kShaderNumericFloat2, "gBatchInfo");
    mPreviewNodeIndex = debug.AddConstant(kShaderNumericFloat, "gPreviewNodeIndex");

    constexpr std::uint64_t kTransientCounts[] = {16, 32, 64};
    for (std::size_t index = 0; index < 3; ++index) {
        auto*& transient = mTransientCBuffers[index];
        transient = new RndShaderCBufferConfig("Transient", 8, 29, 10);
        transient->AddConstantArray(kShaderNumericFloat4, kTransientCounts[index], "gTransientData");
    }

    constexpr std::uint32_t kFnv1aOffsetBasis = 0x811C9DC5U;
    std::uint32_t source_hash = kFnv1aOffsetBasis;
    mSceneCBuffer->PrintCode(source_hash);
    mFixedDefines->PrintCode(source_hash);

    RndShaderCBufferConfig* remaining_blocks[] = {
        mRenderTargetCBuffer,
        mCameraCBuffer,
        mClipPlanesCBuffer,
        mSkeletonCBuffer,
        mMiscDrawStateCBuffer,
        mDebugCBuffer,
    };
    for (const auto* block : remaining_blocks) {
        block->PrintCode(source_hash);
    }
    mFixedDefinesChecksum =
        (mFixedDefinesChecksum & 0xFFFFFFFF00000000ULL) |
        source_hash;
}

// Reconstructed from eboot.elf at 0x63F400.
void RndShaderMgr::PreInit() {
    mPreInitialized = 1;
    _InitFixedDefines();
    _InitDefinesGroups();
    _CreateCBufferConfigs();

    mErrorShader = NewShader<RndShaderError>();
    mBasicShader = NewShader<RndShaderBasic>();
    mBinkConvertShader = NewShader<RndShaderBinkConvert>();
    mBloomShader = NewShader<RndShaderBloom>();
    mBlurShader = NewShader<RndShaderBlur>();
    mFXAAShader = NewShader<RndShaderFXAA>();
    mDisplayShadingModeShader = NewShader<RndShaderDisplayShadingMode>();
    mDisplaySphereMapShader = NewShader<RndShaderDisplaySphereMap>();
    mDisplayTextureCubeShader = NewShader<RndShaderDisplayTextureCube>();
    mDownsampleShader = NewShader<RndShaderDownsample>();
    mLinearizeDepthShader = NewShader<RndShaderLinearizeDepth>();
    mOutputConversionShader = NewShader<RndShaderOutputConversion>();
    mRefineSceneMaskShader = NewShader<RndShaderRefineSceneMask>();
    mStencilSceneMaskShader = NewShader<RndShaderStencilSceneMask>();
    mTestPatternShader = NewShader<RndShaderTestPattern>();

    if (supports_async_compute()) {
        mBlurClassifyCShader = NewShader<RndCShaderBlurClassify>();
        mCalcDepthRangeCShader = NewShader<RndCShaderCalcDepthRange>();
        mClearBufferCShader = NewShader<RndCShaderClearBuffer>();
        mCopyBufferCShader = NewShader<RndCShaderCopyBuffer>();
        mDOFDiscBlurCShader = NewShader<RndCShaderDOFDiscBlur>();
        mDOFSpriteShader = NewShader<RndShaderDOFSprite>();
        mVScatCalcDensityInscatteringCShader = NewShader<RndCShaderVScatCalcDensityInscattering>();
        mVScatAccumScatteringCShader = NewShader<RndCShaderVScatAccumScattering>();
        mVScatDeferredCShader = NewShader<RndCShaderVScatDeferred>();
        mSSAOGenCShader = NewShader<RndCShaderSSAOGen>();
        mCMAAEdgeDetectCShader = NewShader<RndCShaderCMAAEdgeDetect>();
        mCMAAEdgePruneCShader = NewShader<RndCShaderCMAAEdgePrune>();
        mCMAAShapeFitCShader = NewShader<RndCShaderCMAAShapeFit>();
        mCMAAFinalProcessCShader = NewShader<RndCShaderCMAAFinalProcess>();
        mLinearizeDepthCShader = NewShader<RndCShaderLinearizeDepth>();
        mSignedDistanceCShader = NewShader<RndCShaderSignedDistance>();
        mSignedDistanceClassifyCShader = NewShader<RndCShaderSignedDistanceClassify>();
    }

    mRenderTestSimpleShader = NewShader<RndShaderRenderTestSimple>();
    if (supports_async_compute()) {
        mRenderTestComputeCShader = NewShader<RndCShaderRenderTestCompute>();
    }
}

// Reconstructed from eboot.elf at 0x63F350.
RndShaderMgr::~RndShaderMgr() {
    release_list_sentinel(mShaders);
    release_list_sentinel(mShaderGraphs);

    if (mIncludeChecksums != nullptr) {
        auto& array = mIncludeChecksums->mChecksums;
        if (array.mBegin != nullptr) {
            const auto byte_count = static_cast<std::size_t>(
                reinterpret_cast<std::uint8_t*>(array.mCapacity) -
                reinterpret_cast<std::uint8_t*>(array.mBegin));
            HmxAllocator::gStlAllocator.deallocate(array.mBegin, byte_count);
        }
        MemFree(mIncludeChecksums);
        mIncludeChecksums = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x641370.
void RndShaderMgr::Init() {
    mInitialized = 1;
    for (auto* node = mShaders->mNext;
         node != mShaders;
         node = node->mNext) {
        RndShader::FromLink(node)->Init();
    }

    constexpr std::uint32_t kFunctionCount = 4;
    constexpr std::uint32_t kSampleCount = 128;
    constexpr float kSampleStep = 1.0F / 127.0F;
    constexpr RenderDataFormatDescriptor kFunctionTableFormat{
        32,
        10,
        2,
        1,
        -1,
    };

    RndTextureArray1D::Description descriptor;
    descriptor.mName = "function_table";
    descriptor.mFormat.mWrapMode = static_cast<std::uint32_t>(
        TextureDefaultWrapMode(6));
    descriptor.mFormat.mFilterMode = static_cast<std::uint32_t>(
        TextureDefaultFilterMode(6));

    std::array<RndPixelData, kFunctionCount> mip_chains;
    descriptor.mPixels = {
        mip_chains.data(),
        mip_chains.data() + mip_chains.size(),
        mip_chains.data() + mip_chains.size(),
    };

    const auto data_format =
        render_data_format_resolve(kFunctionTableFormat, 7);
    const Vector3i extent{static_cast<int>(kSampleCount), 1, 1};
    std::array<Hmx::Color, kSampleCount> pixels;
    for (std::uint32_t function_index = 0;
         function_index < kFunctionCount;
         ++function_index) {
        for (std::uint32_t sample_index = 0;
             sample_index < kSampleCount;
             ++sample_index) {
            const auto sample = sample_function_table(
                function_index,
                static_cast<float>(sample_index) * kSampleStep);
            pixels[sample_index] = {sample, sample, sample, sample};
        }

        auto& mip = mip_chains[function_index];
        mip.Create(extent, data_format, nullptr);
        const RndPixelCanvas image{
            nullptr,
            static_cast<int>(kSampleCount),
            1,
            1,
            0,
            pixels.data(),
            nullptr,
        };
        mip.ConvertFrom(image);
    }

    mFunctionTable =
        RndTextureArray1D::New(descriptor, nullptr);
}

// Reconstructed from eboot.elf at 0x641740.
void RndShaderMgr::Terminate() {
    RndShaderCBufferConfig** constant_blocks[] = {
        &mSceneCBuffer,
        &mRenderTargetCBuffer,
        &mCameraCBuffer,
        &mClipPlanesCBuffer,
        &mSkeletonCBuffer,
        &mMiscDrawStateCBuffer,
        &mOcclusionQueryCBuffer,
        &mDebugCBuffer,
    };
    for (auto** block : constant_blocks) {
        Release(*block);
    }
    for (auto*& block : mTransientCBuffers) {
        Release(block);
    }
    Release(mFixedDefines);
    Release(mDefinesGroup);

    Release(mFunctionTable);
    Release(mErrorShader);
    Release(mBasicShader);
    Release(mBinkConvertShader);
    Release(mBloomShader);
    Release(mBlurShader);
    Release(mFXAAShader);
    Release(mDOFSpriteShader);
    Release(mDisplayShadingModeShader);
    Release(mDisplaySphereMapShader);
    Release(mDisplayTextureCubeShader);
    Release(mDownsampleShader);
    Release(mLinearizeDepthShader);
    Release(mOutputConversionShader);
    Release(mRefineSceneMaskShader);
    Release(mStencilSceneMaskShader);
    Release(mTestPatternShader);
    release_dynamic_resource(mUnknown528);
    Release(mBlurClassifyCShader);
    Release(mCalcDepthRangeCShader);
    Release(mClearBufferCShader);
    Release(mCopyBufferCShader);
    Release(mDOFDiscBlurCShader);
    Release(mVScatCalcDensityInscatteringCShader);
    Release(mVScatAccumScatteringCShader);
    Release(mVScatDeferredCShader);
    Release(mSSAOGenCShader);
    Release(mCMAAEdgeDetectCShader);
    Release(mCMAAEdgePruneCShader);
    Release(mCMAAShapeFitCShader);
    Release(mCMAAFinalProcessCShader);
    Release(mLinearizeDepthCShader);
    Release(mSignedDistanceCShader);
    Release(mSignedDistanceClassifyCShader);
    Release(mRenderTestSimpleShader);
    Release(mRenderTestComputeCShader);
}

// Reconstructed from eboot.elf at 0x641F30, with the primary-resource clear
// helper at 0x6388C0 and compiled-array clear at 0x63B210.
void RndShaderMgr::ReloadAll() {
    for (auto* node = mShaders->mNext;
         node != mShaders;
         node = node->mNext) {
        RndShader::FromLink(node)->Reload();
    }

    const auto& settings = *TheRndDevice()->mSettings;
    if (settings.mMaxPartialFramerateScenes != 0) {
        return;
    }

    for (auto* node = mShaderGraphs->mNext;
         node != mShaderGraphs;
         node = node->mNext) {
        auto* bytes = reinterpret_cast<std::uint8_t*>(node);
        bytes[kSecondaryShaderDirtyOffset] = true;
    }
}
