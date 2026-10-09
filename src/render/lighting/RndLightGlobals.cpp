#include "render/lighting/RndLightGlobals.h"

#include "entity/core/Entity.h"
#include "os/files/File.h"
#include "render/buffers/RndComputeBuffer.h"
#include "math/scalar/Trig.h"
#include "render/meshes/RndMesh.h"
#include "render/meshes/RndMeshTyped.h"
#include "render/meshes/RndMeshUtl.h"
#include "render/shaders/RndShader.h"
#include "render/lighting/deferred/RndLightDirectionalDeferredShader.h"
#include "render/lighting/deferred/RndLightPointDeferredShader.h"
#include "render/lighting/deferred/RndLightProbeDeferredAccumShader.h"
#include "render/lighting/deferred/RndLightProbeDeferredShader.h"
#include "render/lighting/deferred/RndLightSpotDeferredShader.h"
#include "render/lighting/shadows/RndShaderLightDirectionalShadowGen.h"
#include "render/lighting/shadows/RndShaderLightPointShadowGen.h"
#include "render/lighting/shadows/RndShaderLightSpotShadowGen.h"
#include "render/lighting/tiled/RndCShaderTiledLightsApplication.h"
#include "render/lighting/tiled/RndCShaderTiledLightsCull.h"
#include "render/lighting/tiled/RndCShaderTiledLightsInterpolation.h"
#include "render/lighting/tiled/RndCShaderTiledLightsStereoToMono.h"
#include "render/postprocessing/tonemap/RndCShaderTonemap.h"
#include "render/postprocessing/tonemap/RndTonemapShader.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"
#include "render/textures/RndTextureUtilityCom.h"

namespace {

constexpr const char* kTiledLightIdsCountName = "Tiled Light Ids Count";

constexpr float kPi = 3.1415927F;
constexpr float kHalfPi = 1.5707964F;
constexpr float kQuarterPi = 0.78539819F;
constexpr float kTwoPi = 6.2831855F;

// The first two vtable slots of an owner whose type is not recovered yet.
// Name not in the reference map.
struct UnrecoveredOwnerVtable {
    void (*mDestroy)(void* object);
    void (*mDelete)(void* object);
};

// Runs an unrecovered owner's virtual deleting destructor, as delete would.
// Name not in the reference map.
void DeleteUnrecovered(void*& object) {
    if (object != nullptr) {
        (*static_cast<UnrecoveredOwnerVtable**>(object))->mDelete(object);
    }
    object = nullptr;
}

// Creates a shader and registers it with the shader manager.
template <class T>
T* Create() {
    auto* shader = new T;
    shader->_Register();
    return shader;
}

template <class T>
void SafeDelete(T*& object) {
    delete object;
    object = nullptr;
}

template <class T>
void ReleaseResource(ResourcePtr<T>& resource) {
    if (resource.mResource != nullptr) {
        resource.mResource->ReleaseRef();
    }
    resource.mResource = nullptr;
}

// The loop re-reads the size after each deletion and leaves the deleted
// pointers in place until the vector is cleared.
void DeleteAndClear(eastl::vector<RndTextureBase*>& textures) {
    for (unsigned long index = 0; index < textures.size(); ++index) {
        delete textures[index];
    }
    textures.clear();
}

RndTextureBase* FindByWidth(
    const eastl::vector<RndTextureBase*>& textures,
    int size) {
    for (auto* texture : textures) {
        if (texture->mBaseDesc.mWidth == static_cast<unsigned int>(size)) {
            return texture;
        }
    }
    return nullptr;
}

}  // namespace

// Reconstructed from eboot.elf at 0x47EEE0.
RndLightGlobals::RndLightGlobals()
    : mSphereMesh(nullptr),
      mSpotlightMesh(nullptr),
      mSphereScale(-1.0F),
      mSpotlightSegments(16),
      mSpotlightCapSegments(8),
      mDirectionalShader(nullptr),
      mDirectionalShadowGenShader(nullptr),
      mPointShader(nullptr),
      mPointShadowGenShader(nullptr),
      mSpotShader(nullptr),
      mSpotShadowGenShader(nullptr),
      mProbeShader(nullptr),
      mProbeAccumShader(nullptr),
      mTiledLightIdsCount(nullptr),
      mTiledLightsCullShader(nullptr),
      mTiledLightsApplicationShader(nullptr),
      mTiledLightsInterpolationShader(nullptr),
      mTiledLightsStereoToMonoShader(nullptr),
      mHairReflectanceTextures{},
      mTonemapShader(nullptr),
      mTonemapCShader(nullptr),
      mProbeCaptureBuffers(nullptr),
      mProbeCaptureTexture(nullptr),
      mProbeDiffuseCaptureTexture(nullptr),
      mProbeSpecularCaptureTexture(nullptr),
      mFilterLightProbeCShader(nullptr) {}

// Reconstructed from eboot.elf at 0x47EFA0. The vectors and the resource
// references release themselves.
RndLightGlobals::~RndLightGlobals() {}

// Reconstructed from eboot.elf at 0x47F030.
void RndLightGlobals::Init() {
    _InitMeshes();
    _InitShaders();
    _InitBuffers();
}

// Reconstructed from eboot.elf at 0x47F1C0. The sphere is circumscribed
// around the unit sphere; mSphereScale is the radius of the sphere that
// encloses the mesh.
void RndLightGlobals::_InitMeshes() {
    RndMeshUtl::CreateSphereParams params;
    params.mName = "lighting_sphere";
    params.mVertexType = kVertexPosOnly;
    params.mFlags = RndMeshUtl::kCreateMeshCircumscribe;
    params.mNumSegments = 16;
    params.mRadius = 1.0F;
    params.mNumRings = 8;
    mSphereMesh = RndMeshUtl::CreateSphere(params);

    mSphereScale =
        1.0F / Sine(kHalfPi / static_cast<float>(params.mNumRings) + kHalfPi);
    mSphereScale /=
        Sine(kPi / static_cast<float>(params.mNumSegments) + kHalfPi);
    _InitLightSpotMesh();
}

// Reconstructed from eboot.elf at 0x47F300.
void RndLightGlobals::_InitShaders() {
    constexpr std::uint32_t kAsyncComputeFeature = 0x10;
    mDirectionalShader = Create<RndLightDirectionalDeferredShader>();
    mDirectionalShadowGenShader = Create<RndShaderLightDirectionalShadowGen>();
    mPointShader = Create<RndLightPointDeferredShader>();
    mPointShadowGenShader = Create<RndShaderLightPointShadowGen>();
    mSpotShader = Create<RndLightSpotDeferredShader>();
    mSpotShadowGenShader = Create<RndShaderLightSpotShadowGen>();
    mProbeShader = Create<RndLightProbeDeferredShader>();
    mProbeAccumShader = Create<RndLightProbeDeferredAccumShader>();
    mTonemapShader = Create<RndTonemapShader>();
    if ((TheRndDevice()->mCapabilities[kPlatformPS4].mFeatureFlags & kAsyncComputeFeature) != 0) {
        mTiledLightsCullShader = Create<RndCShaderTiledLightsCull>();
        mTiledLightsApplicationShader = Create<RndCShaderTiledLightsApplication>();
        mTiledLightsInterpolationShader = Create<RndCShaderTiledLightsInterpolation>();
        mTiledLightsStereoToMonoShader = Create<RndCShaderTiledLightsStereoToMono>();
        mTonemapCShader = Create<RndCShaderTonemap>();
    }
}

// Reconstructed from eboot.elf at 0x47F500.
void RndLightGlobals::_InitBuffers() {
    const RndComputeBuffer::Description desc{
        4,
        1,
        nullptr,
        nullptr,
        8,
        1,
        kTiledLightIdsCountName,
    };
    mTiledLightIdsCount = RndComputeBuffer::New(desc);
}

// Reconstructed from eboot.elf at 0x47F580.
void RndLightGlobals::Terminate() {
    SafeDelete(mSphereMesh);
    SafeDelete(mSpotlightMesh);
    SafeDelete(mDirectionalShader);
    SafeDelete(mDirectionalShadowGenShader);
    SafeDelete(mPointShader);
    SafeDelete(mPointShadowGenShader);
    SafeDelete(mSpotShader);
    SafeDelete(mSpotShadowGenShader);
    SafeDelete(mProbeShader);
    SafeDelete(mProbeAccumShader);
    SafeDelete(mTiledLightsCullShader);
    SafeDelete(mTiledLightsApplicationShader);
    SafeDelete(mTiledLightsInterpolationShader);
    SafeDelete(mTiledLightsStereoToMonoShader);
    DeleteUnrecovered(mFilterLightProbeCShader);
    SafeDelete(mTonemapShader);
    SafeDelete(mTonemapCShader);
    SafeDelete(mTiledLightIdsCount);
    DeleteUnrecovered(mProbeCaptureBuffers);
    DeleteUnrecovered(mProbeCaptureTexture);
    DeleteAndClear(mProbeCaptureDownsampleTextures);
    DeleteAndClear(mProbeCaptureHelperTextures);
    DeleteUnrecovered(mProbeDiffuseCaptureTexture);
    DeleteUnrecovered(mProbeSpecularCaptureTexture);
    ReleaseResource(mErrorLightCookie);
    ReleaseResource(mSkinDiffusion);
    ReleaseResource(mInlineLightingTextures);
    mHairReflectanceTextures[0] = nullptr;
    mHairReflectanceTextures[1] = nullptr;
}

// Reconstructed from eboot.elf at 0x47FDE0. The volume is a truncated
// rounded cone with a 45-degree half-angle, a top radius of 0.5 and a length
// of 2. After building it, every vertex is moved onto the unit circle of its
// sweep angle at z = 0 (the top vertices scaled toward the axis), and the
// vertex color records where the vertex lies: green 1 with blue rising over
// the cap, red falling from 1 along the side, and zero across the top.
void RndLightGlobals::_InitLightSpotMesh() {
    RndMeshUtl::CreateTruncatedRoundedConeParams params;
    params.mCone.SetAngleTopRadiusAndLength(kQuarterPi, 0.5F, 2.0F);
    params.mName = "spotlight";
    params.mVertexType = kVertexColor;
    params.mNumSegments = mSpotlightSegments;
    params.mNumTopSegments = 1;
    params.mNumSideSegments = 1;
    params.mNumCapSegments = mSpotlightCapSegments;
    params.mFlags = RndMeshUtl::kCreateMeshNoSync;
    auto* mesh = static_cast<RndMeshTyped<RndVertexColor>*>(
        RndMeshUtl::CreateTruncatedRoundedCone(params));
    mSpotlightMesh = mesh;

    const unsigned long numSegments = params.mNumSegments;
    const unsigned long numCapRings = params.mNumCapSegments / 2;
    const unsigned long stride = numSegments + 1;
    for (unsigned long segment = 0; segment <= numSegments; ++segment) {
        float angle = 0.0F;
        if (segment != 0 && segment != numSegments) {
            angle = segment / static_cast<float>(numSegments) * kTwoPi;
        }
        const float cosine = Sine(angle + kHalfPi);
        const float sine = Sine(angle);
        RndVertexColor* verts = mesh->mVerts.mpBegin;

        unsigned long index = segment;
        for (unsigned long ring = 0; ring < numCapRings; ++ring, index += stride) {
            RndVertexColor& vertex = verts[index];
            vertex.mPos[0] = cosine;
            vertex.mPos[1] = sine;
            vertex.mPos[2] = 0.0F;
            vertex.mColor[0] = 0.0F;
            vertex.mColor[1] = 1.0F;
            vertex.mColor[2] = ring / static_cast<float>(numCapRings);
            vertex.mColor[3] = 0.0F;
        }

        const unsigned long numSide = params.mNumSideSegments;
        for (unsigned long ring = 0; ring <= numSide; ++ring, index += stride) {
            float weight = 1.0F;
            if (ring >= 2) {
                weight = 1.0F - (ring - 1) / static_cast<float>(numSide);
            }
            RndVertexColor& vertex = verts[index];
            vertex.mPos[0] = cosine;
            vertex.mPos[1] = sine;
            vertex.mPos[2] = 0.0F;
            vertex.mColor[0] = weight;
            vertex.mColor[1] = 0.0F;
            vertex.mColor[2] = 0.0F;
            vertex.mColor[3] = 0.0F;
        }

        const unsigned long numTop = params.mNumTopSegments;
        for (unsigned long ring = 0; ring <= numTop; ++ring, index += stride) {
            float scale = 1.0F;
            if (ring >= 2) {
                scale = 0.0F;
                if (ring - 1 != numTop) {
                    scale = 1.0F - (ring - 1) / static_cast<float>(numTop);
                }
            }
            RndVertexColor& vertex = verts[index];
            vertex.mPos[0] = scale * cosine;
            vertex.mPos[1] = scale * sine;
            vertex.mPos[2] = 0.0F;
            vertex.mColor[0] = 0.0F;
            vertex.mColor[1] = 0.0F;
            vertex.mColor[2] = 0.0F;
            vertex.mColor[3] = 0.0F;
        }
    }
    mesh->SyncStatic();
}

// Reconstructed from eboot.elf at 0x47FD60.
RndTextureBase* RndLightGlobals::GetProbeCaptureDownsampleTexture(int size) {
    return FindByWidth(mProbeCaptureDownsampleTextures, size);
}

// Reconstructed from eboot.elf at 0x47FDA0.
RndTextureBase* RndLightGlobals::GetProbeCaptureHelperTexture(int size) {
    return FindByWidth(mProbeCaptureHelperTextures, size);
}

// Reconstructed from eboot.elf at 0x47F8D0. The resources load when the
// renderer initializes rendering or resources are being precached. The
// typed loads are the GetOrLoad instantiations at 0x47FAA0 and 0x44B1A0.
void RndLightGlobals::LoadResources(const RndInitParams& params) {
    if (!gResourcePrecacheMode && !params.mInitRendering) {
        return;
    }
    mInlineLightingTextures = Resource::GetOrLoad<EntityResource>(
        ResourcePath("../../system/data/render/lighting/inline_lighting_textures.entity"),
        false);
    mSkinDiffusion = Resource::GetOrLoad<RndTexture2DResource>(
        ResourcePath("../../system/data/render/lighting/skin_diffusion.bmp"),
        false);
    mHairReflectanceTextures[0] = nullptr;
    mHairReflectanceTextures[1] = nullptr;
    EntityResource* textures = mInlineLightingTextures;
    if (textures == nullptr || textures->Fail()) {
        return;
    }
    if (RndTextureUtilityCom* utility = textures->mEntity->GetRoot()->GetCom<RndTextureUtilityCom>()) {
        mHairReflectanceTextures[0] = utility->GetHairReflectanceTex0();
        mHairReflectanceTextures[1] = utility->GetHairReflectanceTex1();
    }
}

// Reconstructed from eboot.elf at 0x47FCD0.
ResourcePath RndLightGlobals::GetSkinDiffusionTexPath() {
    return ResourcePath("../../system/data/render/lighting/skin_diffusion.bmp");
}
