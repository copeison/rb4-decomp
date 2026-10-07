#pragma once

#include <atomic>
#include <cstdint>

#include "fmod_api.h"

namespace rb4 {

struct FmodAudioState;

using AudioTimingKey = std::uint64_t;

struct AudioTimingAccumulator {
    AudioTimingKey key = 0;
    double total_milliseconds = 0.0;
    std::uint32_t sample_count = 0;
    double maximum_milliseconds = 0.0;
    std::atomic<std::int32_t> statistics_lock{0};
    std::uint64_t start_ticks = 0;
    std::uint64_t elapsed_ticks = 0;
    std::int32_t active_depth = 0;
};

struct AudioRollingTimingAccumulator {
    AudioTimingAccumulator timing;
    double rolling_milliseconds = 0.0;
    std::uint32_t window_index = 0;
    double* window = nullptr;
    std::uint32_t window_size = 0;
};

// Semantic view of the original intrusive list entry. The executable stores
// the link immediately before the timing accumulator.
struct AudioSourceTimingEntry {
    AudioTimingAccumulator timing;
    AudioSourceTimingEntry* next = nullptr;
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
