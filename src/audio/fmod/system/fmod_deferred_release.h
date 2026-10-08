#pragma once

#include <array>
#include <cstdint>
#include <mutex>
#include <vector>

#include "audio/fmod/api/fmod_api.h"

namespace rb4 {

struct FmodAudioState;

struct FmodDeferredRelease {
    FMOD::ChannelControl* channel_control = nullptr;
    FMOD::DSP* dsp = nullptr;
};

struct FmodDeferredReleaseQueue {
    FmodDeferredReleaseQueue();

    std::recursive_mutex mutex;
    std::int32_t dispatch_depth = 0;
    std::array<std::vector<FmodDeferredRelease>, 2> buffers;
    std::int32_t active_buffer = 0;
};

void fmod_defer_channel_dsp_release(
    FmodAudioState& state,
    FMOD::ChannelControl* channel_control,
    FMOD::DSP* dsp);

void fmod_process_deferred_releases(FmodAudioState& state);
void fmod_clear_deferred_releases(FmodAudioState& state);

}  // namespace rb4
