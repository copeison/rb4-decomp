#pragma once

#include <cstddef>

#include "anim/core/AnimInstID.h"
#include "utl/text/Symbol.h"
#include "utl/time/TimeMgr.h"

// How an animation is started (anim/AnimOptions.o, 0x32C6F0-0x32C757): the
// timeline, the part of the animation played and where it starts, its
// speed, the playback it replaces and the name it is found by. The
// "start_anim" handler (AnimDriverCom::_OnStartAnim) names most fields
// through its keys; the field names are not in the reference map.
class AnimOptions {
public:
    // Everything unset: the resource's timeline and range, scale 1.
    AnimOptions();  // 0x32C6F0

    // The value of an unset time, frame or start ("time_range_begin",
    // "time_range_end", "start_time"): 1e30.
    static const float kInvalidTime;  // 0x1275090
    // The value of an unset "period": 1e30.
    static const float kInvalidPeriod;  // 0x1275094

    // "timeline": the clock the playback runs on; kNumTimeUnits takes the
    // resource's units (AnimResource::GetUnits).
    TimeUnits mTimeline;
    // "time_range_begin" and "time_range_end": the part played, in the
    // timeline's units or, with mUseFrameTime, in frames.
    float mTimeRangeBegin;
    float mTimeRangeEnd;
    // "start_time": where the playback starts; unset starts at the
    // beginning of the range in the direction of play.
    float mStartTime;
    // "use_frame_time": the range and the start are frames.
    bool mUseFrameTime;
    // "period": the time one pass over the range takes; unset plays at the
    // resource's rate.
    float mPeriod;
    // "scale": the playback speed.
    float mScale;
    // "start_from_current_frame": the replaced playback's time (or frame,
    // with mUseFrameTime) becomes the start.
    bool mStartFromCurrentFrame;
    // "start_paused".
    bool mStartPaused;
    // The playback does not advance with its clock or wrap when it loops;
    // its owner sets its time (PlaybackInfo::mManualTime). No "start_anim"
    // key sets it; the name is weakly supported.
    bool mManualTime;
    // The blend-tree slot the playback goes into; -1 takes the resource's
    // (AnimResource::GetSubTreeIndex).
    unsigned long mSubTreeIndex;
    // The playback this one replaces, used when mName is empty. It is
    // stopped once the new playback's start is known.
    AnimInstID mReplaceID;
    // "name": the playback is found by this name (AnimDriverCom::FindInstId)
    // and replaces the playback already under it.
    Symbol mName;
};

static_assert(offsetof(AnimOptions, mTimeRangeBegin) == 4);
static_assert(offsetof(AnimOptions, mStartTime) == 12);
static_assert(offsetof(AnimOptions, mUseFrameTime) == 16);
static_assert(offsetof(AnimOptions, mPeriod) == 20);
static_assert(offsetof(AnimOptions, mScale) == 24);
static_assert(offsetof(AnimOptions, mStartFromCurrentFrame) == 28);
static_assert(offsetof(AnimOptions, mManualTime) == 30);
static_assert(offsetof(AnimOptions, mSubTreeIndex) == 32);
static_assert(offsetof(AnimOptions, mReplaceID) == 40);
static_assert(offsetof(AnimOptions, mName) == 56);
static_assert(sizeof(AnimOptions) == 64);
