#pragma once

#include <cstddef>

#include "anim/core/AnimInstID.h"
#include "anim/core/AnimOptions.h"
#include "anim/char/CharAnimResource.h"
#include "entity/resources/Resource.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/nodes/StateNode.h"
#include "stategraph/props/DrivenProp.h"
#include "utl/text/Symbol.h"

class AnimResource;
class Entity;
class StateGraphInstance;

// The node that plays an animation on the graph's context entity
// (stategraph/StateNodeAnimate.o, 0x70AD80-0x7178F7; class id
// "StateNodeAnimate"). The animation is a PropAnimResource or
// CharAnimResource path ("anim") or the CharClipCom of an object ("clip"),
// as "anim_type" chooses; the node starts it on the graph's AnimDriverCom
// with its AnimOptions, the frame range, start, period, weight and scale
// it may drive from other properties, or applies a forced "current_time"
// each frame. The node is also the StateNodeTiming transitions synchronize
// to. The vtable at 0x193AAD0 has 57 slots; the StateNodeTiming one at
// 0x193ACA8 has six. The object is 2344 bytes.
class StateNodeAnimate : public StateNode, public StateNodeTiming {
public:
    // Whether the animation loops ("loop_mode"). The labels are the allowed
    // values'; the enumerator names are not in the reference map.
    enum LoopMode : int {
        kLoopModeDefault = 0,
        kLoopModeLoop = 1,
        kLoopModeDoNotLoop = 2,
    };

    // A node in a running graph: the animation instance it started on the
    // graph's AnimDriverCom (the map's StateNodeAnimate::Instance). The
    // vtable at 0x193ACE8 has 15 slots. The object is 64 bytes, from the
    // small-block pool.
    class Instance : public StateNodeInstance {
    public:
        POOL_OVERLOAD(Instance)

        Instance(const StateNodeAnimate& node, StateGraphInstance& graph);  // 0x7103D0
        // Slots 0-1 at 0x710C60 and 0x710C70.
        ~Instance() override {}

        // Slot 2: starts the animation. The map has the start in
        // _StartAnim(), which this build inlines.
        void _Enter() override;  // 0x710410
        // Slot 3: the share of the node's weight.
        void _SetWeight(float weight) override;  // 0x710CA0
        // Slot 5: updates the weight and the forced time or the scale;
        // false once the animation has finished. The map's _Poll(float).
        bool _Poll() override;  // 0x710690
        // Slot 6: stops the animation when "stop_on_exit" asks.
        void _Exit() override;  // 0x7107F0
        // Slots 12-13: the position of the playback on its range, and the
        // position _Enter starts the next animation at.
        float _GetTimingPosition() override;              // 0x710850
        void _SetTimingPosition(float position) override;  // 0x710890

        // Field names are not in the reference map.
        const StateNodeAnimate* mAnimateNode;
        AnimInstID mAnimId;
        // The node's share of the graph's blend (_SetWeight).
        float mWeight;
        // Set by _SetTimingPosition; -1 when unset.
        float mTimingPosition;
    };

    StateNodeAnimate();  // 0x70AD80
    // Copies the node for an imprint; the loaded animation is not copied.
    // Inlined into _Imprint (0x7109E0).
    StateNodeAnimate(const StateNodeAnimate& other)
        : StateNode(other),
          StateNodeTiming(other),
          mLoopMode(other.mLoopMode),
          mAnimType(other.mAnimType),
          mClip(other.mClip),
          mAnim(other.mAnim),
          mOptions(other.mOptions),
          mCurrentTimeEnabled(other.mCurrentTimeEnabled),
          mUseNormalizedTime(other.mUseNormalizedTime),
          mTimeRangeBeginEnabled(other.mTimeRangeBeginEnabled),
          mTimeRangeEndEnabled(other.mTimeRangeEndEnabled),
          mPeriodEnabled(other.mPeriodEnabled),
          mStartTimeEnabled(other.mStartTimeEnabled),
          mStopOnExit(other.mStopOnExit),
          mCurrentTime(other.mCurrentTime),
          mTimeRangeBegin(other.mTimeRangeBegin),
          mTimeRangeEnd(other.mTimeRangeEnd),
          mPeriod(other.mPeriod),
          mStartTime(other.mStartTime),
          mWeight(other.mWeight),
          mScale(other.mScale),
          mCharAnim() {}
    // Slots 0-1: 0x70E9F0, 0x7108A0.
    ~StateNodeAnimate() override {}

    Symbol GetId() const override;          // slot 4: 0x710960
    Symbol GetClassName() const override;   // slot 5: 0x710970
    int CurrentRev() const override;        // slot 7: 0x710980
    bool IsA(Symbol type) const override;   // slot 8: 0x7109A0
    Component* AsComponent() override;      // slot 9: 0x7109D0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x7109E0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x710BB0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x710BC0
    // Slot 29: loads the CharAnimResource "anim" names, or drops it.
    bool _OnResourcesLoaded() override;  // 0x70EAB0
    // Slot 42: the animation's name, with "(looping)" when it loops.
    const char* GetEditorText() const override;  // 0x70F010
    // Slot 44: the node's own timing.
    const StateNodeTiming* GetTiming() const override;  // 0x710BD0
    // Slot 45.
    StateNodeInstance* _CreateInstance(StateGraphInstance& graph) const override;  // 0x710BE0
    // Slot 46: the animation blends.
    bool CanBlend() const override;  // 0x710C50

    // Slots 50-55 and their StateNodeTiming thunks (0x70F4A0, 0x70F980,
    // 0x70FE50, 0x70FFC0, 0x710010, 0x710190).
    // Valid unless a forced current time replaces the playback.
    bool IsValid() const override;  // 0x70F490
    // The clip object's name ("<None>" without one) or the animation
    // file's base name.
    const char* GetName() const override;  // 0x70F4B0
    // The time one pass over the range takes at the scale, in `units`
    // (4 for the node's own) at `rate` beats per minute: the period, or the
    // range's length, converted between seconds and beats.
    float GetScale(int units, float rate) const override;  // 0x70F990
    // The range's length in frames.
    bool GetLength(float& length) const override;  // 0x70FE60
    // The timeline, or the animation's own for "Defer to anim".
    int GetUnits() const override;  // 0x70FFE0
    // Refreshes the clip object's CharClipCom.
    void Refresh() const override;  // 0x710040
    // Slot 56: the loop mode; a synchronized transition out of the node
    // whose blend ends after one pass forces kLoopModeLoop, which `forced`
    // reports. Name not in the reference map.
    virtual int GetLoopMode(bool* forced) const;  // 0x7101A0

    // Copies the options and enables the range, start and period they set.
    // No caller in this build.
    void SetOptions(const AnimOptions& options);  // 0x70EFA0
    // Plays the animation at the path, undriven, of the type its path
    // names. No caller in this build. Name not in the reference map.
    void SetAnim(ResourcePath path);  // 0x70EEB0
    // The animation resource the node plays, from the context entity's
    // inlined resources, a loaded clip or the loaded CharAnimResource; null
    // when there is none or it failed. Name not in the reference map.
    AnimResource* GetAnimResource() const;  // 0x70F0F0
    // The path of the animation: "anim", or the clip resource of "clip"'s
    // CharClipCom. Name not in the reference map.
    ResourcePath GetAnimPath(Entity* entity) const;  // 0x710240

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in stategraph/StateGraph.o at 0x6F73C0.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x6FEF30.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x70AF90

    static Symbol sId;  // 0x1AB2F30, "StateNodeAnimate"
    // Also "StateNodeAnimate". Name not in the reference map.
    static Symbol sClassName;  // 0x1AB2F38
    static PropRegistry sPropRegistry;  // 0x1AB2F40
    static ComMetaData sMetaData;       // 0x1AB2FE0

    // Field names are not in the reference map; they follow the
    // properties.
    int mLoopMode;  // "loop_mode" (LoopMode)
    // "anim_type": the resource class of the animation.
    Symbol mAnimType;
    // "clip": the object whose CharClipCom defines the clip.
    DrivenProp<GameObjectId> mClip;
    // "anim": the PropAnimResource or CharAnimResource.
    DrivenProp<ResourcePath> mAnim;
    // "timeline", "use_frame_time", "start_from_current_frame" and
    // "anim_instance_name"; the times are the properties below.
    AnimOptions mOptions;
    // "current_time"'s, "time_range_begin"'s, "time_range_end"'s,
    // "period"'s and "start_time"'s "enabled", and "use_normalized_time"
    // and "stop_on_exit".
    bool mCurrentTimeEnabled;
    bool mUseNormalizedTime;
    bool mTimeRangeBeginEnabled;
    bool mTimeRangeEndEnabled;
    bool mPeriodEnabled;
    bool mStartTimeEnabled;
    bool mStopOnExit;
    // The structs' "time" (and "period"), "weight" and "scale".
    DrivenProp<float> mCurrentTime;
    DrivenProp<float> mTimeRangeBegin;
    DrivenProp<float> mTimeRangeEnd;
    DrivenProp<float> mPeriod;
    DrivenProp<float> mStartTime;
    DrivenProp<float> mWeight;
    DrivenProp<float> mScale;
    // The CharAnimResource loaded for "anim" (_OnResourcesLoaded).
    ResourcePtr<CharAnimResource> mCharAnim;
};

static_assert(offsetof(StateNodeAnimate, mLoopMode) == 160);
static_assert(offsetof(StateNodeAnimate, mAnimType) == 168);
static_assert(offsetof(StateNodeAnimate, mClip) == 176);
static_assert(offsetof(StateNodeAnimate, mAnim) == 408);
static_assert(offsetof(StateNodeAnimate, mOptions) == 640);
static_assert(offsetof(StateNodeAnimate, mCurrentTimeEnabled) == 704);
static_assert(offsetof(StateNodeAnimate, mStopOnExit) == 710);
static_assert(offsetof(StateNodeAnimate, mCurrentTime) == 712);
static_assert(offsetof(StateNodeAnimate, mTimeRangeBegin) == 944);
static_assert(offsetof(StateNodeAnimate, mTimeRangeEnd) == 1176);
static_assert(offsetof(StateNodeAnimate, mPeriod) == 1408);
static_assert(offsetof(StateNodeAnimate, mStartTime) == 1640);
static_assert(offsetof(StateNodeAnimate, mWeight) == 1872);
static_assert(offsetof(StateNodeAnimate, mScale) == 2104);
static_assert(offsetof(StateNodeAnimate, mCharAnim) == 2336);
static_assert(sizeof(StateNodeAnimate) == 2344);
static_assert(offsetof(StateNodeAnimate::Instance, mAnimateNode) == 32);
static_assert(offsetof(StateNodeAnimate::Instance, mAnimId) == 40);
static_assert(offsetof(StateNodeAnimate::Instance, mWeight) == 56);
static_assert(offsetof(StateNodeAnimate::Instance, mTimingPosition) == 60);
static_assert(sizeof(StateNodeAnimate::Instance) == 64);
