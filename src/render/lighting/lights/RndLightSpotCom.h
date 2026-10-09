#pragma once

#include <cstddef>
#include <cstdint>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "math/geometry/Capsule.h"
#include "math/geometry/Plane.h"
#include "math/geometry/Sphere.h"
#include "math/geometry/TruncatedRoundedCone.h"
#include "math/vector/Vector3.h"
#include "render/lighting/lights/RndLightCom.h"
#include "render/scene/RndSceneResource.h"
#include "render/textures/RndTexture2DResource.h"
#include "utl/containers/FixedVector.h"
#include "utl/text/Symbol.h"

class RndCameraCom;
class RndShaderCBuffer;
class Transform;
class Vector4;

namespace RndComputeBufferStructs {
struct RndCSLightSpot;
}

// A spot light (render/RndLightSpotCom.o, 0x49F2E0-0x4A867F): a cone of
// light between two angles with a falloff, an optional 2D cookie projected
// along the cone, and an optional shadow map in the light manager's spot
// shadow depth array, rendered from a camera entity of its own. The light
// volume is a truncated cone with a rounded end. The vtable at 0x19077E0
// has 54 slots. The object is 664 bytes. Field names are not in the
// reference map; the property members take the names of the properties the
// registry (0x49F9A0) binds to their offsets.
class RndLightSpotCom : public RndLightCom {
public:
    // One element of "quality_settings", per RndQualityLevel. The element
    // type has its own PropArray vtable (0x19079A0). Name not in the
    // reference map.
    struct ShadowQualitySettings {
        bool mCastsShadows;
        bool mSoftenShadows;
    };

    // The runtime state after the properties. The map names the type and
    // its copy constructor; its field names are not in the reference map.
    struct RuntimeData {
        RuntimeData();  // 0x49F4A0
        // The map's RuntimeData(RuntimeData const&): an imprint's run-time
        // state starts afresh, so the copy is a default construction.
        // Inlined into RndLightSpotCom's copy constructor.
        RuntimeData(const RuntimeData&) : RuntimeData() {}

        // Set when the cone changed; the poll moves the draw node's sphere.
        bool mGeometryDirty;
        // The cookie, found inlined or loaded from "cookie" when the
        // resources load.
        ResourcePtr<RndTexture2DResource> mCookieTexture;
        // The constants of the deferred light and the shadow-map shaders.
        RndShaderCBuffer* mDeferredCBuffer;
        RndShaderCBuffer* mShadowGenCBuffer;
        // The light volume, and the larger cone its draw mesh is sized to so
        // that the tessellated mesh still covers the volume.
        TruncatedRoundedCone mCone;
        TruncatedRoundedCone mMeshCone;
        // The volume's world bounds, set by the poll while the light is on.
        Capsule mWorldBounds;
        // The sphere around the volume's rounded end, in world space.
        Sphere mWorldCapSphere;
        // Whether the camera was inside the volume when the untiled draw
        // was prepared, and whether the geometry clip plane applies
        // (_CalcGeoClipPlane).
        bool mCameraInside;
        bool mUseGeoClipPlane;
        Plane mGeoClipPlane;
        // The spot shadow depth layer and the shadow-contribution slot slot
        // 51 acquired, or -1.
        long mShadowMapIndex;
        long mShadowContributionIndex;
        // The four side planes of the shadow frustum in world space
        // (0x4A2E30).
        FixedVector<Plane, 4> mShadowPlanes;
        // The scene entity with the camera the shadow map is drawn from, the
        // camera object's id and its camera.
        ResourcePtr<RndSceneResource> mShadowEntity;
        GameObjectId mShadowCameraId;
        RndCameraCom* mShadowCamera;
    };

    RndLightSpotCom();            // 0x49F2E0
    // Copies the properties for an imprint, "quality_settings" through
    // PropArrayBase::_Copy; the run-time state starts afresh. Inlined into
    // _Imprint (0x4A6CD0). Not in the reference map.
    RndLightSpotCom(const RndLightSpotCom& other)
        : RndLightCom(other),
          mBulbRadius(other.mBulbRadius),
          mFalloffStart(other.mFalloffStart),
          mFalloffEnd(other.mFalloffEnd),
          mFalloffFunction(other.mFalloffFunction),
          mStartAngle(other.mStartAngle),
          mEndAngle(other.mEndAngle),
          mAngleFalloffFunction(other.mAngleFalloffFunction),
          mTruncation(other.mTruncation),
          mCastsShadows(other.mCastsShadows),
          mOnlyFlaggedObjects(other.mOnlyFlaggedObjects),
          mCastContext(other.mCastContext),
          mShadowOffset(other.mShadowOffset),
          mShadowSoftnessMin(other.mShadowSoftnessMin),
          mShadowSoftnessMax(other.mShadowSoftnessMax),
          mMaxShadowSoftnessDistance(other.mMaxShadowSoftnessDistance),
          mCookie(other.mCookie),
          mCookieTiling{other.mCookieTiling[0], other.mCookieTiling[1]},
          mRuntimeData(other.mRuntimeData) {
        mQualitySettings._Copy(other.mQualitySettings);
    }
    ~RndLightSpotCom() override;  // slots 0-1: 0x49F550, 0x49F670

    Symbol GetId() const override;          // slot 4: 0x4A6C50
    Symbol GetClassName() const override;   // slot 5: 0x4A6C60
    int CurrentRev() const override;        // slot 7: 0x4A6C70
    bool IsA(Symbol type) const override;   // slot 8: 0x4A6C90
    Component* AsComponent() override;      // slot 9: 0x4A6CC0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x4A6CD0
    // Slot 19: gives "quality_settings" one element per quality level.
    void _PostCreate() override;  // 0x4A22D0
    // Slot 20: leaves the bookkeeping, and collapses the draw node's sphere
    // onto the object when the component alone is destroyed.
    void _PreDestroy(DestroyType type) override;  // 0x4A22F0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x4A6FA0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x4A6FB0
    // Slot 29: creates the constant buffers, finds the cookie among the
    // entity's inlined resources or loads it (unlinking the old cookie's
    // texture), creates the shadow-map data, syncs the geometry and the
    // draw node's sphere. The map's _LoadResources(ObjPtr const&). Not
    // reconstructed: the inlined-resource lookup and RndTexture2DResource's
    // layout are not modelled.
    bool _OnResourcesLoaded() override;  // 0x4A23D0
    // Slot 33: moves the draw node's sphere after a change, and while the
    // light is on updates its world bounds and its shadow planes.
    void _Poll() override;  // 0x4A2B20

    int _GetCapabilitiesImpl() const override;       // slot 41: 0x4A3050
    Symbol _GetIntensityUnitsImpl() const override;  // slot 42: 0x4A3060
    // Slot 43: tests the world bounds against the frustum and the extra
    // planes. Not reconstructed: the frustum's sphere classification
    // (0x116E9A0) is not modelled.
    bool _FrustumExcludesImpl(const Frustum& frustum, const RndLightCullPlanes& planes) const override;  // 0x4A30B0
    // Slot 44: fills the deferred constants, with the geometry clip plane.
    // Not reconstructed.
    void _PreDrawNoComputeImpl(RndContext& ctx, const RndLightProbeParams& probe) override;  // 0x4A3B40
    // Slot 45: draws the light's cone mesh ("Spot Light"). Not
    // reconstructed.
    void _DrawDeferredNoComputeImpl(
        RndContext& ctx,
        RndBufferCollection& buffers,
        RndSceneDrawTarget& target,
        const RndLightProbeParams& probe) override;  // 0x4A4790
    // Slot 46: appends the light's entry (_GetComputeShaderData). Not
    // reconstructed.
    void _AddToComputeBufferImpl(
        RndQualityLevel quality,
        const RndCameraContext& camera,
        RndComputeBuffer& buffer) override;  // 0x4A4B40
    int _GetTypeImpl() const override;  // slot 47: 0x4A6FC0
    // Slot 48: the cookie resource's texture, or null. Not reconstructed:
    // RndTexture2DResource's layout is not modelled.
    RndTextureBase* _GetCookieTextureImpl() const override;  // 0x4A4C90
    // Slot 49: "casts_shadows" and the quality level's setting.
    bool _CastsShadowsImpl(RndQualityLevel quality) const override;  // 0x4A4CD0
    // Slot 50: draws the shadow map into the light's depth layer. Not
    // reconstructed.
    void _DrawShadowMapImpl(
        RndContext& ctx,
        RndSceneDrawer& drawer,
        const RndCameraContext& camera,
        const RndShowHideContext& showHide,
        VectorAdapter<PodVector<RndDrawInstance>>& instances,
        VectorAdapter<PodVector<RndDrawInstance*>>& sortable,
        RndLightMgrCom& mgr) override;  // 0x4A4CF0
    // Slot 51: takes a shadow depth layer and a shadow-contribution slot
    // when the light casts shadows.
    bool _AcquireDeferredShadowContributionResourcesImpl(bool castsShadows, RndLightMgrCom& mgr) override;  // 0x4A52B0
    // Slot 52: "Shadow Contrib Spotlight", "Shadow Contrib Geo" and
    // "Shadow Blur". Not reconstructed.
    void _GenerateDeferredShadowContributionImpl(
        RndContext& ctx,
        RndBufferCollection& buffers,
        RndSceneDrawTarget& target,
        RndTextureBase* sceneMask,
        RndLightMgrCom& mgr) override;  // 0x4A5320
    // Slot 53: on while the falloff and the cone reach out.
    bool _IsOnImpl() const override;  // 0x4A6920

    // Setters that keep the start no greater than the end, and the
    // registry's change handlers that do the same for an edited property;
    // each rebuilds the cone.
    void SetFalloffStart(float distance);  // 0x49F690
    void _SyncFalloffStart();              // 0x49F6E0
    void SetFalloffEnd(float distance);    // 0x49F720
    void _SyncFalloffEnd();                // 0x49F760
    void SetStartAngle(float angle);       // 0x49F7A0
    void _SyncStartAngle();                // 0x49F7F0
    void SetEndAngle(float angle);         // 0x49F830
    void _SyncEndAngle();                  // 0x49F870
    void _SyncTruncation();                // 0x4A6960

    // The angular falloff's shader parameters for the angles scaled by
    // `scale`: the reciprocal of the cosine range, the negated start
    // cosine over it, and the falloff function (the property's value less
    // one, or the default: 3 for subtracting lights and 1 otherwise).
    // RndLightEnvironCom calls it (0x47C551). Name not in the reference map.
    Vector3 GetAngleFalloffParams(float scale) const;  // 0x49F8B0
    // Sets the cone from half the end angle, the falloff end and
    // "truncation", and sizes the mesh cone so the light globals'
    // tessellated spotlight mesh covers it.
    void _SyncGeometry();  // 0x4A28A0
    // Creates the shadow camera entity with "casts_shadows". Not
    // reconstructed: the entity cloning (0xEE330) is not modelled.
    void _CreateShadowMapData();  // 0x4A2700
    // Rebuilds mShadowPlanes from the cone and the world transform. Name
    // not in the reference map. Not reconstructed.
    void _SyncShadowPlanes();  // 0x4A2E30
    // The light's compute-buffer entry; _SyncShadowGenConstants reuses it.
    // Not reconstructed.
    void _GetComputeShaderData(
        const RndCameraContext& camera,
        RndComputeBufferStructs::RndCSLightSpot& data) const;  // 0x4A3FB0
    // Decides whether the geometry clip plane applies for the camera. The
    // map's _CalcGeoClipPlane(RndCameraContext const&, Vector3 const&); this
    // build passes the camera alone. Not reconstructed.
    void _CalcGeoClipPlane(const RndCameraContext& camera);  // 0x4A4700
    // Writes the shadow-contribution shader's constants. Not reconstructed.
    void _SyncShadowGenConstants(RndContext& ctx);  // 0x4A6450
    // The shadow depth range (near, far, their product and difference) and
    // the projection scale.
    void _CalcShadowParams(Vector4& projection, Vector4& depthRange) const;  // 0x4A6980
    // The shadow camera's transforms. Not reconstructed.
    void _CalcCameraXfms(
        unsigned long index,
        const RndCameraContext& camera,
        Transform& xfm,
        Vector3& position) const;  // 0x4A6A10

    // Registers the class (0x3F08C0). Not reconstructed: the registration
    // helpers it inlines are not modelled.
    static void Init();
    // The class factory.
    static Component* _Create();
    // Not reconstructed: the property metadata and the std::function
    // handlers (0x4A6FD0-0x4A85FF) are not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x49F9A0

    // The object's statics, constructed by the static initializer before
    // 0x4A8680.
    static Symbol sId;  // 0x1A892F8, "LightSpot"
    // Also "LightSpot". Name not in the reference map.
    static Symbol sClassName;           // 0x1A89300
    static PropRegistry sPropRegistry;  // 0x1A89310
    static ComMetaData sMetaData;       // 0x1A893B0

    float mBulbRadius;
    float mFalloffStart;
    float mFalloffEnd;
    std::int32_t mFalloffFunction;
    // The cone's full angles, in radians.
    float mStartAngle;
    float mEndAngle;
    std::int32_t mAngleFalloffFunction;
    float mTruncation;
    // The "shadows" group.
    bool mCastsShadows;
    PropArray<ShadowQualitySettings> mQualitySettings;
    bool mOnlyFlaggedObjects;
    std::int32_t mCastContext;
    // Returned by RndDefaults::GetLightingShadowOffset.
    float mShadowOffset;
    float mShadowSoftnessMin;
    float mShadowSoftnessMax;
    float mMaxShadowSoftnessDistance;
    // "cookie": the texture's path.
    ResourcePath mCookie;
    // "cookie_tiling": u, then v.
    float mCookieTiling[2];
    RuntimeData mRuntimeData;
};

static_assert(sizeof(RndLightSpotCom::ShadowQualitySettings) == 2);
static_assert(offsetof(RndLightSpotCom::RuntimeData, mCookieTexture) == 8);
static_assert(offsetof(RndLightSpotCom::RuntimeData, mCone) == 32);
static_assert(offsetof(RndLightSpotCom::RuntimeData, mMeshCone) == 60);
static_assert(offsetof(RndLightSpotCom::RuntimeData, mWorldBounds) == 88);
static_assert(offsetof(RndLightSpotCom::RuntimeData, mWorldCapSphere) == 144);
static_assert(offsetof(RndLightSpotCom::RuntimeData, mCameraInside) == 160);
static_assert(offsetof(RndLightSpotCom::RuntimeData, mGeoClipPlane) == 164);
static_assert(offsetof(RndLightSpotCom::RuntimeData, mShadowMapIndex) == 184);
static_assert(offsetof(RndLightSpotCom::RuntimeData, mShadowPlanes) == 200);
static_assert(offsetof(RndLightSpotCom::RuntimeData, mShadowEntity) == 288);
static_assert(offsetof(RndLightSpotCom::RuntimeData, mShadowCamera) == 304);
static_assert(sizeof(RndLightSpotCom::RuntimeData) == 312);
static_assert(offsetof(RndLightSpotCom, mBulbRadius) == 232);
static_assert(offsetof(RndLightSpotCom, mFalloffStart) == 236);
static_assert(offsetof(RndLightSpotCom, mFalloffEnd) == 240);
static_assert(offsetof(RndLightSpotCom, mStartAngle) == 248);
static_assert(offsetof(RndLightSpotCom, mTruncation) == 260);
static_assert(offsetof(RndLightSpotCom, mCastsShadows) == 264);
static_assert(offsetof(RndLightSpotCom, mQualitySettings) == 272);
static_assert(offsetof(RndLightSpotCom, mOnlyFlaggedObjects) == 312);
static_assert(offsetof(RndLightSpotCom, mCastContext) == 316);
static_assert(offsetof(RndLightSpotCom, mShadowOffset) == 320);
static_assert(offsetof(RndLightSpotCom, mCookie) == 336);
static_assert(offsetof(RndLightSpotCom, mCookieTiling) == 344);
static_assert(offsetof(RndLightSpotCom, mRuntimeData) == 352);
static_assert(sizeof(RndLightSpotCom) == 664);
