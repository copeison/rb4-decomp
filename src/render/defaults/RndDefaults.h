#pragma once

#include <cstddef>

#include "audio/core/resources/Resource.h"
#include "entity/core/GameObject.h"
#include "render/scene/RndSceneResource.h"
#include "render/textures/RndTextureBase.h"
#include "utl/containers/Vector.h"

namespace Hmx {
class Color;
}

class Entity;
class RndCameraCom;
class RndComputeBuffer;
class RndLightMgrCom;
class RndLightProbeCom;
class RndMaterialCom;
class RndPixelCanvas;
class RndTexture1D;
class RndTexture2D;
class RndTexture3D;
class RndTextureArray1D;
class RndTextureArray2D;
class RndTextureArrayCube;
class RndTextureCube;
struct RndInitParams;

// The built-in textures. Enumerator names are not in the reference map.
enum RndDefaultTextureType : int {
    kDefaultTextureWhite = 0,
    kDefaultTextureBlack = 1,
    kDefaultTextureZero = 2,
    kDefaultTextureFlatNormal = 3,
    kDefaultTextureError = 4,
    kDefaultTextureErrorGreyscale = 5,
    kDefaultTextureErrorNormal = 6,
    kNumDefaultTextureTypes = 7,
};

// Which default lights are enabled. Enumerator names are not in the
// reference map.
enum RndDefaultLightingType : int {
    kDefaultLightingDirectional = 0,
    kDefaultLightingShadowedSpot = 1,
};

// The renderer's default resources: one texture of every shape for each
// RndDefaultTextureType, two small compute buffers, and a scene holding the
// default camera and materials. The default lighting comes from
// default_lighting.scene, or from a single directional light when that scene
// cannot be loaded. Embedded in RndDevice.
class RndDefaults {
public:
    // One texture of each shape for a single RndDefaultTextureType. Name not
    // in the reference map.
    struct TextureFamily {
        RndTexture1D* mTexture1D;
        RndTexture2D* mTexture2D;
        RndTexture3D* mTexture3D;
        RndTextureCube* mTextureCube;
        RndTextureArray1D* mTextureArray1D;
        RndTextureArray2D* mTextureArray2D;
        RndTextureArrayCube* mTextureArrayCube;
    };

    // Inlined into RndDevice's constructor at 0x6BDB30 in this build.
    RndDefaults();
    ~RndDefaults();  // 0x6BDC20

    // The map's signature is Init(); this build reads
    // RndInitParams::mInitRendering.
    void Init(const RndInitParams& params);  // 0x6BDCA0
    void Terminate();                        // 0x6BF860
    void Poll();                             // 0x6BFA00

    // The texture type is an RndTextureBase::Type; the map's parameter is
    // RndTextureType.
    RndTextureBase* GetTexture(
        RndTextureBase::Type textureType,
        RndDefaultTextureType defaultType) const;  // 0x6C00F0
    // Not reconstructed yet. The map's parameter is RndTargetMode.
    RndTextureBase* GetRTSlicedTexture(
        int targetMode,
        RndDefaultTextureType defaultType);

    float GetLightingShadowOffset() const;  // 0x6BFEA0
    // Not reconstructed yet.
    void SetLightingShadowOffset(float offset);
    // Not reconstructed yet.
    void SetLightingScale(float scale);
    // Not reconstructed yet.
    void SetLightingType(RndDefaultLightingType type);
    // Not reconstructed yet.
    bool IsLightProbeEnabled() const;
    // Not reconstructed yet.
    void SetLightProbeEnabled(bool enabled);

    void _CreateTextures();  // 0x6BDE60
    // Inlined into Init at 0x6BDCA0 in this build.
    void _CreateComputeBuffers();
    // Init carries an inlined copy. Name not in the reference map.
    void _CreateCamera(Entity* entity);  // 0x6BEBC0
    // The map's parameter is EntityPtr const&; this build passes the entity.
    void _CreateMaterials(Entity* entity);  // 0x6BEC50
    // The map's parameter is EntityPtr const&; this build passes the entity.
    void _CreateBackupLighting(Entity* entity);  // 0x6BF4F0
    // Returns false when default_lighting.scene is unusable.
    bool _LoadLighting();       // 0x6BEF40
    void _SyncEnabledLights();  // 0x6BFA60
    // Not reconstructed yet.
    void _SyncLightProbe();
    // The map's parameter is ObjPtr const&; this build passes the object.
    void _SyncSpotlight(GameObject* object);  // 0x6BFD40
    // Inlined into _CreateTextures in this build.
    static void _FillTextureCanvasCheckerboard(
        RndPixelCanvas& canvas,
        const Hmx::Color& primary,
        const Hmx::Color& secondary);

    // Field names are not in the reference map.
    ResourcePtr<RndSceneResource> mSceneResource;
    TextureFamily mTextures[kNumDefaultTextureTypes];
    RndComputeBuffer* mComputeBuffers[2];
    RndCameraCom* mCamera;
    RndMaterialCom* mUnlitMaterial;
    RndMaterialCom* mAdditiveMaterial;
    RndMaterialCom* mLitMaterial;
    RndMaterialCom* mTextMaterial;
    RndMaterialCom* mParticleMaterial;
    RndMaterialCom* mDecalMaterial;
    ResourcePtr<RndSceneResource> mLightingResource;
    RndLightMgrCom* mLightMgr;
    RndLightProbeCom* mLightProbe;
    eastl::vector<GameObjectId> mDirectionalLights;
    eastl::vector<GameObjectId> mShadowedSpotLights;
    RndDefaultLightingType mLightingType;
    float mLightingScale;
};

static_assert(sizeof(RndDefaults::TextureFamily) == 56);
static_assert(offsetof(RndDefaults, mTextures) == 8);
static_assert(offsetof(RndDefaults, mComputeBuffers) == 400);
static_assert(offsetof(RndDefaults, mCamera) == 416);
static_assert(offsetof(RndDefaults, mUnlitMaterial) == 424);
static_assert(offsetof(RndDefaults, mLightingResource) == 472);
static_assert(offsetof(RndDefaults, mLightMgr) == 480);
static_assert(offsetof(RndDefaults, mLightProbe) == 488);
static_assert(offsetof(RndDefaults, mDirectionalLights) == 496);
static_assert(offsetof(RndDefaults, mShadowedSpotLights) == 528);
static_assert(offsetof(RndDefaults, mLightingType) == 560);
static_assert(offsetof(RndDefaults, mLightingScale) == 564);
static_assert(sizeof(RndDefaults) == 568);
