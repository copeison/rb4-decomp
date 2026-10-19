#pragma once

#include <cstddef>

#include "anim/props/PropAnimResource.h"
#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "entity/waveform/Waveform.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"
#include "utl/time/TimeMgr.h"

class PlaybackInfo;
class PropDriver;
class PropKeysBaseCom;

// The component that blends the properties of its entity between frames of
// a property animation by name (anim/PropertyBlenderCom.o,
// 0x2D48E0-0x2D814F; class id "PropertyBlenderCom"): "Used to blend between
// arbitrary PropAnim frames by name." Each named configuration is a frame
// of the "anim" PropAnimResource. When the configuration changes, a second
// animation built at load time holds the properties' current values as its
// only key, and the property drivers blend from it to the new frame over
// "duration" along "blend_curve", exporting "prop_blend_start" and
// "prop_blend_done". The vtable at 0x18F49A0 has 41 slots. The object is
// 312 bytes. The map's methods take the object (ObjPtr const&) first; this
// build's do not. Field names are not in the reference map; the
// registered ones follow the registry.
class PropertyBlenderCom : public Component {
public:
    // A named frame of the animation: "name" and "frame". 16 bytes.
    struct Configuration {
        Symbol mName;
        float mFrame = 0.0F;
    };

    // A configuration with the playback that holds it and whether the
    // property drivers play that playback. Type and field names are not in
    // the reference map.
    struct BlendInfo {
        Configuration mConfiguration;
        // Created once the resources are ready; owned.
        PlaybackInfo* mPlayback;
        // Set while the playback is added to the property drivers.
        bool mPlaying;
    };

    PropertyBlenderCom();  // 0x2D5A30
    // Copies the animation, the curve, the duration and the configurations
    // for an imprint; everything loaded or blending starts afresh and
    // "is_enabled" is set again. Called by _Imprint.
    PropertyBlenderCom(const PropertyBlenderCom& other);  // 0x2D7000
    // Slots 0-1: 0x2D5CF0, 0x2D5F00. Deletes the property drivers and both
    // playbacks.
    ~PropertyBlenderCom() override;

    Symbol GetId() const override;          // slot 4: 0x2D6C20
    Symbol GetClassName() const override;   // slot 5: 0x2D6C30
    int CurrentRev() const override;        // slot 7: 0x2D6C40
    bool IsA(Symbol type) const override;   // slot 8: 0x2D6C60
    Component* AsComponent() override;      // slot 9: 0x2D6C90
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x2D6CA0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x2D6DB0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x2D6DC0
    // Slot 29. Takes the entity's clock, loads "anim" (inlined into the
    // entity or from disk) and creates the empty animation the blends
    // start from, on the seconds timeline. The map's
    // _LoadResources(ObjPtr const&) has this body.
    bool _OnResourcesLoaded() override;  // 0x2D5F70
    // Slot 30. On the first pass, once "anim" loaded: copies its keys into
    // the blend animation, gathers its keys, creates one property driver
    // for each and the two playbacks. True on every pass. The map's
    // _Fixup(bool) may be this slot; the evidence is weak.
    bool _AreResourcesReady(int pass) override;  // 0x2D6100
    // Slot 33. While enabled, starts a blend when the configuration changed
    // or "force_blend" is set, advances it on the animation's timeline,
    // weights the two playbacks by the curve, applies the property drivers
    // and ends the blend once it is complete. The map's
    // _Poll(ObjPtr const&).
    void _Poll() override;  // 0x2D6350

    // Makes the configuration of the name current; an unknown name is
    // reported and ignored. The "current_configuration" setter inlines it.
    void SetConfiguration(Symbol name);  // 0x2D5F20
    // Removes both playbacks from the property drivers when they play.
    // Inlined into _Poll; the copy has no caller.
    void _RemoveCurrentAndPrevInfos();  // 0x2D6BA0

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in anim/AnimInit.o at 0x2AF8E0.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B7320; the
    // placement factory beside it is at 0x2B7350.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x2D48E0

    // "PropertyBlenderCom", constructed by the static initializer at
    // 0x2D8090, which first sets a shared header's null object id to -1
    // (0x19F8100).
    static Symbol sId;  // 0x19F8108
    // Also "PropertyBlenderCom". Name not in the reference map.
    static Symbol sClassName;           // 0x19F8110
    static PropRegistry sPropRegistry;  // 0x19F8120
    static ComMetaData sMetaData;       // 0x19F81C0

    // The loaded "anim".
    ResourcePtr<PropAnimResource> mAnimResource;
    // Its keys, in the order of the property drivers.
    eastl::vector<PropKeysBaseCom*> mSourceKeys;
    // The animation the blends start from: a copy of the keys of "anim"
    // whose first key takes the property's value when a blend starts.
    ResourcePtr<PropAnimResource> mBlendAnim;
    // Its keys, in the same order.
    eastl::vector<PropKeysBaseCom*> mBlendKeys;
    // One property driver for each key of "anim"; owned.
    eastl::vector<PropDriver*> mDrivers;
    // The configuration blended to ("current_configuration",
    // "target_frame"), played from "anim".
    BlendInfo mCurrentInfo;
    // The configuration blended from; its playback holds the blend
    // animation's first frame.
    BlendInfo mPrevInfo;
    // The time since the blend started, on the animation's timeline.
    float mElapsed;
    // "is_enabled". Not saved.
    bool mEnabled;
    // "force_blend". Not saved; the poll clears it.
    bool mForceBlend;
    // The entity's clock.
    TimeMgr::Clock* mClock;
    // "anim", a PropAnimResource.
    ResourcePath mAnim;
    // "blend_curve": the blend's progress over its duration, from 0 to 1.
    Waveform<float> mBlendCurve;
    // "duration", on the animation's timeline; 0 jumps to the
    // configuration.
    float mDuration;
    // "configurations".
    PropArray<Configuration> mConfigurations;
};

static_assert(sizeof(PropertyBlenderCom::Configuration) == 16);
static_assert(sizeof(PropertyBlenderCom::BlendInfo) == 32);
static_assert(offsetof(PropertyBlenderCom, mAnimResource) == 24);
static_assert(offsetof(PropertyBlenderCom, mSourceKeys) == 32);
static_assert(offsetof(PropertyBlenderCom, mBlendAnim) == 64);
static_assert(offsetof(PropertyBlenderCom, mBlendKeys) == 72);
static_assert(offsetof(PropertyBlenderCom, mDrivers) == 104);
static_assert(offsetof(PropertyBlenderCom, mCurrentInfo) == 136);
static_assert(offsetof(PropertyBlenderCom, mPrevInfo) == 168);
static_assert(offsetof(PropertyBlenderCom, mElapsed) == 200);
static_assert(offsetof(PropertyBlenderCom, mEnabled) == 204);
static_assert(offsetof(PropertyBlenderCom, mForceBlend) == 205);
static_assert(offsetof(PropertyBlenderCom, mClock) == 208);
static_assert(offsetof(PropertyBlenderCom, mAnim) == 216);
static_assert(offsetof(PropertyBlenderCom, mBlendCurve) == 224);
static_assert(offsetof(PropertyBlenderCom, mDuration) == 264);
static_assert(offsetof(PropertyBlenderCom, mConfigurations) == 272);
static_assert(sizeof(PropertyBlenderCom) == 312);
