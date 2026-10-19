#pragma once

#include <cstddef>

#include "anim/core/AnimInstID.h"
#include "anim/core/AnimResource.h"
#include "entity/core/GameObject.h"
#include "entity/resources/Resource.h"
#include "utl/containers/List.h"
#include "utl/messages/Message.h"
#include "utl/text/Symbol.h"
#include "utl/time/TimeMgr.h"

class AnimOptions;
class Entity;

// One animation a driver plays (anim/AnimDriver.o): the animation, its
// timeline and range, the weight and rate, the current time and the blend
// in. The times count on the playback's timeline; the frames are the
// animation's. AnimDriver::GetPlaybackInfo finds it by its AnimInstID.
// Field names are not in the reference map. The object is 112 bytes.
class PlaybackInfo {
public:
    // Starts the playback of `anim` on the clock with the options. The
    // map's signature is PlaybackInfo(AnimResource*, unsigned long,
    // AnimOptions const&); this build adds the clock.
    PlaybackInfo(
        AnimResource* anim,
        unsigned long id,
        const AnimOptions& options,
        TimeMgr::Clock* clock);  // 0x3276F0

    // Takes the range, the speed and the start from the options and
    // aligns the start to the clock.
    void SetOptions(const AnimOptions& options);  // 0x3277F0
    // The frame at the current time.
    float GetCurrentFrame() const;  // 0x3279A0
    TimeUnits GetUnits() const;     // 0x3279C0
    // Refreshes the alignment period from the animation and, when there is
    // one, moves the current time into phase with the clock, keeping it in
    // the range.
    void _DoAlignment();  // 0x3279D0
    // Moves the current time into phase with the clock by the period
    // _DoAlignment found. No caller was found. Name not in the reference
    // map.
    void _AlignToClock();  // 0x327AE0
    // Advances the current time by the clock's delta (at most 1/15 on
    // the second-based timelines), keeps it in phase, wraps or clamps it to
    // the range, measures the frame delta and advances the blend in.
    void AdvanceCurrentTime();  // 0x327BC0
    // The frames at the ends of the range. Names not in the reference map.
    float GetEndFrame() const;    // 0x327EA0
    float GetStartFrame() const;  // 0x327EC0
    // Exports "anim_event" to the entity for each event between the last
    // frame and the current one, through the wraps of a looping
    // animation, with the weight. Name not in the reference map.
    void SendEvents(Entity* entity, float weight);  // 0x327EE0
    // Whether a playback that does not loop or hold reached the end of the
    // range in its direction.
    bool IsFinishedPlaying() const;  // 0x328560
    // Sets the current time to the frame's.
    void SetCurrentFrame(float frame);  // 0x3285B0
    // Blends the playback in over `duration` on the timeline from
    // `startTime`, or from now when it is AnimOptions::kInvalidTime. The
    // weight starts at 0.
    void SetBlendIn(TimeUnits units, float duration, float startTime);  // 0x3285E0
    // Sets the current time so the playback started at `time` on the
    // timeline.
    void SetStartTime(TimeUnits units, float time);  // 0x328660
    // The number of completed passes plus the fraction of the range the
    // current time has reached (1 more for an empty range). Name not in
    // the reference map; the evidence is weak.
    float GetTimingPosition() const;  // 0x3286C0

    // The resource, held while the playback runs.
    ResourcePtr<Resource> mResource;
    AnimResource* mAnim;
    TimeMgr::Clock* mClock;
    // The timeline the times count in.
    TimeUnits mUnits;
    // The range played, in times on the timeline; the beginning is never
    // after the end.
    float mBeginTime;
    float mEndTime;
    // The blend weight; the blend in raises it from 0 to 1.
    float mWeight;
    AnimResource::BlendMode mBlendMode;
    // The speed from the options, and the rate the period gives (1 without
    // one).
    float mScale;
    float mRate;
    bool mLooping;
    // Set when the last advance wrapped around the range.
    bool mWrapped;
    unsigned int mLoopCount;
    float mCurrentTime;
    // The animation's alignment period on the timeline, or 0.
    float mAlignPeriod;
    // The frame at the last poll; events between it and the current frame
    // are sent.
    float mLastFrame;
    // The playback does not advance. "start_paused" and PauseAnim set it,
    // and the poll sets it when a playback ends that does not destroy
    // itself.
    bool mPaused;
    // See AnimOptions::mManualTime.
    bool mManualTime;
    // "auto_destruct": the poll removes the playback when it ends. 2 and 3
    // keep it from ever ending; no writer of those values was found.
    int mAutoDestruct;
    // The blend-tree slot; the driver keeps its playbacks sorted by it.
    unsigned long mSubTreeIndex;
    // The blend in's timeline (kNumTimeUnits when there is none), the time
    // left until it starts, negated once it runs, and its duration.
    TimeUnits mBlendUnits;
    float mBlendTime;
    float mBlendDuration;
    // The frames the last advance moved, signed.
    float mFrameDelta;
    unsigned long mId;
};

static_assert(offsetof(PlaybackInfo, mAnim) == 8);
static_assert(offsetof(PlaybackInfo, mClock) == 16);
static_assert(offsetof(PlaybackInfo, mUnits) == 24);
static_assert(offsetof(PlaybackInfo, mBeginTime) == 28);
static_assert(offsetof(PlaybackInfo, mEndTime) == 32);
static_assert(offsetof(PlaybackInfo, mWeight) == 36);
static_assert(offsetof(PlaybackInfo, mBlendMode) == 40);
static_assert(offsetof(PlaybackInfo, mScale) == 44);
static_assert(offsetof(PlaybackInfo, mRate) == 48);
static_assert(offsetof(PlaybackInfo, mLooping) == 52);
static_assert(offsetof(PlaybackInfo, mWrapped) == 53);
static_assert(offsetof(PlaybackInfo, mLoopCount) == 56);
static_assert(offsetof(PlaybackInfo, mCurrentTime) == 60);
static_assert(offsetof(PlaybackInfo, mAlignPeriod) == 64);
static_assert(offsetof(PlaybackInfo, mLastFrame) == 68);
static_assert(offsetof(PlaybackInfo, mPaused) == 72);
static_assert(offsetof(PlaybackInfo, mManualTime) == 73);
static_assert(offsetof(PlaybackInfo, mAutoDestruct) == 76);
static_assert(offsetof(PlaybackInfo, mSubTreeIndex) == 80);
static_assert(offsetof(PlaybackInfo, mBlendUnits) == 88);
static_assert(offsetof(PlaybackInfo, mBlendTime) == 92);
static_assert(offsetof(PlaybackInfo, mFrameDelta) == 100);
static_assert(offsetof(PlaybackInfo, mId) == 104);
static_assert(sizeof(PlaybackInfo) == 112);

// The message a playback exports when it passes an event: (target
// "anim_event" event frame weight anim object instance). AnimDriverCom
// registers the parameters. The vtable is at 0x18FA1A8, after
// Message<6>'s at 0x18FA1C8. Name not in the reference map.
class AnimEventMsg : public Message<6> {
public:
    // The event of the playback's animation at the frame. `object` is the
    // clip's object for a CharClipResource and the null id otherwise.
    AnimEventMsg(
        Symbol event,
        ResourcePath anim,
        GameObjectId object,
        const PlaybackInfo& info,
        float frame,
        float weight);  // 0x328F30
    // The complete destructor at 0x328550 jumps to Message<6>'s (0x329700);
    // the deleting destructor is at 0x3296B0.
    ~AnimEventMsg() override;

    // The message type, built on first use. Inlined into the constructor;
    // the local static is at 0x19FB678.
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("anim_event");
        }
        return t;
    }
};

static_assert(sizeof(AnimEventMsg) == 168);

// The playbacks of one kind of animation resource (anim/AnimDriver.o): an
// AnimDriverCom holds a PropAnimDriver and a CharAnimDriver. The base keeps
// the playbacks, sorted by their blend-tree slot, and advances them; the
// subclass applies them. The vtable at 0x18F51D8 has five slots; the map's
// TAnimDriver<T> template is folded into this class in this build. Field
// names are not in the reference map.
class AnimDriver {
public:
    AnimDriver();  // 0x328720
    // Slots 0-1, emitted in anim/CharAnimDriver.o at 0x2DF140 and 0x2DF1B0.
    virtual ~AnimDriver() {}
    // Slot 2: starts the resource, which must be the driver's kind, and
    // returns the playback.
    virtual AnimInstID StartAnim(Resource* resource, const AnimOptions& options) = 0;
    // Slot 3: called before a playback is removed. Empty here (0x2DF0D0);
    // PropAnimDriver drops the property drivers it fed. Name not in the
    // reference map.
    virtual void _OnStopAnim(PlaybackInfo& info) {
        static_cast<void>(info);
    }
    // Slot 4: applies the playbacks to the entity of the root object.
    virtual void _Poll(GameObject* root) = 0;

    // The frames per unit of the timeline: 30 per second, 16 per beat, 1
    // for the frame-based values from 4 on. The out-of-line copy has no
    // caller; PropAnimResource's frame conversions inline it. The binary
    // emits it in anim/PropAnimDriver.o.
    static float GetDefaultFramesPerUnit(TimeUnits units);  // 0x2FD1F0
    // The timeline of the units, taking 4 off the frame-based values from
    // 4 on. The out-of-line copy has no caller; PropAnimResource inlines
    // it. Emitted in anim/PropAnimDriver.o. Name not in the reference map;
    // the evidence is weak.
    static int GetBaseUnits(int units);  // 0x2FD200

    // Adds a playback of the animation behind the others of its blend-tree
    // slot. The map's TAnimDriver<T>::_StartAnim(ResourcePtr<T> const&,
    // AnimOptions const&).
    PlaybackInfo* _StartAnim(AnimResource* anim, const AnimOptions& options);  // 0x328780
    // Sets the entity and the clock the playbacks run on. The map's
    // Enter(ObjPtr const&).
    void Enter(Entity* entity, TimeMgr::Clock* clock);  // 0x3288B0
    // Whether the playback runs; `includePaused` also counts a paused one.
    bool IsAnimating(const AnimInstID& id, bool includePaused) const;  // 0x3288C0
    // Whether a playback of the resource at the path runs. Name not in the
    // reference map.
    bool IsAnimating(const ResourcePath& path, bool includePaused) const;  // 0x328910
    // Removes the playback and nulls the id; false when it is not found.
    bool StopAnim(AnimInstID& id);  // 0x3289A0
    void PauseAnim(const AnimInstID& id, bool pause);  // 0x328A80
    // Removes every playback.
    void Clear();  // 0x328AC0
    // Advances the playbacks, applies them (slot 4), then removes or pauses
    // the ones that ended and records the frames they reached. The map's
    // TAnimDriver<T>::Poll(ObjPtr const&); AnimDriverCom passes its object,
    // the entity's root.
    void Poll(GameObject* root);  // 0x328B50
    void SetAnimTime(const AnimInstID& id, float time);    // 0x328CA0
    float GetAnimFrame(const AnimInstID& id);              // 0x328CE0
    void SetAnimFrame(const AnimInstID& id, float frame);  // 0x328D30
    float GetAnimTime(const AnimInstID& id);               // 0x328D80
    bool IsFinishedPlaying(const AnimInstID& id) const;    // 0x328DC0
    // The playback, or null.
    PlaybackInfo* GetPlaybackInfo(const AnimInstID& id);  // 0x328E40
    // The id of one of the driver's playbacks. Name not in the reference
    // map.
    AnimInstID GetInstID(const PlaybackInfo& info);  // 0x328E80
    // The id of the playback that sent an "anim_event". Name not in the
    // reference map.
    AnimInstID GetInstID(const AnimEventMsg& msg);  // 0x328ED0

    eastl::list<PlaybackInfo> mPlaybacks;
    // The id the next playback gets; -1 is skipped.
    unsigned long mNextId;
    Entity* mEntity;
    TimeMgr::Clock* mClock;

private:
    // The playback with the id. Inlined into the lookups. Name not in the
    // reference map.
    PlaybackInfo* _FindPlayback(const AnimInstID& id) const {
        for (auto it = const_cast<AnimDriver*>(this)->mPlaybacks.begin();
             it != const_cast<AnimDriver*>(this)->mPlaybacks.end();
             ++it) {
            if (it->mId == id.mId) {
                return &*it;
            }
        }
        return nullptr;
    }
};

static_assert(offsetof(AnimDriver, mPlaybacks) == 8);
static_assert(offsetof(AnimDriver, mNextId) == 40);
static_assert(offsetof(AnimDriver, mEntity) == 48);
static_assert(offsetof(AnimDriver, mClock) == 56);
static_assert(sizeof(AnimDriver) == 64);
