// anim/AnimOptions.o (0x32C6F0 to 0x32C757). The two constants are in the
// read-only data.
#include "anim/core/AnimOptions.h"

const float AnimOptions::kInvalidTime = 1e30f;
const float AnimOptions::kInvalidPeriod = 1e30f;

// Reconstructed from eboot.elf at 0x32C6F0.
AnimOptions::AnimOptions()
    : mTimeline(kNumTimeUnits),
      mTimeRangeBegin(kInvalidTime),
      mTimeRangeEnd(kInvalidTime),
      mStartTime(kInvalidTime),
      mUseFrameTime(false),
      mPeriod(kInvalidPeriod),
      mScale(1.0f),
      mStartFromCurrentFrame(false),
      mStartPaused(false),
      mManualTime(false),
      mSubTreeIndex(static_cast<unsigned long>(-1)),
      mReplaceID(),
      mName() {}
