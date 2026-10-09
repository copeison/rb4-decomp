#pragma once

#include <cstddef>
#include <cstdint>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "math/geometry/Plane.h"
#include "render/lighting/lights/RndLightCom.h"
#include "render/scene/RndSceneResource.h"
#include "render/textures/RndTextureCubeResource.h"
#include "utl/text/Symbol.h"

class RndCameraCom;
class RndShaderCBuffer;
class RndTextureCube;
class Vector3;

namespace RndComputeBufferStructs {
struct RndCSLightPoint;
}

// A point light (render/RndLightPointCom.o, 0x490060-0x496C9F): a sphere of
// light with a falloff, an optional cube cookie and an optional cube shadow
// map rendered from a camera entity of its own. The vtable at 0x1906B90
// has 54 slots. The object is 424 bytes. Field names are not in the
// reference map; the property members take the names of the properties the
// registry (0x490340) binds to their offsets.
class RndLightPointCom : public RndLightCom {
public:
    // One element of "quality_settings", per RndQualityLevel. The element
    // type has its own PropArray vtable (0x1906D50). Name not in the
    // reference map.
    struct ShadowQualitySettings {
        bool mCastsShadows;
        bool mSoftenShadows;
    };

    // The runtime state after the properties. The map names the type
    // (constructor 0x51 bytes, inlined into RndLightPointCom's); its field
    // names are not in the reference map.
    struct RuntimeData {
        RuntimeData()
            : mSphereDirty(false),
              mDeferredCBuffer(nullptr),
              mShadowGenCBuffer(nullptr),
              mUseGeoClipPlane(false),
              mShadowMap(nullptr),
              mShadowContributionIndex(-1),
              mShadowCameraId{0xFFFFFFFFU},
              mShadowCamera(nullptr) {
            mGeoClipPlane.c = 0.0F;
        }

        // Set when the falloff end changed; the poll moves the draw node's
        // sphere.
        bool mSphereDirty;
        // The cookie, loaded from "cookie" when the resources load.
        ResourcePtr<RndTextureCubeResource> mCookieTexture;
        // The constants of the deferred light and the shadow-map shaders.
        RndShaderCBuffer* mDeferredCBuffer;
        RndShaderCBuffer* mShadowGenCBuffer;
        // The plane _CalcGeoClipPlane builds against the camera for the
        // untiled draw, and whether it applies.
        bool mUseGeoClipPlane;
        Plane mGeoClipPlane;
        // The shadow cube map, created with "casts_shadows".
        RndTextureCube* mShadowMap;
        // The shadow-contribution slot slot 51 acquired, or -1.
        long mShadowContributionIndex;
        // The scene entity with the camera the shadow map is drawn from, the
        // camera object's id and its camera.
        ResourcePtr<RndSceneResource> mShadowEntity;
        GameObjectId mShadowCameraId;
        RndCameraCom* mShadowCamera;
    };

    RndLightPointCom();            // 0x490060
    // Copies the properties for an imprint, "quality_settings" through
    // PropArrayBase::_Copy; the run-time state starts afresh. Inlined into
    // _Imprint (0x495620). Not in the reference map.
    RndLightPointCom(const RndLightPointCom& other)
        : RndLightCom(other),
          mBulbRadius(other.mBulbRadius),
          mFalloffStart(other.mFalloffStart),
          mFalloffEnd(other.mFalloffEnd),
          mFalloffFunction(other.mFalloffFunction),
          mCookie(other.mCookie),
          mCastsShadows(other.mCastsShadows),
          mOnlyFlaggedObjects(other.mOnlyFlaggedObjects),
          mShadowMapResolution(other.mShadowMapResolution),
          mShadowOffset(other.mShadowOffset),
          mShadowSoftnessMin(other.mShadowSoftnessMin),
          mShadowSoftnessMax(other.mShadowSoftnessMax),
          mMaxShadowSoftnessDistance(other.mMaxShadowSoftnessDistance),
          mRuntimeData() {
        mQualitySettings._Copy(other.mQualitySettings);
    }
    ~RndLightPointCom() override;  // slots 0-1: 0x4901E0, 0x490320

    Symbol GetId() const override;          // slot 4: 0x4955A0
    Symbol GetClassName() const override;   // slot 5: 0x4955B0
    int CurrentRev() const override;        // slot 7: 0x4955C0
    bool IsA(Symbol type) const override;   // slot 8: 0x4955E0
    Component* AsComponent() override;      // slot 9: 0x495610
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x495620
    // Slot 19: gives "quality_settings" one element per quality level.
    void _PostCreate() override;  // 0x4921E0
    // Slot 20: leaves the bookkeeping, and collapses the draw node's sphere
    // onto the object when the component alone is destroyed.
    void _PreDestroy(DestroyType type) override;  // 0x492200
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x4958A0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x4958B0
    // Slot 29: creates the constant buffers, loads the cookie, creates the
    // shadow map and syncs the draw node's sphere. The map's
    // _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;  // 0x4922E0
    // Slot 33: moves the draw node's sphere after a falloff change. The
    // map's _SyncLocalSphere(ObjPtr const&) is inlined.
    void _Poll() override;  // 0x492920

    int _GetCapabilitiesImpl() const override;       // slot 41: 0x4929D0
    Symbol _GetIntensityUnitsImpl() const override;  // slot 42: 0x4929E0
    // Slot 43: whether the frustum, or one of the extra planes, excludes the
    // draw node's world sphere. Not reconstructed: the frustum's sphere
    // classification (0x116E9A0) is not modelled.
    bool _FrustumExcludesImpl(const Frustum& frustum, const RndLightCullPlanes& planes) const override;  // 0x492A30
    // Slot 44: fills the deferred constants, with the geometry clip plane.
    // Not reconstructed.
    void _PreDrawNoComputeImpl(RndContext& ctx, const RndLightProbeParams& probe) override;  // 0x492B10
    // Slot 45: draws the light's sphere ("Point Light"). Not reconstructed.
    void _DrawDeferredNoComputeImpl(
        RndContext& ctx,
        RndBufferCollection& buffers,
        RndSceneDrawTarget& target,
        const RndLightProbeParams& probe) override;  // 0x4936D0
    // Slot 46: appends the light's entry (_GetComputeShaderData). Not
    // reconstructed.
    void _AddToComputeBufferImpl(
        RndQualityLevel quality,
        const RndCameraContext& camera,
        RndComputeBuffer& buffer) override;  // 0x493A90
    int _GetTypeImpl() const override;                        // slot 47: 0x4958C0
    RndTextureBase* _GetCookieTextureImpl() const override;   // slot 48: 0x493BE0
    // Slot 49: point lights do not cast shadows in this build.
    bool _CastsShadowsImpl(RndQualityLevel quality) const override;  // 0x493C20
    // Slot 50: draws the six faces of the shadow cube. Not reconstructed.
    void _DrawShadowMapImpl(
        RndContext& ctx,
        RndSceneDrawer& drawer,
        const RndCameraContext& camera,
        const RndShowHideContext& showHide,
        VectorAdapter<PodVector<RndDrawInstance>>& instances,
        VectorAdapter<PodVector<RndDrawInstance*>>& sortable,
        RndLightMgrCom& mgr) override;  // 0x493C30
    // Slot 51: takes a shadow-contribution slot when the light casts
    // shadows.
    bool _AcquireDeferredShadowContributionResourcesImpl(bool castsShadows, RndLightMgrCom& mgr) override;  // 0x493FB0
    // Slot 52: "Shadow Contrib Point Light", "Shadow Contrib Geo" and
    // "Shadow Blur". Not reconstructed.
    void _GenerateDeferredShadowContributionImpl(
        RndContext& ctx,
        RndBufferCollection& buffers,
        RndSceneDrawTarget& target,
        RndTextureBase* sceneMask,
        RndLightMgrCom& mgr) override;  // 0x493FF0
    // Slot 53: on while the falloff reaches out.
    bool _IsOnImpl() const override;  // 0x4954A0

    // The registry's change handlers for the two falloff properties.
    void _SyncFalloffStart();  // 0x4954C0
    void _SyncFalloffEnd();    // 0x4954F0
    // Releases the shadow map and, with "casts_shadows", creates the
    // "Point Light Shadowmap" cube at the "shadow_map_resolution" size and
    // the camera entity. Not reconstructed: the texture description and
    // the entity creation are not modelled.
    void _CreateShadowMapData();  // 0x492630
    // The plane through the light that keeps its sphere from being drawn
    // behind the camera. Not reconstructed.
    void _CalcGeoClipPlane(const RndCameraContext& camera, const Vector3& cameraPos);  // 0x492EB0
    // The light's compute-buffer entry. Not reconstructed.
    void _GetComputeShaderData(
        const RndCameraContext& camera,
        RndComputeBufferStructs::RndCSLightPoint& data) const;  // 0x493340
    // Writes the shadow-map shader's constants for the cube faces. Not
    // reconstructed.
    void _SyncShadowGenConstants(RndContext& ctx);  // 0x495100

    // Registers the class (0x3F0120). Not reconstructed: the registration
    // helpers it inlines are not modelled.
    static void Init();
    // The class factory.
    static Component* _Create();
    // Not reconstructed: the property metadata and the std::function
    // handlers (0x4958D0-0x4959E0) are not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x490340

    // The object's statics, constructed by its static initializer before
    // 0x496CA0.
    static Symbol sId;  // 0x1A888C8, "LightPoint"
    // Also "LightPoint". Name not in the reference map.
    static Symbol sClassName;           // 0x1A888D0
    static PropRegistry sPropRegistry;  // 0x1A888E0
    static ComMetaData sMetaData;       // 0x1A88980

    float mBulbRadius;
    float mFalloffStart;
    float mFalloffEnd;
    std::int32_t mFalloffFunction;
    // "cookie": the cube texture's path.
    ResourcePath mCookie;
    // The "shadows" group.
    bool mCastsShadows;
    PropArray<ShadowQualitySettings> mQualitySettings;
    bool mOnlyFlaggedObjects;
    // 256, 512 or 1024 texels (0x1289120).
    std::int32_t mShadowMapResolution;
    float mShadowOffset;
    float mShadowSoftnessMin;
    float mShadowSoftnessMax;
    float mMaxShadowSoftnessDistance;
    RuntimeData mRuntimeData;
};

static_assert(sizeof(RndLightPointCom::ShadowQualitySettings) == 2);
static_assert(offsetof(RndLightPointCom::RuntimeData, mCookieTexture) == 8);
static_assert(offsetof(RndLightPointCom::RuntimeData, mUseGeoClipPlane) == 32);
static_assert(offsetof(RndLightPointCom::RuntimeData, mGeoClipPlane) == 36);
static_assert(offsetof(RndLightPointCom::RuntimeData, mShadowMap) == 56);
static_assert(offsetof(RndLightPointCom::RuntimeData, mShadowEntity) == 72);
static_assert(offsetof(RndLightPointCom::RuntimeData, mShadowCamera) == 88);
static_assert(sizeof(RndLightPointCom::RuntimeData) == 96);
static_assert(offsetof(RndLightPointCom, mBulbRadius) == 232);
static_assert(offsetof(RndLightPointCom, mFalloffEnd) == 240);
static_assert(offsetof(RndLightPointCom, mCookie) == 248);
static_assert(offsetof(RndLightPointCom, mCastsShadows) == 256);
static_assert(offsetof(RndLightPointCom, mQualitySettings) == 264);
static_assert(offsetof(RndLightPointCom, mOnlyFlaggedObjects) == 304);
static_assert(offsetof(RndLightPointCom, mShadowMapResolution) == 308);
static_assert(offsetof(RndLightPointCom, mShadowOffset) == 312);
static_assert(offsetof(RndLightPointCom, mMaxShadowSoftnessDistance) == 324);
static_assert(offsetof(RndLightPointCom, mRuntimeData) == 328);
static_assert(sizeof(RndLightPointCom) == 424);
