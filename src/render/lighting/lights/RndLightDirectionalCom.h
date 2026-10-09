#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "math/transform/Transform.h"
#include "render/lighting/lights/RndLightCom.h"
#include "utl/text/Symbol.h"

class RndCameraCom;
class RndSceneResource;
class RndShaderCBuffer;
class RndTexture2DResource;

namespace RndComputeBufferStructs {

// A directional light as the tiled-lighting shaders read it: 112 bytes,
// filled by RndLightDirectionalCom::_GetComputeShaderData. The map names
// the type; the field names are not in the reference map.
struct RndCSLightDirectional {
    // The color times the intensity and the light manager's master
    // multiplier; a type 3 light blends from white instead.
    float mColor[3];
    // "volumetric", as 0 or 1.
    int mVolumetric;
    // The direction the light travels, in camera space, normalized.
    float mDirection[3];
    int mIlluminationType;
    // The light-wrap factors, from "light_wrap".
    float mLightWrapParams[2];
    unsigned int mEnvironBits;
    unsigned int mPad;
    // The reciprocal cookie tile size; v is negated.
    float mCookieScale[2];
    // The cookie's texture array in the bits from 11 and its slice below,
    // or -1.
    int mCookieIndex;
    unsigned int mPad2;
    // The camera-to-light transform, as three rows of a 3x4 matrix.
    float mCamToLightXfm[3][4];
};

static_assert(offsetof(RndCSLightDirectional, mDirection) == 0x10);
static_assert(offsetof(RndCSLightDirectional, mLightWrapParams) == 0x20);
static_assert(offsetof(RndCSLightDirectional, mEnvironBits) == 0x28);
static_assert(offsetof(RndCSLightDirectional, mCookieScale) == 0x30);
static_assert(offsetof(RndCSLightDirectional, mCookieIndex) == 0x38);
static_assert(offsetof(RndCSLightDirectional, mCamToLightXfm) == 0x40);
static_assert(sizeof(RndCSLightDirectional) == 0x70);

}  // namespace RndComputeBufferStructs

// A directional light (render/RndLightDirectionalCom.o, 0x470600 to
// 0x47730F): light along the object's -y axis with an optional tiled 2D
// cookie and cascaded shadow maps, rendered from a camera entity of its
// own. Its class id is "LightDirectional". The vtable at 0x19051B0 has 54
// slots. The object is 672 bytes. Field names are not in the reference map;
// the property members take the names of the properties the registry
// (0x4709A0) binds to their offsets.
class RndLightDirectionalCom : public RndLightCom {
public:
    // One element of "quality_settings", per RndQualityLevel. The element
    // type has its own PropArray vtable (0x1905370). Name not in the
    // reference map.
    struct ShadowQualitySettings {
        bool mCastsShadows;
        bool mSoftenShadows;
    };

    // One shadow cascade's camera: its world transform and the near and far
    // planes and height of its orthographic view, computed by
    // _CalcCascadeCamera and given to the shadow camera. Name not in the
    // reference map.
    struct ShadowCascade {
        Transform mXfm;
        float mNearPlane;
        float mFarPlane;
        float mOrthoHeight;
    };

    // The runtime state after the properties. The map names a RuntimeData
    // type; its field names are not in the reference map.
    struct RuntimeData {
        // The map's RuntimeData(): no buffers, camera or slots, and identity
        // cascades. Inlined into the constructor (0x470600) and the copy
        // constructor (0x475A30).
        RuntimeData()
            : mDeferredCBuffer(nullptr),
              mShadowGenCBuffer(nullptr),
              mShadowCamera(nullptr),
              mShadowMapLayer(-1),
              mShadowContributionIndex(-1) {
            for (ShadowCascade& cascade : mCascades) {
                cascade.mXfm = Transform::sID;
                cascade.mNearPlane = 0.0F;
                cascade.mFarPlane = 0.0F;
                cascade.mOrthoHeight = 0.0F;
            }
        }

        // The cookie, loaded from "cookie" when the resources load.
        ResourcePtr<RndTexture2DResource> mCookieTexture;
        // The constants of the deferred light and of the shadow-map shader.
        RndShaderCBuffer* mDeferredCBuffer;
        RndShaderCBuffer* mShadowGenCBuffer;
        // The entity the shadow camera lives in, and its camera.
        ResourcePtr<RndSceneResource> mShadowEntity;
        RndCameraCom* mShadowCamera;
        // The first layer of the spot shadow depth array the cascades
        // render into this frame, and the shadow-contribution slot, or -1.
        long mShadowMapLayer;
        long mShadowContributionIndex;
        ShadowCascade mCascades[4];
    };

    RndLightDirectionalCom();  // 0x470600
    // Copies the properties for an imprint, the arrays through
    // PropArrayBase::_Copy; the run-time state starts afresh. Not in the
    // reference map, whose build inlines it into _Imprint.
    RndLightDirectionalCom(const RndLightDirectionalCom& other);  // 0x475A30

    // Slots 0-1: 0x470810, 0x470980.
    ~RndLightDirectionalCom() override;
    Symbol GetId() const override;          // slot 4: 0x475730
    Symbol GetClassName() const override;   // slot 5: 0x475740
    int CurrentRev() const override;        // slot 7: 0x475750
    bool IsA(Symbol type) const override;   // slot 8: 0x475770
    Component* AsComponent() override;      // slot 9: 0x4757A0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x4757B0
    // Slot 19 at 0x473920: four cascade distances and one quality setting
    // per quality level.
    void _PostCreate() override;
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x4758C0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x4758D0
    // Slot 29 at 0x473960: creates the constants, loads the cookie and the
    // shadow camera. The map's _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;
    // Slot 33 at 0x473D30.
    void _Poll() override;

    int _GetCapabilitiesImpl() const override;        // slot 41: 0x473D40
    Symbol _GetIntensityUnitsImpl() const override;   // slot 42: 0x473D50
    // Slot 44 at 0x473DA0: writes the light's deferred constants, and the
    // probe's falloff and transform when it is drawn with a probe.
    void _PreDrawNoComputeImpl(RndContext& ctx, const RndLightProbeParams& probe) override;
    // Slot 45 at 0x474530: draws the light as a full-screen quad, under the
    // GPU timer "Directional Light" or "Directional Light + Probe".
    void _DrawDeferredNoComputeImpl(
        RndContext& ctx,
        RndBufferCollection& buffers,
        RndSceneDrawTarget& target,
        const RndLightProbeParams& probe) override;
    // Slot 46 at 0x4747E0.
    void _AddToComputeBufferImpl(
        RndQualityLevel quality,
        const RndCameraContext& camera,
        RndComputeBuffer& buffer) override;
    int _GetTypeImpl() const override;                        // slot 47: 0x4758E0
    RndTextureBase* _GetCookieTextureImpl() const override;   // slot 48: 0x4748C0
    bool _CastsShadowsImpl(RndQualityLevel quality) const override;  // slot 49: 0x474900
    // Slot 50 at 0x474920: renders each cascade's shadow depth from the
    // shadow camera into the spot shadow depth array. Not reconstructed.
    void _DrawShadowMapImpl(
        RndContext& ctx,
        RndSceneDrawer& drawer,
        const RndCameraContext& camera,
        const RndShowHideContext& showHide,
        VectorAdapter<PodVector<RndDrawInstance>>& instances,
        VectorAdapter<PodVector<RndDrawInstance*>>& sortable,
        RndLightMgrCom& mgr) override;
    // Slot 51 at 0x474FE0: reserves one shadow-map layer per cascade and a
    // shadow-contribution slot.
    bool _AcquireDeferredShadowContributionResourcesImpl(bool castsShadows, RndLightMgrCom& mgr) override;
    // Slot 52 at 0x475050: resolves the cascades into the light's
    // shadow-contribution slot under the GPU timer "Shadow Contrib
    // Directional". Not reconstructed.
    void _GenerateDeferredShadowContributionImpl(
        RndContext& ctx,
        RndBufferCollection& buffers,
        RndSceneDrawTarget& target,
        RndTextureBase* sceneMask,
        RndLightMgrCom& mgr) override;

    // Fills the light's entry of the tiled lights' buffer for the camera.
    // The map's _GetComputeShaderData(RndCameraContext const&,
    // RndComputeBufferStructs::RndCSLightDirectional&) const.
    void _GetComputeShaderData(
        const RndCameraContext& camera,
        RndComputeBufferStructs::RndCSLightDirectional& data) const;  // 0x4741D0
    // Creates the shadow camera's entity and the shadow-map constants when
    // the light casts shadows: a new RndSceneResource's entity gets an
    // object with a RndCameraCom, built in temporary memory and copied out
    // with Entity::Copy (0xEE330), which Entity.h does not declare yet. Name
    // not in the reference map. Not reconstructed.
    void _SyncShadowResources();  // 0x473B50
    // Fits the cascade's orthographic frustum around the camera's slice of
    // the view and the occluders before it. Name not in the reference map.
    // Not reconstructed.
    bool _CalcCascadeFrustum(
        unsigned int cascade,
        const RndCameraContext& camera,
        Frustum& frustum);  // 0x4732F0
    // The cascade camera for the fitted frustum. Name not in the reference
    // map. Not reconstructed.
    ShadowCascade _CalcCascadeCamera(const Frustum& frustum) const;  // 0x473600
    // Points the shadow camera at the cascade and records it. Name not in
    // the reference map. Not reconstructed.
    void _SetCascadeCamera(unsigned int cascade, const Frustum& frustum);  // 0x474EE0

    // Not reconstructed: the property metadata, and the std::function
    // handlers at 0x475950-0x475A2B that keep the cascade distances and
    // the softness range ordered, are not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x4709A0

    // The class symbol, "LightDirectional", constructed by the static
    // initializer at 0x477240.
    static Symbol sId;  // 0x1A87638
    // The class's second symbol, with the same string, which the component
    // factory is given. Name not in the reference map.
    static Symbol sClassName;  // 0x1A87640
    static PropRegistry sPropRegistry;  // 0x1A87650
    static ComMetaData sMetaData;       // 0x1A876F0

    // "cookie": the texture projected along the light, and its tile size in
    // world units ("cookie_tile_size", u then v).
    ResourcePath mCookie;
    float mCookieTileSize[2];
    // The "shadows" group.
    bool mCastsShadows;
    // The farthest distance from the scene camera that receives shadows.
    float mMaxDistance;
    // "cascades": the number of cascades, and where each ends as a fraction
    // of mMaxDistance.
    unsigned int mNumCascades;
    PropArray<float> mDistanceFracs;
    float mMaxOffscreenOccluderDist;
    // "softening".
    bool mSoften;
    float mMinSoftness;
    float mMaxSoftness;
    float mMaxSoftnessDist;
    bool mOnlyFlaggedObjects;
    // "cast_context" flags: 1 volumetrics, 2 geometry.
    unsigned int mCastContext;
    float mShadowOffset;
    PropArray<ShadowQualitySettings> mQualitySettings;
    RuntimeData mRuntimeData;
};

static_assert(sizeof(RndLightDirectionalCom::ShadowCascade) == 60);
static_assert(offsetof(RndLightDirectionalCom::RuntimeData, mShadowCamera) == 0x20);
static_assert(offsetof(RndLightDirectionalCom::RuntimeData, mShadowMapLayer) == 0x28);
static_assert(offsetof(RndLightDirectionalCom::RuntimeData, mCascades) == 0x38);
static_assert(sizeof(RndLightDirectionalCom::RuntimeData) == 0x128);
static_assert(offsetof(RndLightDirectionalCom, mCookie) == 0xE8);
static_assert(offsetof(RndLightDirectionalCom, mCookieTileSize) == 0xF0);
static_assert(offsetof(RndLightDirectionalCom, mCastsShadows) == 0xF8);
static_assert(offsetof(RndLightDirectionalCom, mMaxDistance) == 0xFC);
static_assert(offsetof(RndLightDirectionalCom, mNumCascades) == 0x100);
static_assert(offsetof(RndLightDirectionalCom, mDistanceFracs) == 0x108);
static_assert(offsetof(RndLightDirectionalCom, mMaxOffscreenOccluderDist) == 0x130);
static_assert(offsetof(RndLightDirectionalCom, mSoften) == 0x134);
static_assert(offsetof(RndLightDirectionalCom, mMaxSoftnessDist) == 0x140);
static_assert(offsetof(RndLightDirectionalCom, mOnlyFlaggedObjects) == 0x144);
static_assert(offsetof(RndLightDirectionalCom, mCastContext) == 0x148);
static_assert(offsetof(RndLightDirectionalCom, mShadowOffset) == 0x14C);
static_assert(offsetof(RndLightDirectionalCom, mQualitySettings) == 0x150);
static_assert(offsetof(RndLightDirectionalCom, mRuntimeData) == 0x178);
static_assert(sizeof(RndLightDirectionalCom) == 0x2A0);
