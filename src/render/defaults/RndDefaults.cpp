#include "render/defaults/RndDefaults.h"

#include <cmath>
#include <cstdint>

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"
#include "math/color/Color.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector3.h"
#include "math/vector/Vector3i.h"
#include "os/files/File.h"
#include "os/memory/MemMgr.h"
#include "render/buffers/RndComputeBuffer.h"
#include "render/context/RndCameraCom.h"
#include "render/context/RndCameraContext.h"
#include "render/lighting/lights/RndLightCom.h"
#include "render/lighting/lights/RndLightDirectionalCom.h"
#include "render/lighting/lights/RndLightProbeCom.h"
#include "render/lighting/lights/RndLightSpotCom.h"
#include "render/materials/RndMaterialCom.h"
#include "render/scene/RndSceneCom.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndPixelFormat.h"
#include "render/textures/RndPixelCanvas.h"
#include "render/textures/RndPixelDataCube.h"
#include "render/textures/RndTexture1D.h"
#include "render/textures/RndTexture2D.h"
#include "render/textures/RndTexture3D.h"
#include "render/textures/RndTextureArray1D.h"
#include "render/textures/RndTextureArray2D.h"
#include "render/textures/RndTextureArrayCube.h"
#include "render/textures/RndTextureCube.h"
#include "render/textures/RndTextureUtl.h"

namespace {

constexpr const char* kDefaultComputeBufferName = "Default Compute Buffer";

constexpr const char* kDefaultLightingScene =
    "../../system/data/render/default_lighting.scene";
constexpr const char* kDefaultDirectional = "default_directional";
constexpr const char* kDefaultShadowedSpot = "default_spot_with_shadows";
constexpr const char* kDefaultProbe = "default_probe";
constexpr const char* kBackupDirectional = "default_directional_light";

constexpr const char* kDefaultUnlitShader =
    "../../system/data/shared/shadergraph/default_unlit.sgraph";
constexpr const char* kDefaultLitShader =
    "../../system/data/shared/shadergraph/default.sgraph";
constexpr const char* kDefaultTextShader =
    "../../system/data/shared/shadergraph/default_text_unlit.sgraph";
constexpr const char* kDefaultParticleShader =
    "../../system/data/shared/shadergraph/default_particle.sgraph";
constexpr const char* kDefaultDecalShader =
    "../../system/data/shared/shadergraph/default_decal_lit.sgraph";

Vector3 Cross(const Vector3& a, const Vector3& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

// A zero vector stays zero.
Vector3 Normalize(const Vector3& v) {
    const float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    const float scale = length != 0.0f ? 1.0f / length : 0.0f;
    return {scale * v.x, scale * v.y, scale * v.z};
}

// Creates an object named for a default material and gives it a material
// that is never shared. Inlined six times into
// RndDefaults::_CreateMaterials (0x6BEC50). Name not in the reference map.
RndMaterialCom* CreateMaterialObject(Entity* entity, const char* name) {
    GameObject* object = entity->CreateObject(0, 0);
    object->SetName(Symbol(name));
    auto* material = reinterpret_cast<RndMaterialCom*>(
        object->CreateComponent(RndMaterialCom::sClassName, false));
    material->SetSharingType(*object, RndMaterialSharing::kUnique);
    return material;
}

}  // namespace

// The backup directional light's forward axis. It sits in the object's
// zero-initialized data, and nothing in this build writes it. Name not in
// the reference map.
static Vector3 sBackupLightDirection;  // 0x1AB007C

// Reconstructed from eboot.elf at 0x6BDB30.
RndDefaults::RndDefaults()
    : mSceneResource(),
      mTextures1D(),
      mTextures2D(),
      mTextures3D(),
      mTexturesCube(),
      mTexturesArray1D(),
      mTexturesArray2D(),
      mTexturesArrayCube(),
      mComputeBuffers(),
      mCamera(nullptr),
      mUnlitMaterial(nullptr),
      mAdditiveMaterial(nullptr),
      mLitMaterial(nullptr),
      mTextMaterial(nullptr),
      mParticleMaterial(nullptr),
      mDecalMaterial(nullptr),
      mLightingResource(),
      mLightMgr(nullptr),
      mLightProbe(nullptr),
      mLightingType(kDefaultLightingDirectional),
      mLightingScale(100.0f) {
}

// Reconstructed from eboot.elf at 0x6BDC20. The members release the light
// lists and the two scene resources.
RndDefaults::~RndDefaults() {
}

// Reconstructed from eboot.elf at 0x6BDCA0.
void RndDefaults::Init(const RndInitParams& params) {
    if (!params.mInitRendering && !gResourcePrecacheMode) {
        return;
    }

    mSceneResource = new RndSceneResource();
    Entity* entity = mSceneResource->CreateEntity();
    _CreateTextures();
    _CreateComputeBuffers();
    _CreateCamera(entity);
    _CreateMaterials(entity);
    if (!_LoadLighting()) {
        _CreateBackupLighting(entity);
    }

    mSceneResource->LoadResources();
    mSceneResource->EnterEntity(mSceneResource->mEntity, 1);
    if (mLightingResource) {
        mLightingResource->EnterEntity(mLightingResource->mEntity, 1);
    }
}

// Reconstructed from eboot.elf at 0x6BDE60. Every default type is made in
// every shape. The 1D, 2D and 3D textures convert a canvas filled with the
// type's colours; the array and cube textures copy the 1D or 2D pixels.
void RndDefaults::_CreateTextures() {
    for (int i = 0; i < kNumDefaultTextureTypes; ++i) {
        const auto type = static_cast<RndDefaultTextureType>(i);
        const RndDataFormatInfo info{32, 4, 0, 2, -1};
        const int dataFormat = RndFindSupportedDataFormat(info, kPlatformPS4);

        RndPixelFormat format{};
        Hmx::Color primary(0.0F, 0.0F, 0.0F, 1.0F);
        Hmx::Color secondary(0.0F, 0.0F, 0.0F, 1.0F);
        bool checkerboard = false;
        switch (type) {
        case kDefaultTextureWhite:
            primary = Hmx::Color::GetWhite();
            break;
        case kDefaultTextureBlack:
            primary = Hmx::Color::GetBlack();
            break;
        case kDefaultTextureZero:
            primary = Hmx::Color::GetZero();
            break;
        case kDefaultTextureFlatNormal:
            primary = Hmx::Color(0.5F, 0.5F, 1.0F, 1.0F);
            format.mUsage = 3;
            break;
        case kDefaultTextureError:
            primary = Hmx::Color::GetOrange();
            secondary = Hmx::Color::GetCyan();
            checkerboard = true;
            break;
        case kDefaultTextureErrorGreyscale:
            primary = Hmx::Color(0.25F, 0.25F, 0.25F, 1.0F);
            secondary = Hmx::Color(0.75F, 0.75F, 0.75F, 1.0F);
            checkerboard = true;
            break;
        case kDefaultTextureErrorNormal:
            primary = Hmx::Color(1.0F, 0.0F, 0.0F, 1.0F);
            secondary = Hmx::Color(0.0F, 1.0F, 0.0F, 1.0F);
            checkerboard = true;
            format.mUsage = 3;
            break;
        default:
            break;
        }
        format.mWrapMode = 2;
        format.mFilterMode = 1;

        const int size = checkerboard ? 64 : 8;
        const char* name = RndTextureUtl::GetDefaultTextureName(type);

        RndPixelCanvas canvas;
        canvas.CreateWithColor(size, 1, 1, primary);
        if (checkerboard) {
            RndTextureUtl::FillCheckerboard(canvas, primary, secondary, 8);
        }

        RndTexture1D::Description desc1D;
        desc1D.mRequestedFormat = format;
        desc1D.mName = name;
        desc1D.mPixels.CreateEmpty(size, 1, 1, dataFormat);
        desc1D.mPixels.ConvertFrom(canvas);
        mTextures1D[i] = RndTexture1D::New(desc1D, nullptr);

        RndTextureArray1D::Description descArray1D;
        descArray1D.mRequestedFormat = format;
        descArray1D.mName = name;
        descArray1D.mPixels.resize(1);
        descArray1D.mPixels[0].Create(size, 1, 1, dataFormat, nullptr);
        descArray1D.mPixels[0].CopyFrom(desc1D.mPixels);
        mTexturesArray1D[i] = RndTextureArray1D::New(descArray1D, nullptr);

        canvas.CreateWithColor(size, size, 1, primary);
        if (checkerboard) {
            RndTextureUtl::FillCheckerboard(canvas, primary, secondary, 8);
        }

        RndTexture2D::Description desc2D;
        desc2D.mRequestedFormat = format;
        desc2D.mName = name;
        desc2D.mPixels.CreateEmpty(size, size, 1, dataFormat);
        desc2D.mPixels.ConvertFrom(canvas);
        mTextures2D[i] = RndTexture2D::New(desc2D);

        RndTextureArray2D::Description descArray2D;
        descArray2D.mRequestedFormat = format;
        descArray2D.mName = name;
        descArray2D.mPixels.resize(1);
        descArray2D.mPixels[0].Create(size, size, 1, dataFormat, nullptr);
        descArray2D.mPixels[0].CopyFrom(desc2D.mPixels);
        mTexturesArray2D[i] = RndTextureArray2D::New(descArray2D);

        RndTextureCube::Description descCube;
        descCube.mRequestedFormat = format;
        descCube.mName = name;
        for (RndPixelData& face : descCube.mCube.mFaces) {
            face.Create(size, size, 1, dataFormat, nullptr);
            face.CopyFrom(desc2D.mPixels);
        }
        mTexturesCube[i] = RndTextureCube::New(descCube, nullptr);

        RndTextureArrayCube::Description descArrayCube;
        descArrayCube.mRequestedFormat = format;
        descArrayCube.mName = name;
        descArrayCube.mCubes.resize(1);
        for (RndPixelData& face : descArrayCube.mCubes[0].mFaces) {
            face.Create(size, size, 1, dataFormat, nullptr);
            face.CopyFrom(desc2D.mPixels);
        }
        mTexturesArrayCube[i] = RndTextureArrayCube::New(descArrayCube, nullptr);

        canvas.CreateWithColor(size, size, size, primary);
        if (checkerboard) {
            RndTextureUtl::FillCheckerboard(canvas, primary, secondary, 8);
        }

        RndTexture3D::Description desc3D;
        desc3D.mRequestedFormat = format;
        desc3D.mName = name;
        desc3D.mPixels.CreateEmpty(size, size, size, dataFormat);
        desc3D.mPixels.ConvertFrom(canvas);
        mTextures3D[i] = RndTexture3D::New(desc3D, nullptr);
    }
}

// Reconstructed from eboot.elf at 0x6BEAF0. Init carries an inlined copy.
void RndDefaults::_CreateComputeBuffers() {
    for (std::uint32_t index = 0; index < 2; ++index) {
        const RndComputeBuffer::Description descriptor{
            sizeof(std::uint32_t),
            1,
            &index,
            nullptr,
            0,
            0,
            kDefaultComputeBufferName,
        };
        mComputeBuffers[index] = RndComputeBuffer::New(descriptor);
    }
}

// Reconstructed from eboot.elf at 0x6BEBC0 and the equivalent inlined sequence
// in Init at 0x6BDCA0.
void RndDefaults::_CreateCamera(Entity* entity) {
    GameObject* object = entity->CreateObject(0, 0);
    object->SetName(Symbol("default_cam"));
    mCamera = reinterpret_cast<RndCameraCom*>(
        object->CreateComponent(RndCameraCom::sClassName, false));
}

// Reconstructed from eboot.elf at 0x6BEC50.
void RndDefaults::_CreateMaterials(Entity* entity) {
    mUnlitMaterial = CreateMaterialObject(entity, "default_mat_unlit");
    mUnlitMaterial->SetShaderGraphFile(kDefaultUnlitShader);

    mAdditiveMaterial = CreateMaterialObject(entity, "default_mat_add");
    mAdditiveMaterial->SetBlendMode(RndBlendMode::kAdd);
    mAdditiveMaterial->SetShaderGraphFile(kDefaultUnlitShader);

    mLitMaterial = CreateMaterialObject(entity, "default_mat_lit");
    mLitMaterial->SetShaderGraphFile(kDefaultLitShader);

    mTextMaterial = CreateMaterialObject(entity, "default_text_mat");
    mTextMaterial->SetShaderGraphFile(kDefaultTextShader);

    mParticleMaterial = CreateMaterialObject(entity, "default_particle_mat");
    mParticleMaterial->SetBlendMode(RndBlendMode::kAdd);
    mParticleMaterial->SetShaderGraphFile(kDefaultParticleShader);

    mDecalMaterial = CreateMaterialObject(entity, "default_decal_mat");
    mDecalMaterial->SetBlendMode(RndBlendMode::kSource);
    mDecalMaterial->SetShaderGraphFile(kDefaultDecalShader);
}

// Reconstructed from eboot.elf at 0x6BEF40.
bool RndDefaults::_LoadLighting() {
    mLightingResource = Resource::GetOrLoad<RndSceneResource>(
        ResourcePath(kDefaultLightingScene), false);
    if (!mLightingResource) {
        return false;
    }

    Entity* entity = mLightingResource->mEntity;
    RndLightMgrCom* lightMgr =
        entity->GetRoot()->GetCom<RndSceneCom>()->GetLightMgr();
    if (lightMgr == nullptr) {
        mLightingResource = nullptr;
        return false;
    }

    mLightMgr = lightMgr;
    mDirectionalLights.clear();
    mShadowedSpotLights.clear();

    // Only the lights picked out below stay on.
    for (GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        if (auto* light = object->GetBaseCom<RndLightCom>()) {
            light->mEnabled = false;
        }
        if (auto* probe = object->GetCom<RndLightProbeCom>()) {
            probe->mEnabled = false;
        }
    }

    GameObject* directional =
        entity->TryGetObject(Symbol(kDefaultDirectional), false);
    if (directional != nullptr
        && directional->GetCom<RndLightDirectionalCom>() != nullptr) {
        mDirectionalLights.push_back(directional->mId);
    }

    GameObject* spot = entity->TryGetObject(Symbol(kDefaultShadowedSpot), false);
    if (spot != nullptr && spot->GetCom<RndLightSpotCom>() != nullptr) {
        _SyncSpotlight(spot);
        mShadowedSpotLights.push_back(spot->mId);
    }

    GameObject* probe = entity->TryGetObject(Symbol(kDefaultProbe), false);
    if (probe != nullptr) {
        mLightProbe = probe->GetCom<RndLightProbeCom>();
        if (mLightProbe != nullptr) {
            mLightProbe->mEnabled = true;
            mLightProbe->SetFalloffStart(mLightingScale + mLightingScale);
            mLightProbe->SetFalloffEnd(mLightingScale * 3.0f);
        }
    }

    _SyncEnabledLights();
    return true;
}

// Reconstructed from eboot.elf at 0x6BF4F0.
void RndDefaults::_CreateBackupLighting(Entity* entity) {
    mLightMgr = entity->GetRoot()->GetCom<RndSceneCom>()->GetLightMgr();

    GameObject* object = entity->CreateObject(0, 0);
    object->SetName(Symbol(kBackupDirectional));
    auto* light = reinterpret_cast<RndLightDirectionalCom*>(
        object->CreateComponent(RndLightDirectionalCom::sClassName, false));
    light->mLightWrap = 2.0f;

    // Face the light along the backup direction with Z kept up.
    TransCom* trans = object->GetCom<TransCom>();
    Transform xfm = Transform::sID;
    xfm.m.y = sBackupLightDirection;
    xfm.m.x = Normalize(Cross(xfm.m.y, Vector3::sZ));
    xfm.m.z = Cross(xfm.m.x, xfm.m.y);
    trans->SetLocalXfm(xfm);

    mDirectionalLights.push_back(object->mId);
    _SyncEnabledLights();
}

// Reconstructed from eboot.elf at 0x6BF860. The camera is left set.
void RndDefaults::Terminate() {
    mSceneResource = nullptr;
    mLightingResource = nullptr;
    mUnlitMaterial = nullptr;
    mAdditiveMaterial = nullptr;
    mLitMaterial = nullptr;
    mTextMaterial = nullptr;
    mParticleMaterial = nullptr;
    mDecalMaterial = nullptr;
    mLightMgr = nullptr;
    mLightProbe = nullptr;
    mDirectionalLights.clear();
    mShadowedSpotLights.clear();

    for (int i = 0; i < kNumDefaultTextureTypes; ++i) {
        delete mTextures1D[i];
        mTextures1D[i] = nullptr;
        delete mTextures2D[i];
        mTextures2D[i] = nullptr;
        delete mTextures3D[i];
        mTextures3D[i] = nullptr;
        delete mTexturesCube[i];
        mTexturesCube[i] = nullptr;
        delete mTexturesArray1D[i];
        mTexturesArray1D[i] = nullptr;
        delete mTexturesArray2D[i];
        mTexturesArray2D[i] = nullptr;
        delete mTexturesArrayCube[i];
        mTexturesArrayCube[i] = nullptr;
    }
    for (auto*& buffer : mComputeBuffers) {
        if (buffer != nullptr) {
            delete buffer;
        }
        buffer = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x6BFA00.
void RndDefaults::Poll() {
    if (mSceneResource) {
        mSceneResource->PollEntity(mSceneResource->mEntity);
    }
    if (mLightingResource) {
        mLightingResource->PollEntity(mLightingResource->mEntity);
    }
}

// Reconstructed from eboot.elf at 0x6BFA40.
void RndDefaults::SetLightingType(RndDefaultLightingType type) {
    if (mLightingType == type) {
        return;
    }
    mLightingType = type;
    _SyncEnabledLights();
}

// Reconstructed from eboot.elf at 0x6BFA60.
void RndDefaults::_SyncEnabledLights() {
    const ResourcePtr<RndSceneResource> scene =
        mLightingResource ? mLightingResource : mSceneResource;
    for (const GameObjectId& id : mDirectionalLights) {
        Entity* entity = scene->mEntity;
        entity->GetObject(id)->GetExistingBaseCom<RndLightCom>()->mEnabled =
            mLightingType == kDefaultLightingDirectional;
    }
    for (const GameObjectId& id : mShadowedSpotLights) {
        Entity* entity = scene->mEntity;
        entity->GetObject(id)->GetExistingBaseCom<RndLightCom>()->mEnabled =
            mLightingType == kDefaultLightingShadowedSpot;
    }
}

// Reconstructed from eboot.elf at 0x6BFBB0.
bool RndDefaults::IsLightProbeEnabled() const {
    return mLightProbe != nullptr && mLightProbe->mEnabled;
}

// Reconstructed from eboot.elf at 0x6BFBD0.
void RndDefaults::SetLightProbeEnabled(bool enabled) {
    if (mLightProbe != nullptr) {
        mLightProbe->mEnabled = enabled;
    }
}

// Reconstructed from eboot.elf at 0x6BFBF0. The shadowed spots are re-placed
// at the new scale.
void RndDefaults::SetLightingScale(float scale) {
    if (scale == mLightingScale) {
        return;
    }
    mLightingScale = scale;
    _SyncLightProbe();

    const ResourcePtr<RndSceneResource> scene =
        mLightingResource ? mLightingResource : mSceneResource;
    for (const GameObjectId& id : mShadowedSpotLights) {
        _SyncSpotlight(scene->mEntity->GetObject(id));
    }
}

// Reconstructed from eboot.elf at 0x6BFCF0.
void RndDefaults::_SyncLightProbe() {
    if (mLightProbe != nullptr) {
        mLightProbe->SetFalloffStart(mLightingScale + mLightingScale);
        mLightProbe->SetFalloffEnd(mLightingScale * 3.0f);
    }
}

// Reconstructed from eboot.elf at 0x6BFD40. The light is placed on its
// local Z axis at the lighting scale.
void RndDefaults::_SyncSpotlight(GameObject* object) {
    RndLightSpotCom* light = object->GetCom<RndLightSpotCom>();
    light->SetFalloffStart(mLightingScale);
    light->SetFalloffEnd(mLightingScale + mLightingScale);

    TransCom* trans = object->GetCom<TransCom>();
    Transform xfm = Transform::sID;
    xfm.v.x = mLightingScale * xfm.m.z.x;
    xfm.v.y = mLightingScale * xfm.m.z.y;
    xfm.v.z = mLightingScale * xfm.m.z.z;
    trans->SetLocalXfm(xfm);
}

// Reconstructed from eboot.elf at 0x6BFEA0.
float RndDefaults::GetLightingShadowOffset() const {
    if (mShadowedSpotLights.empty()) {
        return 0.0f;
    }

    const ResourcePtr<RndSceneResource> scene =
        mLightingResource ? mLightingResource : mSceneResource;
    return scene->mEntity->GetObject(mShadowedSpotLights.front())
        ->GetExistingCom<RndLightSpotCom>()
        ->mShadowOffset;
}

// Reconstructed from eboot.elf at 0x6BFF70.
void RndDefaults::SetLightingShadowOffset(float offset) {
    const ResourcePtr<RndSceneResource> scene =
        mLightingResource ? mLightingResource : mSceneResource;
    for (const GameObjectId& id : mShadowedSpotLights) {
        scene->mEntity->GetObject(id)
            ->GetExistingCom<RndLightSpotCom>()
            ->mShadowOffset = offset;
    }
}

// Reconstructed from eboot.elf at 0x6C0050. GetTexture is inlined. Stereo
// targets have no sliced default.
RndTextureBase* RndDefaults::GetRTSlicedTexture(
    RndTargetMode targetMode,
    RndDefaultTextureType defaultType) {
    switch (static_cast<int>(targetMode)) {
    case kTargetModeCube:
        return GetTexture(RndTextureBase::kTextureCube, defaultType);
    case kTargetMode2D:
    case kTargetModeLeftEye:
    case kTargetModeRightEye:
    case kTargetModeCubeFace:
    case kTargetModeCubeFace + 1:
    case kTargetModeCubeFace + 2:
    case kTargetModeCubeFace + 3:
    case kTargetModeCubeFace + 4:
    case kTargetModeCubeFace + 5:
        return GetTexture(RndTextureBase::kTexture2D, defaultType);
    default:
        return nullptr;
    }
}

// Reconstructed from eboot.elf at 0x6C00F0.
RndTextureBase* RndDefaults::GetTexture(
    RndTextureBase::Type textureType,
    RndDefaultTextureType defaultType) const {
    switch (textureType) {
    case RndTextureBase::kTexture1D:
        return mTextures1D[defaultType];
    case RndTextureBase::kTexture2D:
        return mTextures2D[defaultType];
    case RndTextureBase::kTexture3D:
        return mTextures3D[defaultType];
    case RndTextureBase::kTextureCube:
        return mTexturesCube[defaultType];
    case RndTextureBase::kTextureArray1D:
        return mTexturesArray1D[defaultType];
    case RndTextureBase::kTextureArray2D:
        return mTexturesArray2D[defaultType];
    case RndTextureBase::kTextureArrayCube:
        return mTexturesArrayCube[defaultType];
    default:
        return nullptr;
    }
}
