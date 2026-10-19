// anim/AnimDriver.o (0x3276F0 to 0x3297EF): PlaybackInfo, AnimDriver and
// the "anim_event" message, whose Message<6> base (0x329460, 0x3296D0,
// 0x3296E0, 0x329700) the binary emits here.
#include "anim/drivers/AnimDriver.h"

#include <math.h>

#include "anim/core/AnimOptions.h"
#include "entity/core/Entity.h"

namespace {

// The largest step a second-based timeline advances a playback by, in
// seconds. Name not in the reference map.
constexpr float kMaxDeltaSeconds = 1.0f / 15.0f;

// The offset that brings `time` into the phase `target` has modulo
// `period`, between half a period back and half a period on; 0 without a
// period. Inlined into _DoAlignment, _AlignToClock and AdvanceCurrentTime.
// Name not in the reference map.
float PhaseOffset(float target, float time, float period) {
    const float half = period * 0.5f;
    float targetPhase = 0.0f;
    float timePhase = 0.0f;
    if (period != 0.0f) {
        targetPhase = fmodf(target, period);
        if (targetPhase < 0.0f) {
            targetPhase += period;
        }
        timePhase = fmodf(time, period);
        if (timePhase < 0.0f) {
            timePhase += period;
        }
    }
    float offset = targetPhase - timePhase;
    if (offset > half) {
        offset -= period;
    } else if (offset < -half) {
        offset += period;
    }
    return offset;
}

}  // namespace

// Reconstructed from eboot.elf at 0x3276F0.
PlaybackInfo::PlaybackInfo(
    AnimResource* anim,
    unsigned long id,
    const AnimOptions& options,
    TimeMgr::Clock* clock)
    : mResource(anim->AsResource()),
      mAnim(anim),
      mClock(clock),
      mUnits(options.mTimeline),
      mBeginTime(0.0f),
      mEndTime(0.0f),
      mWeight(1.0f),
      mScale(options.mScale),
      mRate(1.0f),
      mLooping(false),
      mWrapped(false),
      mLoopCount(0),
      mAlignPeriod(0.0f),
      mPaused(options.mStartPaused),
      mManualTime(options.mManualTime),
      mAutoDestruct(0),
      mBlendUnits(kNumTimeUnits),
      mBlendTime(0.0f),
      mBlendDuration(0.0f),
      mFrameDelta(0.0f),
      mId(id) {
    if (mUnits == kNumTimeUnits) {
        mUnits = mAnim->GetUnits();
    }
    mBlendMode = mAnim->GetBlendMode();
    SetOptions(options);
    mLastFrame = mAnim->GetFrameFromTime(mCurrentTime, mUnits);
}

// Reconstructed from eboot.elf at 0x3277F0. An unset range takes the
// animation's frames; a playback that runs backwards starts at the end.
void PlaybackInfo::SetOptions(const AnimOptions& options) {
    mSubTreeIndex = options.mSubTreeIndex == static_cast<unsigned long>(-1) ? mAnim->GetSubTreeIndex()
                                                                            : options.mSubTreeIndex;
    if (options.mTimeRangeBegin == AnimOptions::kInvalidTime) {
        mBeginTime = mAnim->GetTimeFromFrame(mAnim->GetStartFrame(), mUnits);
    } else if (options.mUseFrameTime) {
        mBeginTime = mAnim->GetTimeFromFrame(options.mTimeRangeBegin, mUnits);
    } else {
        mBeginTime = options.mTimeRangeBegin;
    }
    if (options.mTimeRangeEnd == AnimOptions::kInvalidTime) {
        mEndTime = mAnim->GetTimeFromFrame(mAnim->GetEndFrame(), mUnits);
    } else if (options.mUseFrameTime) {
        mEndTime = mAnim->GetTimeFromFrame(options.mTimeRangeEnd, mUnits);
    } else {
        mEndTime = options.mTimeRangeEnd;
    }
    mScale = options.mScale;
    mLooping = mAnim->IsLooping();
    mRate = options.mPeriod == AnimOptions::kInvalidPeriod ? 1.0f : (mEndTime - mBeginTime) / options.mPeriod;
    if (!(mEndTime >= mBeginTime)) {
        const float begin = mBeginTime;
        mBeginTime = mEndTime;
        mEndTime = begin;
    }
    if (options.mStartTime != AnimOptions::kInvalidTime) {
        mCurrentTime = options.mUseFrameTime ? mAnim->GetTimeFromFrame(options.mStartTime, mUnits)
                                             : options.mStartTime;
    } else {
        mCurrentTime = mRate * mScale >= 0.0f ? mBeginTime : mEndTime;
    }
    _DoAlignment();
}

// Reconstructed from eboot.elf at 0x3279A0.
float PlaybackInfo::GetCurrentFrame() const {
    return mAnim->GetFrameFromTime(mCurrentTime, mUnits);
}

// Reconstructed from eboot.elf at 0x3279C0.
TimeUnits PlaybackInfo::GetUnits() const {
    return mUnits;
}

// Reconstructed from eboot.elf at 0x3279D0.
void PlaybackInfo::_DoAlignment() {
    mAlignPeriod = mAnim->GetTimeAlignment(mUnits);
    if (mAlignPeriod == 0.0f) {
        return;
    }
    const float now = mClock->Time(mUnits);
    mCurrentTime += PhaseOffset(now * mScale * mRate, mCurrentTime, mAlignPeriod);
    if (mCurrentTime < mBeginTime) {
        mCurrentTime += mAlignPeriod;
    } else if (mCurrentTime > mEndTime) {
        mCurrentTime -= mAlignPeriod;
    }
}

// Reconstructed from eboot.elf at 0x327AE0.
void PlaybackInfo::_AlignToClock() {
    const float now = mClock->Time(mUnits);
    mCurrentTime += PhaseOffset(now * mScale * mRate, mCurrentTime, mAlignPeriod);
}

// Reconstructed from eboot.elf at 0x327BC0. A paused or manually timed
// playback does not move. A manually timed loop does not wrap; when its
// owner wrapped it, the frame delta is corrected by the range.
void PlaybackInfo::AdvanceCurrentTime() {
    float delta = 0.0f;
    bool advanced = false;
    if (!mPaused && !mManualTime) {
        delta = mClock->DeltaTime(mUnits);
        advanced = true;
    }
    if (static_cast<unsigned int>(mUnits) <= kTimeUnitsTutorialSeconds && mUnits != kTimeUnitsBeats) {
        if (delta > kMaxDeltaSeconds) {
            delta = kMaxDeltaSeconds;
        } else if (delta < -kMaxDeltaSeconds) {
            delta = -kMaxDeltaSeconds;
        }
    }
    mCurrentTime += delta * mScale * mRate;
    if (advanced && mAlignPeriod != 0.0f) {
        const float now = mClock->Time(mUnits);
        mCurrentTime += PhaseOffset(now * mScale * mRate, mCurrentTime, mAlignPeriod);
    }

    if (mLooping) {
        mFrameDelta = mBeginTime != mEndTime ? mAnim->GetFrameFromTime(mCurrentTime, mUnits) - mLastFrame : 0.0f;
        if (mManualTime) {
            if (mWrapped) {
                const float endFrame = mAnim->GetFrameFromTime(mEndTime, mUnits);
                const float beginFrame = mAnim->GetFrameFromTime(mBeginTime, mUnits);
                mFrameDelta += (beginFrame - endFrame) * (mFrameDelta > 0.0f ? 1.0f : -1.0f);
            }
        } else {
            int loops = 0;
            float time = mBeginTime;
            if (mBeginTime != mEndTime) {
                const float span = mEndTime - mBeginTime;
                loops = static_cast<int>(floorf((mCurrentTime - mBeginTime) / span));
                time = mCurrentTime - static_cast<float>(loops) * span;
            }
            mCurrentTime = time;
            mWrapped = loops != 0;
            mLoopCount += loops;
        }
    } else {
        if (mCurrentTime > mEndTime) {
            mCurrentTime = mEndTime;
        } else if (mCurrentTime < mBeginTime) {
            mCurrentTime = mBeginTime;
        }
        mWrapped = false;
        mFrameDelta = mAnim->GetFrameFromTime(mCurrentTime, mUnits) - mLastFrame;
    }

    if (mBlendUnits == kNumTimeUnits) {
        return;
    }
    // The blend in eases from 0 to 1 over its duration (smoothstep).
    mBlendTime -= delta;
    float t = mBlendDuration + mBlendTime == mBlendTime ? 1.0f : -mBlendTime / mBlendDuration;
    if (t > 1.0f) {
        t = 1.0f;
    } else if (t < 0.0f) {
        t = 0.0f;
    }
    mWeight = t * t * (t * -2.0f + 3.0f);
}

// Reconstructed from eboot.elf at 0x327EA0.
float PlaybackInfo::GetEndFrame() const {
    return mAnim->GetFrameFromTime(mEndTime, mUnits);
}

// Reconstructed from eboot.elf at 0x327EC0.
float PlaybackInfo::GetStartFrame() const {
    return mAnim->GetFrameFromTime(mBeginTime, mUnits);
}

// Reconstructed from eboot.elf at 0x327EE0. The last advance moved the
// frame by mFrameDelta from mLastFrame; a looping playback may have crossed
// the range several times. Each pass searches the sorted events for the
// first one at or past its start in the direction of play and sends those
// up to its end; an event at the end of the range is sent once.
void PlaybackInfo::SendEvents(Entity* entity, float weight) {
    if (mFrameDelta == 0.0f) {
        return;
    }
    const PropArray<AnimResource::Event>* events = mAnim->GetEvents(entity);
    if (events == nullptr || events->mSize == 0) {
        return;
    }
    const float beginFrame = mAnim->GetFrameFromTime(mBeginTime, mUnits);
    const float endFrame = mAnim->GetFrameFromTime(mEndTime, mUnits);
    const float lastFrame = mLastFrame;
    const float delta = mFrameDelta;
    const bool forward = delta >= 0.0f;
    float remaining = fabsf(delta);
    const float toEdge = forward ? endFrame - lastFrame : lastFrame - beginFrame;
    const unsigned long passes =
        static_cast<unsigned long>((remaining - toEdge) / (endFrame - beginFrame) + 2.0f);
    if (passes == 0) {
        return;
    }
    const float wrapFrame = forward ? beginFrame : endFrame;
    const float limitFrame = forward ? endFrame : beginFrame;
    const unsigned int endIndex = forward ? events->mSize : 0xFFFFFFFFU;

    for (unsigned long pass = 0; pass < passes; ++pass) {
        const float from = pass == 0 ? lastFrame : wrapFrame;
        const AnimResource::Event* data = static_cast<const AnimResource::Event*>(events->mData);
        // The first event past the start: at or after it going forwards,
        // after it going backwards.
        long position = 0;
        long count = events->mSize;
        while (count > 0) {
            const long half = static_cast<unsigned long>(count) >> 1;
            const float frame = mAnim->EventFrameToFrame(data[position + half].mFrame);
            if (forward ? frame < from : !(frame > from)) {
                position += half + 1;
                count -= half + 1;
            } else {
                count = half;
            }
        }
        float to;
        if (forward) {
            const float target = remaining + from;
            to = target < endFrame ? target : endFrame;
        } else {
            // The last event at or before the start; -1 when there is none.
            --position;
            const float target = from - remaining;
            to = target > beginFrame ? target : beginFrame;
        }

        unsigned int index = static_cast<unsigned int>(position);
        if (index != endIndex) {
            float frame = mAnim->EventFrameToFrame(data[index].mFrame);
            bool send = (frame == to && frame == limitFrame) || (forward ? frame < to : frame > to);
            while (send) {
                GameObject* const object = mAnim->GetObject(entity);
                GameObjectId objectId = {0xFFFFFFFFU};
                if (object != nullptr && object->mEntity == entity) {
                    objectId = object->mId;
                }
                data = static_cast<const AnimResource::Event*>(events->mData);
                AnimEventMsg msg(
                    data[index].mName, mAnim->AsResource()->mPath, objectId, *this, data[index].mFrame, weight);
                entity->Export(msg.mData);
                index = forward ? index + 1 : index - 1;
                if (index == endIndex) {
                    break;
                }
                data = static_cast<const AnimResource::Event*>(events->mData);
                frame = mAnim->EventFrameToFrame(data[index].mFrame);
                send = (frame == to && frame == limitFrame) || (forward ? frame < to : frame > to);
            }
        }
        remaining -= fabsf(to - from);
    }
}

// Reconstructed from eboot.elf at 0x328560.
bool PlaybackInfo::IsFinishedPlaying() const {
    if (mLooping || (mAutoDestruct & ~1) == 2) {
        return false;
    }
    if (mRate * mScale >= 0.0f) {
        return mCurrentTime >= mEndTime;
    }
    return mCurrentTime <= mBeginTime;
}

// Reconstructed from eboot.elf at 0x3285B0.
void PlaybackInfo::SetCurrentFrame(float frame) {
    mCurrentTime = mAnim->GetTimeFromFrame(frame, mUnits);
}

// Reconstructed from eboot.elf at 0x3285E0.
void PlaybackInfo::SetBlendIn(TimeUnits units, float duration, float startTime) {
    mBlendUnits = units;
    mBlendTime = mClock->DeltaTime(units);
    mBlendDuration = duration;
    if (startTime != AnimOptions::kInvalidTime) {
        mBlendTime += startTime - mClock->Time(units);
    }
    mWeight = 0.0f;
}

// Reconstructed from eboot.elf at 0x328660.
void PlaybackInfo::SetStartTime(TimeUnits units, float time) {
    mCurrentTime -= mClock->DeltaTime(units);
    mCurrentTime += mClock->Time(units) - time;
}

// Reconstructed from eboot.elf at 0x3286C0.
float PlaybackInfo::GetTimingPosition() const {
    const float loops = static_cast<float>(mLoopCount);
    if (mEndTime == mBeginTime) {
        return 1.0f + loops;
    }
    const float low = mBeginTime < mEndTime ? mBeginTime : mEndTime;
    const float high = mEndTime > mBeginTime ? mEndTime : mBeginTime;
    float time = mCurrentTime > low ? mCurrentTime : low;
    if (mCurrentTime > high) {
        time = high;
    }
    return (time - mBeginTime) / (mEndTime - mBeginTime) + loops;
}

// Reconstructed from eboot.elf at 0x328720. The entity is set by Enter.
AnimDriver::AnimDriver() : mPlaybacks(), mNextId(0), mClock(nullptr) {}

// Reconstructed from eboot.elf at 0x328780.
PlaybackInfo* AnimDriver::_StartAnim(AnimResource* anim, const AnimOptions& options) {
    const PlaybackInfo info(anim, mNextId++, options, mClock);
    if (mNextId == static_cast<unsigned long>(-1)) {
        mNextId = 0;
    }
    auto position = mPlaybacks.begin();
    while (position != mPlaybacks.end() && position->mSubTreeIndex <= info.mSubTreeIndex) {
        ++position;
    }
    return &*mPlaybacks.insert(position, info);
}

// Reconstructed from eboot.elf at 0x3288B0.
void AnimDriver::Enter(Entity* entity, TimeMgr::Clock* clock) {
    mEntity = entity;
    mClock = clock;
}

// Reconstructed from eboot.elf at 0x3288C0.
bool AnimDriver::IsAnimating(const AnimInstID& id, bool includePaused) const {
    const PlaybackInfo* const info = _FindPlayback(id);
    return info != nullptr && (includePaused || !info->mPaused);
}

// Reconstructed from eboot.elf at 0x328910.
bool AnimDriver::IsAnimating(const ResourcePath& path, bool includePaused) const {
    auto& playbacks = const_cast<AnimDriver*>(this)->mPlaybacks;
    for (auto it = playbacks.begin(); it != playbacks.end(); ++it) {
        if (it->mAnim->AsResource()->mPath == path && (includePaused || !it->mPaused)) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x3289A0.
bool AnimDriver::StopAnim(AnimInstID& id) {
    for (auto it = mPlaybacks.begin(); it != mPlaybacks.end(); ++it) {
        if (it->mId == id.mId) {
            _OnStopAnim(*it);
            mPlaybacks.erase(it);
            id = AnimInstID();
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x328A80.
void AnimDriver::PauseAnim(const AnimInstID& id, bool pause) {
    PlaybackInfo* const info = _FindPlayback(id);
    if (info != nullptr) {
        info->mPaused = pause;
    }
}

// Reconstructed from eboot.elf at 0x328AC0.
void AnimDriver::Clear() {
    for (auto it = mPlaybacks.begin(); it != mPlaybacks.end();) {
        _OnStopAnim(*it);
        it = mPlaybacks.erase(it);
    }
}

// Reconstructed from eboot.elf at 0x328B50.
void AnimDriver::Poll(GameObject* root) {
    for (auto it = mPlaybacks.begin(); it != mPlaybacks.end(); ++it) {
        it->AdvanceCurrentTime();
    }
    _Poll(root);
    for (auto it = mPlaybacks.begin(); it != mPlaybacks.end();) {
        if (it->IsFinishedPlaying()) {
            if (it->mAutoDestruct != 0) {
                _OnStopAnim(*it);
                it = mPlaybacks.erase(it);
                continue;
            }
            it->mPaused = true;
        }
        it->mLastFrame = it->GetCurrentFrame();
        ++it;
    }
}

// Reconstructed from eboot.elf at 0x328CA0.
void AnimDriver::SetAnimTime(const AnimInstID& id, float time) {
    PlaybackInfo* const info = _FindPlayback(id);
    if (info != nullptr) {
        info->mCurrentTime = time;
    }
}

// Reconstructed from eboot.elf at 0x328CE0.
float AnimDriver::GetAnimFrame(const AnimInstID& id) {
    const PlaybackInfo* const info = _FindPlayback(id);
    return info != nullptr ? info->GetCurrentFrame() : 0.0f;
}

// Reconstructed from eboot.elf at 0x328D30.
void AnimDriver::SetAnimFrame(const AnimInstID& id, float frame) {
    PlaybackInfo* const info = _FindPlayback(id);
    if (info != nullptr) {
        info->SetCurrentFrame(frame);
    }
}

// Reconstructed from eboot.elf at 0x328D80.
float AnimDriver::GetAnimTime(const AnimInstID& id) {
    const PlaybackInfo* const info = _FindPlayback(id);
    return info != nullptr ? info->mCurrentTime : 0.0f;
}

// Reconstructed from eboot.elf at 0x328DC0.
bool AnimDriver::IsFinishedPlaying(const AnimInstID& id) const {
    const PlaybackInfo* const info = _FindPlayback(id);
    return info != nullptr && info->IsFinishedPlaying();
}

// Reconstructed from eboot.elf at 0x328E40.
PlaybackInfo* AnimDriver::GetPlaybackInfo(const AnimInstID& id) {
    return _FindPlayback(id);
}

// Reconstructed from eboot.elf at 0x328E80.
AnimInstID AnimDriver::GetInstID(const PlaybackInfo& info) {
    return AnimInstID(this, info.mId);
}

// Reconstructed from eboot.elf at 0x328ED0.
AnimInstID AnimDriver::GetInstID(const AnimEventMsg& msg) {
    return AnimInstID(this, msg.mData->Int(7));
}

// Reconstructed from eboot.elf at 0x328F30.
AnimEventMsg::AnimEventMsg(
    Symbol event,
    ResourcePath anim,
    GameObjectId object,
    const PlaybackInfo& info,
    float frame,
    float weight)
    : Message<6>(Event()) {
    mData->Node(2) = DataNode(event);
    mData->Node(3) = DataNode(frame);
    mData->Node(4) = DataNode(weight);
    DataNode path;
    path.mType = kDataResourcePath;
    path.mValue.symbol = anim.Str();
    mData->Node(5) = path;
    DataNode id;
    id.mType = kDataGameObjectId;
    id.mValue.var = object.mId;
    mData->Node(6) = id;
    mData->Node(7) = DataNode(static_cast<int>(info.mId));
}

// Reconstructed from eboot.elf at 0x328550.
AnimEventMsg::~AnimEventMsg() {}
