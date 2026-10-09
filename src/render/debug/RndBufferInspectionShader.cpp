#include "render/debug/RndBufferInspectionShader.h"

#include <cmath>
#include <limits>

#include "render/buffers/RndComputeBuffer.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/defaults/RndDefaults.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTexture1D.h"
#include "render/textures/RndTexture2D.h"
#include "render/textures/RndTexture3D.h"
#include "render/textures/RndTextureArray1D.h"
#include "render/textures/RndTextureArray2D.h"
#include "render/textures/RndTextureArrayCube.h"
#include "render/textures/RndTextureBase.h"
#include "render/textures/RndTextureCube.h"

namespace {

// Rounds half away from zero, saturating at the int range. Name not in the
// reference map.
int RoundToInt(float value) {
    if (value > 0.0F) {
        value += 0.5F;
        return value < 2147483648.0F ? static_cast<int>(value)
                                     : std::numeric_limits<int>::max();
    }
    value -= 0.5F;
    return value > -2147483648.0F ? static_cast<int>(value)
                                  : std::numeric_limits<int>::min();
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B5FA0.
RndBufferInspectionShader::RndBufferInspectionShader()
    : mTexMode{},
      mBufferDisplayMode(-1),
      mAuxDims(-1),
      mTextureArrayParams(-1),
      mTint(-1),
      mDiscardPadPixels(-1),
      mBufferInspectionDepthFrac(-1),
      mStencilParams(-1),
      mLightTileCounts(-1),
      mCBufferSize(0),
      mTexture1D(-1),
      mTexture2D(-1),
      mTexture3D(-1),
      mTextureArray1D(-1),
      mTextureArray2D(-1),
      mTextureCube(-1),
      mTextureArrayCube(-1),
      mTexture2DRTSliced(-1),
      mTextureArray2DRTSliced(-1),
      mStencilBuffer(-1),
      mTexture2DNoSampler(-1),
      mComputeBuffer2D(-1),
      mLightIds(-1),
      mLightIdRanges(-1) {}

// Reconstructed from eboot.elf at 0x6B6020.
RndBufferInspectionShader::~RndBufferInspectionShader() {}

// Reconstructed from eboot.elf at 0x6B7280.
const char* RndBufferInspectionShader::_GetClassNameImpl() const {
    return "RndBufferInspectionShader";
}

// Reconstructed from eboot.elf at 0x6B6850.
const char* RndBufferInspectionShader::_GetShaderFilePath() const {
    return "../../system/data/shaders/BufferInspection.hlsl";
}

// Reconstructed from eboot.elf at 0x6B6860. The tile sizes come from the
// device's settings, which must exist.
void RndBufferInspectionShader::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    const RndConfig* settings = TheRndDevice()->mSettings;
    fixedDefines.Add(Symbol("HX_LIGHT_TILE_SIZE"), static_cast<int>(settings->mLightTileSize));
    fixedDefines.Add(
        Symbol("HX_LIGHT_TILE_DEPTH_SLICES"),
        static_cast<int>(settings->mLightTileDepthSlices));

    mTexMode = defines.GetDefines(kShaderProgramPixel).Add(Symbol("HX_BUFFER_TEXMODE"), 0, 24);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_1D"), 0);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_2D"), 1);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_3D"), 2);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_ARRAY1D_ELEM"), 3);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_ARRAY1D_ALL"), 4);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_ARRAY2D_ELEM"), 5);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_ARRAY2D_ALL"), 6);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_CUBE"), 7);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_ARRAYCUBE_ALL"), 8);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_2D_NOSAMPLER"), 9);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_2D_RTSLICED"), 10);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_ARRAY2D_RTSLICED"), 11);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_DEPTH"), 12);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_DEPTH_RTSLICED"), 13);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_DEPTH_ARRAY_ELEM"), 14);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_STENCIL"), 15);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_LIGHT_TILE_COORD"), 16);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_TILED_LIGHTS_INTERP_DIFFUSE"), 17);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_TILED_LIGHTS_INTERP_SPECULAR"), 18);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_COMPUTE_2D_FLOAT4"), 19);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_TILED_LIGHTING_OVERDRAW"), 20);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_DEPTH_TILED_LIGHTING_OVERDRAW"), 21);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_BOTHEYES_TILED_LIGHTING_OVERDRAW"), 22);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_BOTHEYES_DEPTH_TILED_LIGHTING_OVERDRAW"), 23);

    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_RGBA"), 0);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_RGBX"), 1);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_RED"), 2);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_GREEN"), 3);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_BLUE"), 4);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_ALPHA"), 5);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_NORMAL_MAP"), 6);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_DEPTH"), 7);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_STENCIL"), 8);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_GBUFFER_PIXELNORMAL"), 9);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_GBUFFER_VERTEXNORMAL"), 10);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_FUNCTIONTABLE"), 11);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_TILED_LIGHTS_OVERDRAW"), 12);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_TILED_LIGHT_PROBES_OVERDRAW"), 13);

    mBufferDisplayMode = cbuffer.AddConstant(kShaderNumericFloat, "gBufferDisplayMode");
    mAuxDims = cbuffer.AddConstant(kShaderNumericFloat2, "gAuxDims");
    mTextureArrayParams = cbuffer.AddConstant(kShaderNumericFloat3, "gTextureArrayParams");
    mTint = cbuffer.AddConstant(kShaderNumericFloat4, "gTint");
    mDiscardPadPixels = cbuffer.AddConstant(kShaderNumericFloat, "gDiscardPadPixels");
    mBufferInspectionDepthFrac =
        cbuffer.AddConstant(kShaderNumericFloat, "gBufferInspectionDepthFrac");
    mStencilParams = cbuffer.AddConstant(kShaderNumericFloat2, "gStencilParams");
    mLightTileCounts = cbuffer.AddConstant(kShaderNumericFloat2, "gLightTileCounts");
    mMaxOverdraw = cbuffer.AddConstant(kShaderNumericFloat2, "gMaxOverdraw");
    mCBufferSize = cbuffer.mSize;

    mTexture1D = resources.AddTexture(
        "gTexture1D",
        "gTex1DSampler",
        RndTextureBase::kTexture1D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTexture2D = resources.AddTexture(
        "gTexture2D",
        "gTex2DSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTexture3D = resources.AddTexture(
        "gTexture3D",
        "gTex3DSampler",
        RndTextureBase::kTexture3D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTextureArray1D = resources.AddTexture(
        "gTextureArray1D",
        "gTexArray1DSampler",
        RndTextureBase::kTextureArray1D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTextureArray2D = resources.AddTexture(
        "gTextureArray2D",
        "gTexArray2DSampler",
        RndTextureBase::kTextureArray2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTextureCube = resources.AddTexture(
        "gTextureCube",
        "gTexCubeSampler",
        RndTextureBase::kTextureCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTextureArrayCube = resources.AddTexture(
        "gTextureArrayCube",
        "gTexArrayCubeSampler",
        RndTextureBase::kTextureArrayCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTexture2DRTSliced = resources.AddTexture2DRTSliced(
        "gTexture2DRTSliced",
        "gTex2DRTSlicedSampler",
        RndTextureBase::kTexture2D,
        kShaderNumericFloat4);
    mTextureArray2DRTSliced = resources.AddTexture2DRTSliced(
        "gTextureArray2DRTSliced",
        "gTexArray2DRTSlicedSampler",
        RndTextureBase::kTextureArray2D,
        kShaderNumericFloat4);
    mStencilBuffer = resources.AddTexture(
        "gStencilBuffer",
        "gStencilBufferSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericUInt4);
    mTexture2DNoSampler = resources.AddTexture(
        "gTexture2DNoSampler",
        nullptr,
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mComputeBuffer2D = resources.AddComputeBuffer(
        "gComputeBuffer2D", 0, kShaderNumericFloat4, kShaderProgramPixel);
    mLightIds = resources.AddComputeBufferCustomTyped(
        "gLightIds", "Uint2As32", 0, kShaderProgramPixel);
    mLightIdRanges = resources.AddComputeBufferCustomTyped(
        "gLightIdRanges", "CSLightIdRange", 0, kShaderProgramPixel);
}

// Reconstructed from eboot.elf at 0x6B6050. The 2D compute mode shows its
// buffer as the smallest square that holds it. The array modes that show
// every element pass the element count; the maximum overdraws come from
// the device's settings. Each shape of texture is bound at its slot, the
// 2D texture at the stencil or unsampled slot in those modes; the tiled
// lighting modes bind the light ID ranges, and the light IDs as well in
// the two modes that read them.
void RndBufferInspectionShader::Select(RndContext& context, Params& params) {
    constexpr unsigned long kPixelKey = 3;
    constexpr int kTexMode1DArrayAll = 4;
    constexpr int kTexMode2DArrayAll = 6;
    constexpr int kTexModeCubeArrayAll = 8;
    constexpr int kTexMode2DNoSampler = 9;
    constexpr int kTexModeStencil = 15;
    constexpr int kTexModeCompute2D = 19;
    constexpr int kTexModeTiledLightingOverdraw = 20;
    constexpr int kTexModeDepthTiledLightingOverdraw = 21;
    constexpr int kTexModeBothEyesTiledLightingOverdraw = 22;
    constexpr int kTexModeBothEyesDepthTiledLightingOverdraw = 23;

    const RndConfig* settings = TheRndDevice()->mSettings;
    RndShaderCBuffer& buffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    auto member = [&buffer](unsigned long offset) {
        return static_cast<float*>(RndShaderDrawUtl::GetCBufferMember(buffer, offset));
    };

    member(mBufferDisplayMode)[0] = static_cast<float>(params.mDisplayMode);
    buffer.mSyncPending = true;

    int auxWidth;
    int auxHeight;
    if (params.mTexMode == kTexModeCompute2D && params.mComputeBuffer != nullptr) {
        const unsigned long count = params.mComputeBuffer->mDesc.mNumElements;
        auxWidth = RoundToInt(std::sqrt(static_cast<float>(count)));
        auxHeight = auxWidth;
    } else {
        auxWidth = params.mAuxDims[0];
        auxHeight = params.mAuxDims[1];
    }
    float* auxDims = member(mAuxDims);
    auxDims[0] = static_cast<float>(auxWidth);
    auxDims[1] = static_cast<float>(auxHeight);
    buffer.mSyncPending = true;

    float arraySize = 1.0F;
    switch (params.mTexMode) {
    case kTexModeCubeArrayAll:
        if (params.mTextureArrayCube != nullptr) {
            arraySize = static_cast<float>(params.mTextureArrayCube->mCubes.size());
        }
        break;
    case kTexMode2DArrayAll:
        if (params.mTextureArray2D != nullptr) {
            arraySize = static_cast<float>(params.mTextureArray2D->mPixels.size());
        }
        break;
    case kTexMode1DArrayAll:
        if (params.mTextureArray1D != nullptr) {
            arraySize = static_cast<float>(params.mTextureArray1D->mPixels.size());
        }
        break;
    default:
        break;
    }
    float* arrayParams = member(mTextureArrayParams);
    arrayParams[0] = static_cast<float>(params.mArrayElement);
    arrayParams[1] = arraySize;
    arrayParams[2] = static_cast<float>(params.mArrayParamZ);
    buffer.mSyncPending = true;

    float* tint = member(mTint);
    tint[0] = params.mTint.red;
    tint[1] = params.mTint.green;
    tint[2] = params.mTint.blue;
    tint[3] = params.mTint.alpha;
    member(mDiscardPadPixels)[0] = params.mDiscardPadPixels ? 1.0F : 0.0F;
    member(mBufferInspectionDepthFrac)[0] = params.mDepthFrac;
    float* stencilParams = member(mStencilParams);
    stencilParams[0] = static_cast<float>(params.mStencilParams[0]);
    stencilParams[1] = static_cast<float>(params.mStencilParams[1]);
    float* lightTileCounts = member(mLightTileCounts);
    lightTileCounts[0] = static_cast<float>(params.mLightTileCounts[0]);
    lightTileCounts[1] = static_cast<float>(params.mLightTileCounts[1]);
    float* maxOverdraw = member(mMaxOverdraw);
    maxOverdraw[0] =
        static_cast<float>(static_cast<unsigned long>(settings->mMaxLightingOverdraw));
    maxOverdraw[1] =
        static_cast<float>(static_cast<unsigned long>(settings->mMaxLightProbeOverdraw));
    RndShaderDrawUtl::CommitCBuffer(buffer, context, mCBufferSize);

    const RndDefaults& defaults = TheRndDevice()->mDefaults;
    auto select = [&context](RndShaderResource* resource, unsigned long slot, unsigned int flags) {
        resource->Select(context, kShaderProgramPixel, slot, flags, 0);
    };
    auto orError = [](RndTextureBase* texture, RndTextureBase* error) {
        return texture != nullptr ? texture : error;
    };
    select(
        orError(params.mTexture1D, defaults.mTextures1D[kDefaultTextureError]), mTexture1D, 0);
    RndTextureBase* texture2D =
        orError(params.mTexture2D, defaults.mTextures2D[kDefaultTextureError]);
    if (params.mTexMode == kTexMode2DNoSampler) {
        select(texture2D, mTexture2DNoSampler, RndShaderResource::kSelectNoSampler);
    } else if (params.mTexMode == kTexModeStencil) {
        select(texture2D, mStencilBuffer, RndShaderResource::kSelectStencil);
    } else {
        select(texture2D, mTexture2D, 0);
    }
    select(
        orError(params.mTexture3D, defaults.mTextures3D[kDefaultTextureError]), mTexture3D, 0);
    select(
        orError(params.mTextureArray1D, defaults.mTexturesArray1D[kDefaultTextureError]),
        mTextureArray1D,
        0);
    select(
        orError(params.mTextureArray2D, defaults.mTexturesArray2D[kDefaultTextureError]),
        mTextureArray2D,
        0);
    select(
        orError(params.mTextureCube, defaults.mTexturesCube[kDefaultTextureError]),
        mTextureCube,
        0);
    select(
        orError(params.mTextureArrayCube, defaults.mTexturesArrayCube[kDefaultTextureError]),
        mTextureArrayCube,
        0);
    select(
        orError(params.mTexture2DRTSliced, defaults.mTextures2D[kDefaultTextureError]),
        mTexture2DRTSliced,
        RndShaderResource::kSelectRTSliced);
    if (params.mTextureArray2DRTSliced != nullptr) {
        select(
            params.mTextureArray2DRTSliced,
            mTextureArray2DRTSliced,
            RndShaderResource::kSelectRTSliced);
    }

    switch (params.mTexMode) {
    case kTexModeCompute2D:
        if (params.mComputeBuffer != nullptr) {
            select(params.mComputeBuffer, mComputeBuffer2D, 0);
        }
        break;
    case kTexModeTiledLightingOverdraw:
    case kTexModeBothEyesTiledLightingOverdraw:
        if (params.mComputeBuffer != nullptr && params.mLightIds != nullptr) {
            select(params.mComputeBuffer, mLightIdRanges, 0);
            select(params.mLightIds, mLightIds, 0);
        }
        break;
    case kTexModeDepthTiledLightingOverdraw:
    case kTexModeBothEyesDepthTiledLightingOverdraw:
        if (params.mComputeBuffer != nullptr) {
            select(params.mComputeBuffer, mLightIdRanges, 0);
        }
        break;
    default:
        break;
    }

    RndShaderKeyGroup keys{};
    keys.mKeys[kPixelKey] = mTexMode.SetValue(0, static_cast<unsigned int>(params.mTexMode));
    _SelectShaderCollection(context, keys);
}
