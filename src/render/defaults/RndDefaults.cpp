#include "render/defaults/RndDefaults.h"

#include <array>
#include <cstddef>
#include <cstdint>

#include "math/color/Color.h"
#include "math/vector/Vector3i.h"
#include "os/memory/MemMgr.h"
#include "render/buffers/RndComputeBuffer.h"
#include "render/textures/render_data_format.h"
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

using namespace rb4;

namespace {

constexpr const char* kDefaultComputeBufferName = "Default Compute Buffer";

constexpr const char* kDefaultLightingScene =
    "../../system/data/render/default_lighting.scene";
constexpr const char* kDefaultDirectional = "default_directional";
constexpr const char* kDefaultShadowedSpot = "default_spot_with_shadows";
constexpr const char* kDefaultProbe = "default_probe";
constexpr const char* kBackupDirectional = "default_directional_light";

constexpr float kProbeFalloffStartScale = 2.0f;
constexpr float kProbeFalloffEndScale = 3.0f;
constexpr float kSpotFalloffStartScale = 1.0f;
constexpr float kSpotFalloffEndScale = 2.0f;

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

constexpr RenderDataFormatDescriptor kDefaultTextureFormat{
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

void ReplaceSceneResource(
    RndSceneResource*& destination,
    RndSceneResource* replacement) {
    if (destination != nullptr) {
        rnd_scene_resource_release(destination);
    }
    destination = replacement;
}

RndMaterial* CreateMaterialObject(RndScene& scene, const char* objectName) {
    auto* object = rnd_scene_create_object(scene, objectName);
    if (object == nullptr) {
        return nullptr;
    }

    auto* material = rnd_object_add_material(*object);
    if (material == nullptr) {
        return nullptr;
    }

    rnd_material_set_sharing_type(
        *material, *object, RndMaterialSharingType::kUnique);
    return material;
}

RndMaterial* CreateDefaultMaterial(
    RndScene& scene,
    const char* objectName,
    const char* shaderGraph) {
    auto* material = CreateMaterialObject(scene, objectName);
    if (material != nullptr) {
        rnd_material_set_shader_graph(*material, shaderGraph);
    }
    return material;
}

RndMaterial* CreateDefaultMaterial(
    RndScene& scene,
    const char* objectName,
    const char* shaderGraph,
    RndMaterialBlendMode blendMode) {
    auto* material = CreateMaterialObject(scene, objectName);
    if (material != nullptr) {
        rnd_material_set_blend_mode(*material, blendMode);
        rnd_material_set_shader_graph(*material, shaderGraph);
    }
    return material;
}

void DisableAuthoredLighting(RndScene& scene) {
    const auto objectCount = rnd_scene_object_count(scene);
    for (std::size_t index = 0; index < objectCount; ++index) {
        auto* object = rnd_scene_object_at(scene, index);
        if (object == nullptr) {
            continue;
        }

        if (auto* light = rnd_object_light(*object)) {
            rnd_light_set_enabled(*light, false);
        }
        if (auto* probe = rnd_object_light_probe(*object)) {
            rnd_light_probe_set_enabled(*probe, false);
        }
    }
}

void SetLightListEnabled(
    RndScene& scene,
    const std::vector<RndObjectId>& objectIds,
    bool enabled) {
    for (const auto id : objectIds) {
        auto* object = rnd_scene_find_object(scene, id);
        if (object == nullptr) {
            continue;
        }

        auto* light = rnd_object_light(*object);
        if (light != nullptr) {
            rnd_light_set_enabled(*light, enabled);
        }
    }
}

}  // namespace

RndDefaults::RndDefaults()
    : mSceneResource(nullptr),
      mTextures(),
      mComputeBuffers(),
      mCamera(nullptr),
      mUnlitMaterial(nullptr),
      mAdditiveMaterial(nullptr),
      mLitMaterial(nullptr),
      mTextMaterial(nullptr),
      mParticleMaterial(nullptr),
      mDecalMaterial(nullptr),
      mLightingResource(nullptr),
      mSceneSettings(nullptr),
      mLightProbe(nullptr),
      mLightingType(kDefaultLightingDirectional),
      mLightingScale(100.0f) {
}

// Reconstructed from eboot.elf at 0x6BDC20.
RndDefaults::~RndDefaults() {
    if (mLightingResource != nullptr) {
        rnd_scene_resource_release(mLightingResource);
    }
    if (mSceneResource != nullptr) {
        rnd_scene_resource_release(mSceneResource);
    }
}

// Reconstructed from eboot.elf at 0x6BDCA0.
void RndDefaults::Init(bool initRendering) {
    if (!initRendering && !render_force_default_resources()) {
        return;
    }

    ReplaceSceneResource(mSceneResource, rnd_scene_resource_create());
    if (mSceneResource == nullptr) {
        return;
    }

    auto* scene = rnd_scene_resource_scene(*mSceneResource);
    if (scene == nullptr) {
        return;
    }

    _CreateTextures();
    _CreateComputeBuffers();
    _CreateCamera(*scene);
    _CreateMaterials(*scene);
    if (!_LoadLighting()) {
        _CreateBackupLighting(*scene);
    }

    rnd_scene_resource_finalize_contents(*mSceneResource);
    rnd_scene_resource_finalize(*mSceneResource);
    if (mLightingResource != nullptr) {
        rnd_scene_resource_finalize(*mLightingResource);
    }
}

// Reconstructed from eboot.elf at 0x6BDE60.
void RndDefaults::_CreateTextures() {
    const auto dataFormat =
        render_data_format_resolve(kDefaultTextureFormat, 7);
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
void RndDefaults::_CreateCamera(RndScene& scene) {
    auto* object = rnd_scene_create_object(scene, "default_cam");
    mCamera = object == nullptr ? nullptr : rnd_object_add_camera(*object);
}

// Reconstructed from eboot.elf at 0x6BEC50.
void RndDefaults::_CreateMaterials(RndScene& scene) {
    mUnlitMaterial = CreateDefaultMaterial(
        scene, "default_mat_unlit", kDefaultUnlitShader);
    mAdditiveMaterial = CreateDefaultMaterial(
        scene,
        "default_mat_add",
        kDefaultUnlitShader,
        RndMaterialBlendMode::kAdd);
    mLitMaterial = CreateDefaultMaterial(
        scene, "default_mat_lit", kDefaultLitShader);
    mTextMaterial = CreateDefaultMaterial(
        scene, "default_text_mat", kDefaultTextShader);
    mParticleMaterial = CreateDefaultMaterial(
        scene,
        "default_particle_mat",
        kDefaultParticleShader,
        RndMaterialBlendMode::kAdd);
    mDecalMaterial = CreateDefaultMaterial(
        scene,
        "default_decal_mat",
        kDefaultDecalShader,
        RndMaterialBlendMode::kSource);
}

// Reconstructed from eboot.elf at 0x6BEF40.
bool RndDefaults::_LoadLighting() {
    ReplaceSceneResource(
        mLightingResource, _LoadSceneResource(kDefaultLightingScene));
    if (mLightingResource == nullptr) {
        return false;
    }

    auto* scene = rnd_scene_resource_scene(*mLightingResource);
    if (scene == nullptr) {
        ReplaceSceneResource(mLightingResource, nullptr);
        return false;
    }

    mSceneSettings = rnd_scene_settings(*scene);
    if (mSceneSettings == nullptr) {
        ReplaceSceneResource(mLightingResource, nullptr);
        return false;
    }

    mDirectionalLights.clear();
    mShadowedSpotLights.clear();
    DisableAuthoredLighting(*scene);

    auto* directional = rnd_scene_find_object(*scene, kDefaultDirectional);
    if (directional != nullptr
        && rnd_object_directional_light(*directional) != nullptr) {
        mDirectionalLights.push_back(rnd_object_id(*directional));
    }

    auto* spot = rnd_scene_find_object(*scene, kDefaultShadowedSpot);
    if (spot != nullptr && rnd_object_spot_light(*spot) != nullptr) {
        _SyncSpotlight(*spot);
        mShadowedSpotLights.push_back(rnd_object_id(*spot));
    }

    auto* probe = rnd_scene_find_object(*scene, kDefaultProbe);
    mLightProbe = probe == nullptr ? nullptr : rnd_object_light_probe(*probe);
    if (mLightProbe != nullptr) {
        rnd_light_probe_set_enabled(*mLightProbe, true);
        rnd_light_probe_set_falloff_start(
            *mLightProbe, mLightingScale * kProbeFalloffStartScale);
        rnd_light_probe_set_falloff_end(
            *mLightProbe, mLightingScale * kProbeFalloffEndScale);
    }

    _SyncEnabledLights();
    return true;
}

// Reconstructed from eboot.elf at 0x6BF4F0.
void RndDefaults::_CreateBackupLighting(RndScene& scene) {
    mSceneSettings = rnd_scene_settings(scene);

    auto* object = rnd_scene_create_object(scene, kBackupDirectional);
    if (object == nullptr) {
        return;
    }

    auto* light = rnd_object_add_directional_light(*object);
    if (light == nullptr) {
        return;
    }

    rnd_light_directional_set_intensity(*light, 2.0f);
    rnd_object_set_default_directional_light_transform(*object);
    mDirectionalLights.push_back(rnd_object_id(*object));
    _SyncEnabledLights();
}

// Reconstructed from eboot.elf at 0x6BF860.
void RndDefaults::Terminate() {
    ReplaceSceneResource(mSceneResource, nullptr);
    ReplaceSceneResource(mLightingResource, nullptr);

    mCamera = nullptr;
    mUnlitMaterial = nullptr;
    mAdditiveMaterial = nullptr;
    mLitMaterial = nullptr;
    mTextMaterial = nullptr;
    mParticleMaterial = nullptr;
    mDecalMaterial = nullptr;
    mSceneSettings = nullptr;
    mLightProbe = nullptr;
    mDirectionalLights.clear();
    mShadowedSpotLights.clear();

    for (auto& family : mTextures) {
        ReleaseTextureFamily(family);
    }
    for (auto*& buffer : mComputeBuffers) {
        if (buffer != nullptr) {
            delete buffer;
            buffer = nullptr;
        }
    }
}

// Reconstructed from eboot.elf at 0x6BFA00.
void RndDefaults::Poll() {
    if (mSceneResource != nullptr) {
        rnd_scene_resource_poll(*mSceneResource);
    }
    if (mLightingResource != nullptr) {
        rnd_scene_resource_poll(*mLightingResource);
    }
}

// Reconstructed from eboot.elf at 0x6BFA60.
void RndDefaults::_SyncEnabledLights() {
    if (mLightingResource == nullptr) {
        return;
    }

    auto* scene = rnd_scene_resource_scene(*mLightingResource);
    if (scene == nullptr) {
        return;
    }

    SetLightListEnabled(
        *scene,
        mDirectionalLights,
        mLightingType == kDefaultLightingDirectional);
    SetLightListEnabled(
        *scene,
        mShadowedSpotLights,
        mLightingType == kDefaultLightingShadowedSpot);
}

// Reconstructed from eboot.elf at 0x6BFD40.
void RndDefaults::_SyncSpotlight(RndObject& object) {
    auto* light = rnd_object_spot_light(object);
    if (light == nullptr) {
        return;
    }

    rnd_light_spot_set_falloff_start(
        *light, mLightingScale * kSpotFalloffStartScale);
    rnd_light_spot_set_falloff_end(
        *light, mLightingScale * kSpotFalloffEndScale);
    rnd_object_reset_transform_with_scaled_position(object, mLightingScale);
}

// Reconstructed from eboot.elf at 0x6BFEA0.
float RndDefaults::GetLightingShadowOffset() const {
    if (mLightingResource == nullptr || mShadowedSpotLights.empty()) {
        return 0.0f;
    }

    auto* scene = rnd_scene_resource_scene(*mLightingResource);
    if (scene == nullptr) {
        return 0.0f;
    }

    auto* object = rnd_scene_find_object(*scene, mShadowedSpotLights.front());
    if (object == nullptr) {
        return 0.0f;
    }

    auto* light = rnd_object_spot_light(*object);
    return light == nullptr ? 0.0f : rnd_light_spot_shadow_offset(*light);
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

// Reconstructed from eboot.elf at 0x6C0160.
RndSceneResource* RndDefaults::_LoadSceneResource(const char* path) {
    return resource_manager_load_scene(path, false);
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
