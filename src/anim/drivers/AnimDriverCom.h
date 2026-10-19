#pragma once

#include <cstddef>
#include <functional>

#include "anim/core/AnimInstID.h"
#include "anim/drivers/CharAnimDriver.h"
#include "anim/drivers/PropAnimDriver.h"
#include "entity/core/Component.h"
#include "entity/props/PropMetadata.h"
#include "entity/resources/Resource.h"
#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"
#include "utl/time/TimeMgr.h"

class AnimDriver;
class AnimOptions;
class PlaybackInfo;

// The root object's component that plays the entity's animations
// (anim/AnimDriverCom.o, 0x3297F0-0x32C6B6; class id "AnimDriver", alias
// "AnimDriverCom"). It owns a driver for property animations and one for
// character animations and clips, both on the entity's clock, polls them
// and keeps the playbacks started under a name. AnimCom and the state
// graphs play through it. The vtable at 0x18FA1E8 has 41 slots. The object
// is 592 bytes.
class AnimDriverCom : public Component {
public:
    // The drivers and the named playbacks. A copy starts empty, as the
    // map's RuntimeData(RuntimeData const&) does. Field names are not in the
    // reference map.
    struct RuntimeData {
        // Inlined into the constructor (0x329820).
        RuntimeData() : mClock(nullptr) {}
        // Inlined into _Imprint (0x32BB40).
        RuntimeData(const RuntimeData& other) : mClock(nullptr) {
            static_cast<void>(other);
        }

        // The entity's clock, set when the resources load.
        TimeMgr::Clock* mClock;
        PropAnimDriver mPropAnimDriver;
        CharAnimDriver mCharAnimDriver;
        // The playbacks started with AnimOptions::mName.
        eastl::map<Symbol, AnimInstID> mNamedAnims;
    };

    AnimDriverCom();  // 0x329820
    // Copies the default timeline for an imprint. Inlined into _Imprint
    // (0x32BB40).
    AnimDriverCom(const AnimDriverCom& other)
        : Component(other), mDefaultTimeline(other.mDefaultTimeline), mRuntime(other.mRuntime) {}
    // Slots 0-1: 0x32A000, 0x32BA60.
    ~AnimDriverCom() override {}

    // Slot 2: "start_anim", "stop_anim" and "apply_propanim_frame", then
    // the component's messages.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x32B790
    Symbol GetId() const override;          // slot 4: 0x32BAC0
    Symbol GetClassName() const override;   // slot 5: 0x32BAD0
    int CurrentRev() const override;        // slot 7: 0x32BAE0
    bool IsA(Symbol type) const override;   // slot 8: 0x32BB00
    Component* AsComponent() override;      // slot 9: 0x32BB30
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x32BB40
    // Slot 18: the driver comes before TransCom.
    void _GetComponentOrderDeps(
        eastl::vector<Symbol>& follows,
        eastl::vector<Symbol>& precedes) override;  // 0x32A050
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x32BCF0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x32BD00
    // Slot 29: takes the entity's clock and enters the drivers. The map's
    // _PostCreate(ObjPtr const&) and _Enter(ObjPtr const&) did this.
    bool _OnResourcesLoaded() override;  // 0x32A110
    // Slot 32: an object or a component that goes away stops every
    // playback.
    void _Exit(DestroyType type) override;  // 0x32A1D0
    // Slots 33 and 38: poll the drivers in both modes.
    void _Poll() override;      // 0x32A170
    void _EditPoll() override;  // 0x32A1A0

    // Starts the animation at the path: an inlined resource of the entity,
    // or the loaded one.
    AnimInstID StartAnim(const ResourcePath& path, const AnimOptions& options);  // 0x32A240
    // Starts the resource with its kind's driver. A named start replaces
    // the playback under its name; otherwise mReplaceID is replaced. The
    // project's timeline callback may choose the timeline. Returns the null
    // id for a missing or failed resource or one no driver plays.
    AnimInstID StartAnim(Resource* resource, const AnimOptions& options);  // 0x32A370
    // The resource at the path, inlined into the entity or loaded. The
    // map's _FindResource(ResourcePath const&).
    ResourcePtr<Resource> _FindResource(const ResourcePath& path);  // 0x32A2C0
    // The playback started under the name, or the null id.
    AnimInstID FindInstId(Symbol name) const;  // 0x32A670
    bool IsAnimating(const AnimInstID& id, bool includePaused) const;  // 0x32A710
    // Stops the playback; false for the null id. The playback is nulled.
    bool StopAnim(AnimInstID& id);  // 0x32A730
    // Whether a playback of the resource at the path runs. Name not in the
    // reference map.
    bool IsAnimating(const ResourcePath& path, bool includePaused);  // 0x32A740
    // The driver of the resource class; CharClipResource shares the
    // character driver. Null for other classes. The map's
    // _FindDriver(Symbol const&); a second entry point at 0x32A660 jumps
    // here.
    AnimDriver* _FindDriver(const Symbol& resourceId);  // 0x32A7D0
    // The playback calls; nothing for the null id.
    void PauseAnim(const AnimInstID& id, bool pause);      // 0x32A970
    void SetAnimTime(const AnimInstID& id, float time);    // 0x32A990
    float GetAnimTime(const AnimInstID& id);               // 0x32A9A0
    void SetAnimFrame(const AnimInstID& id, float frame);  // 0x32A9C0
    float GetAnimFrame(const AnimInstID& id);              // 0x32A9D0
    bool IsFinishedPlaying(const AnimInstID& id) const;    // 0x32A9F0
    // The playback, or null.
    PlaybackInfo* GetPlaybackInfo(const AnimInstID& id);  // 0x32AA00
    // The number of playbacks in both drivers.
    unsigned long GetActiveAnimsCount() const;  // 0x32AA10
    // Stops every playback.
    void Clear();  // 0x32AA20
    // The timelines a component in the entity can pick, as TimeUnits
    // allowed values: a "Default (<units>)" entry (kNumTimeUnits) for the
    // default timeline, then the four timelines. Name not in the reference
    // map.
    eastl::vector<AllowedValue<int>> GetTimeUnitsAllowedValues() const;  // 0x32AA50

    // The message handlers. "start_anim" (path, then (key value) arrays
    // naming the AnimOptions and, after the start, "auto_destruct",
    // "weight" and "loop") returns 0; "stop_anim" (name) and
    // "apply_propanim_frame" (path frame) whether they did anything.
    DataNode _OnStartAnim(const DataArray* msg);           // 0x32ABE0
    DataNode _OnStopAnim(const DataArray* msg);            // 0x32B460
    DataNode _OnApplyPropanimFrame(const DataArray* msg);  // 0x32B570

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in anim/AnimInit.o at 0x2A76E0.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B1D20.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x329900

    // "AnimDriver", constructed by the static initializer at 0x32C5E0.
    static Symbol sId;  // 0x19FB690
    // Also "AnimDriver"; the class symbol GameObject::CreateComponent takes.
    // Name not in the reference map.
    static Symbol sClassName;  // 0x19FB698
    static PropRegistry sPropRegistry;  // 0x19FB6A0
    static ComMetaData sMetaData;       // 0x19FB740
    // Chooses the timeline of an animation the entity starts; kNumTimeUnits
    // keeps the options'. Set by the game (0xBB5590).
    static std::function<TimeUnits(Entity*, const Resource*)> sProjectTimelineCallback;  // 0x19FB8F0

    // "default_timeline": the timeline the entity's time-based components
    // run on unless they pick another (StateGraphInstance::ResolveTimeUnits
    // reads it). Name not in the reference map.
    TimeUnits mDefaultTimeline;
    RuntimeData mRuntime;
};

static_assert(offsetof(AnimDriverCom, mDefaultTimeline) == 24);
static_assert(offsetof(AnimDriverCom, mRuntime) == 32);
static_assert(offsetof(AnimDriverCom::RuntimeData, mPropAnimDriver) == 8);
static_assert(offsetof(AnimDriverCom::RuntimeData, mCharAnimDriver) == 104);
static_assert(offsetof(AnimDriverCom::RuntimeData, mNamedAnims) == 504);
static_assert(sizeof(AnimDriverCom) == 592);
