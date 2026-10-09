#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "math/color/Color.h"
#include "math/vector/Vector2.h"
#include "math/vector/Vector3.h"
#include "render/drawing/RndDrawInstanceCom.h"
#include "render/meshes/RndMesh.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class RndContext;
class RndLightCom;
class RndMaterialCom;
class RndOcclusionQuery;
class TransCom;

// A lens flare (render/RndLightFlareCom.o, 0x47A3C0-0x47E7AF). Its class
// id is "LightFlare": "Attach to a point or spotlight to render a
// screen-space flare when looking directly at it". The flare draws its
// subflares as screen-space quads, one draw instance each, behind an
// occlusion query on a sphere of "light_source_radius" around the object;
// an attached light drives the color and intensity, and a spotlight fades
// the flare outside its cone. The vtable at 0x1905F10 has 46 slots (slots
// 41-45 are RndDrawInstanceCom's, 0x1937BB0). The object is 376 bytes.
// Field names are not in the reference map; the properties are named after
// the registry (0x47A780).
class RndLightFlareCom : public RndDrawInstanceCom {
public:
    // One flare element, the map's RndLightFlareCom::Subflare (44 bytes,
    // the array's element type). Field names follow the registry's
    // "subflares" item.
    struct Subflare {
        // "type", labelled "Starburst" and "Ghost" in the registry. The
        // enumerator names are not in the reference map.
        enum Type : int {
            // "A bright starburst; intensity independent of size".
            kStarburst = 0,
            // "A ghosted image of the lens aperture; intensity will
            // decrease as size increases".
            kGhost = 1,
        };

        // 0x47A520; also inlined into the array's element constructor
        // (0x47D900).
        Subflare()
            : mType(kGhost),
              mMaterial{0xFFFFFFFFU},
              mScale(1.0F),
              mTint(Hmx::Color::GetWhite()),
              mIntensityMult(1.0F),
              mIntensityScaling(0.0F),
              mDivergence(0.0F),
              mDivergenceAccel(1.5F) {}

        Type mType;
        // "material": "Optional material to use when drawing this
        // subflare", or the null id.
        GameObjectId mMaterial;
        // "scale": the subflare's size relative to "size".
        float mScale;
        // "tint", applied to the vertex color.
        Hmx::Color mTint;
        // "intensity_mult".
        float mIntensityMult;
        // "intensity_scaling": how much of a starburst's intensity change
        // shows as a change of size.
        float mIntensityScaling;
        // "divergence" and "divergence_accel": how far, and how fast, the
        // subflare moves away from the light source along the line through
        // the screen center.
        float mDivergence;
        float mDivergenceAccel;
    };

    // The drawable of one subflare: the scene drawer's batches call it, and
    // it draws its subflare. The vtable at 0x1906080 has 3 slots.
    class SubflareDrawable : public RndDrawable {
    public:
        // Inlined into the vector's growth (0x47E4E0).
        SubflareDrawable() : mOwner(nullptr), mIndex(0) {}
        // Slots 0-1 at 0x47D800 and 0x47D810.
        ~SubflareDrawable() override {}
        // Slot 2. Draws the subflare with the batch's first instance. The
        // map's _DrawBatchImpl(RndContext&, VectorAdapter<RndInstanceData>
        // const&); this build adds the range, which is ignored.
        void _DrawBatchImpl(
            RndContext& context,
            const VectorAdapter<RndInstanceData>& instances,
            const DrawRange& range) override;  // 0x47D4D0

        RndLightFlareCom* mOwner;
        unsigned long mIndex;
    };

    // The run-time state after the properties, the map's
    // RndLightFlareCom::RuntimeData. Its field names are not in the
    // reference map.
    struct RuntimeData {
        // 0x47A5A0; also inlined into the constructor and the copy.
        RuntimeData()
            : mTrans(nullptr),
              mLight(nullptr),
              mIsSpotLight(false),
              mSpotDirection(Vector3::sZero),
              mSpotFalloffParams(Vector3::sZero),
              mOcclusionQuery(nullptr),
              mVisible(false),
              mSubflareDrawables() {}

        // The object's transform, found when the resources load.
        TransCom* mTrans;
        // The light named by "light", found every poll; null without one.
        RndLightCom* mLight;
        // Set by the poll when the light is a spotlight; the spotlight's
        // world z axis and its angular falloff parameters
        // (RndLightSpotCom::GetAngleFalloffParams) then fade the flare.
        bool mIsSpotLight;
        Vector3 mSpotDirection;
        Vector3 mSpotFalloffParams;
        // The query on the light source's sphere, created when the
        // resources load and registered with the scene drawer's query
        // manager on entering.
        RndOcclusionQuery* mOcclusionQuery;
        // Whether the flare draws this frame; cleared while hidden.
        bool mVisible;
        // One drawable per subflare (_SyncSubflares).
        eastl::vector<SubflareDrawable> mSubflareDrawables;
    };

    RndLightFlareCom();  // 0x47A3C0
    // Copies the properties for an imprint; "subflares" through
    // PropArrayBase::_Copy, and the run-time state starts afresh. Inlined
    // into _Imprint (0x47D570).
    RndLightFlareCom(const RndLightFlareCom& other)
        : RndDrawInstanceCom(other),
          mLightSourceRadius(other.mLightSourceRadius),
          mSize(other.mSize),
          mAngleRadians(other.mAngleRadians),
          mLight(other.mLight),
          mColor(other.mColor),
          mIntensity(other.mIntensity),
          mCamCenteringIntensity(other.mCamCenteringIntensity),
          mSpotlightAngleMult(other.mSpotlightAngleMult),
          mRuntime() {
        mSubflares._Copy(other.mSubflares);
    }
    // Slots 0-1: 0x47A600, 0x47A760. Deletes the occlusion query.
    ~RndLightFlareCom() override;

    Symbol GetId() const override;         // slot 4: 0x47D4F0
    Symbol GetClassName() const override;  // slot 5: 0x47D500
    int CurrentRev() const override;       // slot 7: 0x47D510
    bool IsA(Symbol type) const override;  // slot 8: 0x47D530
    Component* AsComponent() override;     // slot 9: 0x47D560
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x47D570
    // Slot 19: a new flare starts with one starburst.
    void _PostCreate() override;  // 0x47BE20
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x47D7E0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x47D7F0
    // Slot 29: creates the occlusion query and finds the transform. The
    // map's _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;  // 0x47BEF0
    // Slot 31: syncs the drawables, registers the query and sizes the draw
    // node's sphere to the light source. The map's _Enter(ObjPtr const&).
    void _Enter() override;  // 0x47BF80
    // Slot 33: follows the light and decides visibility. The map's
    // _Poll(ObjPtr const&).
    void _Poll() override;  // 0x47C260

    // Slot 41: one instance per subflare at the levels in "lods". The map's
    // signature starts with an ObjPtr const&.
    unsigned long _GetNumDrawInstancesImpl(
        RndSceneLod lod) const override;  // 0x47C5A0
    // Slot 42: the renderer's additive default material.
    RndMaterialCom* _GetDefaultMaterial() const override;  // 0x47C5D0
    // Slots 43-44: the instances' state flags, and every poll their
    // drawables, transforms, materials and draw node state. The map's
    // signatures start with an ObjPtr const&.
    void _InitDrawInstancesImpl(
        VectorAdapter<RndDrawInstance>& instances) override;  // 0x47C5F0
    void _SyncDrawInstancesImpl(
        VectorAdapter<RndDrawInstance>& instances) override;  // 0x47C8A0

    // Gives every subflare a drawable that points back at it. Inlined into
    // _Enter.
    void _SyncSubflares();  // 0x47C120
    // Registers the occlusion query with the scene drawer's query manager.
    // Inlined into _Enter. The map's signature takes an ObjPtr const&.
    void _RegisterOcclusionQuery();  // 0x47C220
    // Draws one subflare as a rotated screen-space quad, or two mirrored
    // about the light source when it diverges, unless the light source is
    // occluded for the context's view.
    void _DrawSubflare(
        RndContext& context,
        const RndInstanceData& instance,
        unsigned long index);  // 0x47CE60

    // The class factory. Emitted at 0x405640 with the other render
    // factories.
    static Component* _Create();

    // Registers the class description and the properties, with handlers
    // that convert "angle" (degrees) to "angle_radians" (0x47D880,
    // 0x47D8A0). Not reconstructed: the property metadata and the
    // std::function handlers are not modelled.
    static void _Init(
        PropRegistry& registry,
        ComMetaData& metadata);  // 0x47A780

    static Symbol sId;                  // 0x1A87DF8, "LightFlare"
    // Also "LightFlare". Name not in the reference map.
    static Symbol sClassName;           // 0x1A87E00
    static PropRegistry sPropRegistry;  // 0x1A87E10
    static ComMetaData sMetaData;       // 0x1A87EB0

    // "light_source_radius": the radius of the occlusion test sphere, in
    // world units.
    float mLightSourceRadius;
    // "size": the flare's width and height in screen heights.
    Vector2 mSize;
    // "angle_radians": the counterclockwise rotation of every subflare;
    // "angle" shows it in degrees.
    float mAngleRadians;
    // "light": the light that drives the color and intensity, or the null
    // id.
    GameObjectId mLight;
    // "color": the vertex color.
    Hmx::Color mColor;
    // "intensity".
    float mIntensity;
    // "cam_centering_intensity": how much the flare intensifies as it
    // nears the screen center.
    float mCamCenteringIntensity;
    // "spotlight_angle_mult": how much to fudge the spotlight angle by.
    float mSpotlightAngleMult;
    // "subflares".
    PropArray<Subflare> mSubflares;
    RuntimeData mRuntime;
};

static_assert(sizeof(RndLightFlareCom::Subflare) == 44);
static_assert(offsetof(RndLightFlareCom::Subflare, mMaterial) == 4);
static_assert(offsetof(RndLightFlareCom::Subflare, mScale) == 8);
static_assert(offsetof(RndLightFlareCom::Subflare, mTint) == 12);
static_assert(offsetof(RndLightFlareCom::Subflare, mIntensityMult) == 28);
static_assert(offsetof(RndLightFlareCom::Subflare, mIntensityScaling) == 32);
static_assert(offsetof(RndLightFlareCom::Subflare, mDivergence) == 36);
static_assert(offsetof(RndLightFlareCom::Subflare, mDivergenceAccel) == 40);
static_assert(alignof(RndLightFlareCom::Subflare) == 4);
static_assert(offsetof(RndLightFlareCom::SubflareDrawable, mOwner) == 8);
static_assert(offsetof(RndLightFlareCom::SubflareDrawable, mIndex) == 16);
static_assert(sizeof(RndLightFlareCom::SubflareDrawable) == 24);
static_assert(offsetof(RndLightFlareCom::RuntimeData, mLight) == 8);
static_assert(offsetof(RndLightFlareCom::RuntimeData, mIsSpotLight) == 16);
static_assert(offsetof(RndLightFlareCom::RuntimeData, mSpotDirection) == 20);
static_assert(
    offsetof(RndLightFlareCom::RuntimeData, mSpotFalloffParams) == 32);
static_assert(offsetof(RndLightFlareCom::RuntimeData, mOcclusionQuery) == 48);
static_assert(offsetof(RndLightFlareCom::RuntimeData, mVisible) == 56);
static_assert(
    offsetof(RndLightFlareCom::RuntimeData, mSubflareDrawables) == 64);
static_assert(sizeof(RndLightFlareCom::RuntimeData) == 96);
static_assert(offsetof(RndLightFlareCom, mLightSourceRadius) == 0xC0);
static_assert(offsetof(RndLightFlareCom, mSize) == 0xC4);
static_assert(offsetof(RndLightFlareCom, mAngleRadians) == 0xCC);
static_assert(offsetof(RndLightFlareCom, mLight) == 0xD0);
static_assert(offsetof(RndLightFlareCom, mColor) == 0xD4);
static_assert(offsetof(RndLightFlareCom, mIntensity) == 0xE4);
static_assert(offsetof(RndLightFlareCom, mCamCenteringIntensity) == 0xE8);
static_assert(offsetof(RndLightFlareCom, mSpotlightAngleMult) == 0xEC);
static_assert(offsetof(RndLightFlareCom, mSubflares) == 0xF0);
static_assert(offsetof(RndLightFlareCom, mRuntime) == 0x118);
static_assert(sizeof(RndLightFlareCom) == 0x178);
