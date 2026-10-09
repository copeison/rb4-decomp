#include "render/defaults/RndDefaults.h"

#include <array>
#include <cmath>
#include <cstddef>
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
#include "utl/containers/Std.h"

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

// Describes one RndDefaultTextureType.
struct DefaultTextureSpec {
    const char* mName;
    std::uint32_t mExtent;
    std::uint32_t mUsage;
    Hmx::Color mPrimary;
    Hmx::Color mSecondary;
    bool mCheckerboard;
};

constexpr Hmx::Color kTransparentBlack{0.0F, 0.0F, 0.0F, 0.0F};

constexpr std::array<DefaultTextureSpec, kNumDefaultTextureTypes>
    kDefaultTextureSpecs{{
        {"White", 8, 0, {1.0F, 1.0F, 1.0F, 1.0F}, kTransparentBlack, false},
        {"Black", 8, 0, {0.0F, 0.0F, 0.0F, 1.0F}, kTransparentBlack, false},
        {"Zero", 8, 0, kTransparentBlack, kTransparentBlack, false},
        {"Flat Normal", 8, 3, {0.5F, 0.5F, 1.0F, 1.0F}, kTransparentBlack, false},
        {"Error", 64, 0, {1.0F, 0.5F, 0.0F, 1.0F}, {0.0F, 1.0F, 1.0F, 1.0F}, true},
        {"Error Greyscale", 64, 0, {0.25F, 0.25F, 0.25F, 1.0F}, {0.75F, 0.75F, 0.75F, 1.0F}, true},
        {"Error Normal", 64, 3, {1.0F, 0.0F, 0.0F, 1.0F}, {0.0F, 1.0F, 0.0F, 1.0F}, true},
    }};

constexpr RndDataFormatInfo kDefaultTextureFormat{
    32,
    4,
    0,
    2,
    -1,
};

// Pixel storage reused across the shapes of one texture family.
class DefaultPixelBuffer {
public:
    explicit DefaultPixelBuffer(const DefaultTextureSpec& spec)
        : mSpec(spec) {
    }

    ~DefaultPixelBuffer() {
        if (mPixels != nullptr) {
            HmxAllocator::gStlAllocator.deallocate(mPixels, mByteCount);
        }
    }

    DefaultPixelBuffer(const DefaultPixelBuffer&) = delete;
    DefaultPixelBuffer& operator=(const DefaultPixelBuffer&) = delete;

    RndPixelCanvas Reshape(
        std::uint32_t width,
        std::uint32_t height,
        std::uint32_t depth) {
        const auto pixelCount =
            static_cast<std::size_t>(width) * height * depth;
        const auto byteCount = pixelCount * sizeof(Hmx::Color);
        if (byteCount != mByteCount) {
            if (mPixels != nullptr) {
                HmxAllocator::gStlAllocator.deallocate(mPixels, mByteCount);
            }
            mPixels = static_cast<Hmx::Color*>(
                HmxAllocator::gStlAllocator.allocate(byteCount));
            mByteCount = byteCount;
        }

        RndPixelCanvas canvas{
            nullptr,
            static_cast<int>(width),
            static_cast<int>(height),
            static_cast<int>(depth),
            0,
            mPixels,
            nullptr,
        };
        RndDefaults::_FillTextureCanvasCheckerboard(
            canvas,
            mSpec.mPrimary,
            mSpec.mCheckerboard ? mSpec.mSecondary : mSpec.mPrimary);
        return canvas;
    }

private:
    const DefaultTextureSpec& mSpec;
    Hmx::Color* mPixels = nullptr;
    std::size_t mByteCount = 0;
};

void ConfigureTextureDescription(
    RndTextureBase::Description& description,
    const DefaultTextureSpec& spec) {
    description.mRequestedFormat.mUsage = static_cast<int>(spec.mUsage);
    description.mRequestedFormat.mWrapMode = 2;
    description.mRequestedFormat.mFilterMode = 1;
    description.mName = spec.mName;
}

void PopulateMip(
    RndPixelData& mip,
    const RndPixelCanvas& image,
    std::int32_t dataFormat) {
    const Vector3i extent{image.mWidth, image.mHeight, image.mDepth};
    mip.Create(extent, dataFormat, nullptr);
    mip.ConvertFrom(image);
}

void PopulateCubeMips(
    RndPixelDataCube& cube,
    const RndPixelCanvas& image,
    std::int32_t dataFormat) {
    for (auto& face : cube.mFaces) {
        PopulateMip(face, image, dataFormat);
    }
}

void CreateTextureFamily(
    RndDefaults::TextureFamily& family,
    const DefaultTextureSpec& spec,
    std::int32_t dataFormat) {
    DefaultPixelBuffer pixels(spec);

    auto image = pixels.Reshape(spec.mExtent, 1, 1);
    RndTexture1D::Description texture1D;
    ConfigureTextureDescription(texture1D, spec);
    PopulateMip(texture1D.mPixels, image, dataFormat);
    family.mTexture1D = RndTexture1D::New(texture1D, nullptr);

    RndTextureArray1D::Description array1D;
    ConfigureTextureDescription(array1D, spec);
    array1D.mPixels = {
        &texture1D.mPixels,
        &texture1D.mPixels + 1,
        &texture1D.mPixels + 1,
    };
    family.mTextureArray1D = RndTextureArray1D::New(array1D, nullptr);

    image = pixels.Reshape(spec.mExtent, spec.mExtent, 1);
    RndTexture2D::Description texture2D;
    ConfigureTextureDescription(texture2D, spec);
    PopulateMip(texture2D.mPixels, image, dataFormat);
    family.mTexture2D = RndTexture2D::New(texture2D);

    RndTextureArray2D::Description array2D;
    ConfigureTextureDescription(array2D, spec);
    array2D.mPixels = {
        &texture2D.mPixels,
        &texture2D.mPixels + 1,
        &texture2D.mPixels + 1,
    };
    family.mTextureArray2D = RndTextureArray2D::New(array2D);

    RndTextureCube::Description cube;
    ConfigureTextureDescription(cube, spec);
    PopulateCubeMips(cube.mCube, image, dataFormat);
    family.mTextureCube = RndTextureCube::New(cube, nullptr);

    RndPixelDataCube arrayCubeState;
    PopulateCubeMips(arrayCubeState, image, dataFormat);
    RndTextureArrayCube::Description arrayCube;
    ConfigureTextureDescription(arrayCube, spec);
    arrayCube.mCubes = {
        &arrayCubeState,
        &arrayCubeState + 1,
        &arrayCubeState + 1,
    };
    family.mTextureArrayCube = RndTextureArrayCube::New(arrayCube, nullptr);

    image = pixels.Reshape(spec.mExtent, spec.mExtent, spec.mExtent);
    RndTexture3D::Description texture3D;
    ConfigureTextureDescription(texture3D, spec);
    PopulateMip(texture3D.mPixels, image, dataFormat);
    family.mTexture3D = RndTexture3D::New(texture3D, nullptr);
}

template <typename Texture>
void ReleaseTexture(Texture*& texture) {
    if (texture != nullptr) {
        delete texture;
        texture = nullptr;
    }
}

void ReleaseTextureFamily(RndDefaults::TextureFamily& family) {
    ReleaseTexture(family.mTexture1D);
    ReleaseTexture(family.mTexture2D);
    ReleaseTexture(family.mTexture3D);
    ReleaseTexture(family.mTextureCube);
    ReleaseTexture(family.mTextureArray1D);
    ReleaseTexture(family.mTextureArray2D);
    ReleaseTexture(family.mTextureArrayCube);
}

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

// Inlined into RndDevice's constructor at 0x6BDB30 in this build.
RndDefaults::RndDefaults()
    : mSceneResource(),
      mTextures(),
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
    mSceneResource->EnterEntity(mSceneResource->mEntity);
    if (mLightingResource) {
        mLightingResource->EnterEntity(mLightingResource->mEntity);
    }
}

// Reconstructed from eboot.elf at 0x6BDE60.
void RndDefaults::_CreateTextures() {
    const auto dataFormat =
        RndFindSupportedDataFormat(kDefaultTextureFormat, kPlatformPS4);
    for (std::size_t index = 0; index < kDefaultTextureSpecs.size(); ++index) {
        CreateTextureFamily(
            mTextures[index], kDefaultTextureSpecs[index], dataFormat);
    }
}

// Reconstructed from the inlined sequence in Init at 0x6BDCA0.
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
    light->mIntensity = 2.0f;

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

    for (auto& family : mTextures) {
        ReleaseTextureFamily(family);
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

// Reconstructed from eboot.elf at 0x6C00F0.
RndTextureBase* RndDefaults::GetTexture(
    RndTextureBase::Type textureType,
    RndDefaultTextureType defaultType) const {
    const auto index = static_cast<std::size_t>(defaultType);
    if (index >= kNumDefaultTextureTypes) {
        return nullptr;
    }

    const auto& family = mTextures[index];
    switch (textureType) {
    case RndTextureBase::kTexture1D:
        return family.mTexture1D;
    case RndTextureBase::kTexture2D:
        return family.mTexture2D;
    case RndTextureBase::kTexture3D:
        return family.mTexture3D;
    case RndTextureBase::kTextureCube:
        return family.mTextureCube;
    case RndTextureBase::kTextureArray1D:
        return family.mTextureArray1D;
    case RndTextureBase::kTextureArray2D:
        return family.mTextureArray2D;
    case RndTextureBase::kTextureArrayCube:
        return family.mTextureArrayCube;
    default:
        return nullptr;
    }
}

// Inlined into _CreateTextures in this build. Pixels in alternating 8x8x8
// cells take the primary and secondary colours; the cell at the origin is
// secondary.
void RndDefaults::_FillTextureCanvasCheckerboard(
    RndPixelCanvas& canvas,
    const Hmx::Color& primary,
    const Hmx::Color& secondary) {
    auto* pixels = const_cast<Hmx::Color*>(canvas.mPixels);
    const auto width = static_cast<std::uint32_t>(canvas.mWidth);
    const auto height = static_cast<std::uint32_t>(canvas.mHeight);
    const auto depth = static_cast<std::uint32_t>(canvas.mDepth);
    for (std::uint32_t z = 0; z < depth; ++z) {
        for (std::uint32_t y = 0; y < height; ++y) {
            for (std::uint32_t x = 0; x < width; ++x) {
                const auto cell = (x / 8U) ^ (y / 8U) ^ (z / 8U);
                pixels[x + width * (y + height * z)] =
                    (cell & 1U) != 0 ? primary : secondary;
            }
        }
    }
}
