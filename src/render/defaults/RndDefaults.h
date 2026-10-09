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
enum RndTargetMode : int;

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
    RndDefaults();  // 0x6BDB30
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
    // The default texture a render target of this mode is sliced from.
    RndTextureBase* GetRTSlicedTexture(
        RndTargetMode targetMode,
        RndDefaultTextureType defaultType);  // 0x6C0050

    float GetLightingShadowOffset() const;       // 0x6BFEA0
    void SetLightingShadowOffset(float offset);  // 0x6BFF70
    void SetLightingScale(float scale);          // 0x6BFBF0
    void SetLightingType(RndDefaultLightingType type);  // 0x6BFA40
    bool IsLightProbeEnabled() const;            // 0x6BFBB0
    void SetLightProbeEnabled(bool enabled);     // 0x6BFBD0

    void _CreateTextures();  // 0x6BDE60
    // Init carries an inlined copy.
    void _CreateComputeBuffers();  // 0x6BEAF0
    // Init carries an inlined copy. Name not in the reference map.
    void _CreateCamera(Entity* entity);  // 0x6BEBC0
    // The map's parameter is EntityPtr const&; this build passes the entity.
    void _CreateMaterials(Entity* entity);  // 0x6BEC50
    // The map's parameter is EntityPtr const&; this build passes the entity.
    void _CreateBackupLighting(Entity* entity);  // 0x6BF4F0
    // Returns false when default_lighting.scene is unusable.
    bool _LoadLighting();       // 0x6BEF40
    void _SyncEnabledLights();  // 0x6BFA60
    // SetLightingScale carries an inlined copy.
    void _SyncLightProbe();  // 0x6BFCF0
    // The map's parameter is ObjPtr const&; this build passes the object.
    void _SyncSpotlight(GameObject* object);  // 0x6BFD40
    // Not in this build: _CreateTextures calls
    // RndTextureUtl::FillCheckerboard (0x6AED00) with an eight-texel cell.
    static void _FillTextureCanvasCheckerboard(
        RndPixelCanvas& canvas,
        const Hmx::Color& primary,
        const Hmx::Color& secondary);

    // Field names are not in the reference map. The default textures are
    // stored shape first: one array per texture shape, each indexed by
    // RndDefaultTextureType.
    ResourcePtr<RndSceneResource> mSceneResource;
    RndTexture1D* mTextures1D[kNumDefaultTextureTypes];
    RndTexture2D* mTextures2D[kNumDefaultTextureTypes];
    RndTexture3D* mTextures3D[kNumDefaultTextureTypes];
    RndTextureCube* mTexturesCube[kNumDefaultTextureTypes];
    RndTextureArray1D* mTexturesArray1D[kNumDefaultTextureTypes];
    RndTextureArray2D* mTexturesArray2D[kNumDefaultTextureTypes];
    RndTextureArrayCube* mTexturesArrayCube[kNumDefaultTextureTypes];
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

static_assert(offsetof(RndDefaults, mTextures1D) == 8);
static_assert(offsetof(RndDefaults, mTextures2D) == 64);
static_assert(offsetof(RndDefaults, mTextures3D) == 120);
static_assert(offsetof(RndDefaults, mTexturesCube) == 176);
static_assert(offsetof(RndDefaults, mTexturesArray1D) == 232);
static_assert(offsetof(RndDefaults, mTexturesArray2D) == 288);
static_assert(offsetof(RndDefaults, mTexturesArrayCube) == 344);
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
