#pragma once

#include <atomic>
#include <cstdint>

#include "fmod_api.h"

namespace rb4 {

struct FmodAudioState;

struct AudioTimingAccumulator {
    double total_seconds = 0.0;
    std::uint32_t sample_count = 0;
    double maximum_seconds = 0.0;
    std::atomic_flag statistics_lock = ATOMIC_FLAG_INIT;
    std::uint64_t start_ticks = 0;
    std::uint64_t elapsed_ticks = 0;
    std::int32_t active_depth = 0;
};

struct AudioRollingTimingAccumulator {
    AudioTimingAccumulator timing;
    double rolling_seconds = 0.0;
    std::uint32_t window_index = 0;
    double* window = nullptr;
    std::uint32_t window_size = 0;
};

void fmod_audio_dispatch_mix_buffers(
    FmodAudioState& state,
    std::uint64_t mix_sequence);

FMOD_RESULT fmod_system_callback(
    FMOD_SYSTEM* system,
    FMOD_SYSTEM_CALLBACK_TYPE type,
    void* command_data1,
    void* command_data2,
    void* user_data);

}  // namespace rb4
