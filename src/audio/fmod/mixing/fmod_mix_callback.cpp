#include "audio/fmod/mixing/fmod_mix_callback.h"

#include <algorithm>

#include "audio/core/output/audio_output_dispatcher.h"
#include "audio/core/runtime/audio_runtime_adapters.h"
#include "audio/fmod/system/fmod_audio_system.h"

namespace rb4 {

namespace {

void begin_timing(AudioTimingAccumulator& timing) {
    timing.start_ticks = performance_counter_read();
    timing.elapsed_ticks = 0;
    timing.active_depth = 1;
}

double finish_timing(AudioTimingAccumulator& timing) {
    if (timing.active_depth > 0 && --timing.active_depth == 0) {
        timing.elapsed_ticks += performance_counter_read() - timing.start_ticks;
    }
    return performance_counter_ticks_to_milliseconds(timing.elapsed_ticks);
}

void lock_statistics(AudioTimingAccumulator& timing) {
    std::int32_t expected = 0;
    while (!timing.statistics_lock.compare_exchange_weak(
        expected, 1, std::memory_order_acquire)) {
        expected = 0;
    }
}

void unlock_statistics(AudioTimingAccumulator& timing) {
    timing.statistics_lock.store(0, std::memory_order_release);
}

void record_timing(AudioTimingAccumulator& timing) {
    const double milliseconds = finish_timing(timing);
    lock_statistics(timing);
    timing.total_milliseconds += milliseconds;
    ++timing.sample_count;
    timing.maximum_milliseconds = std::max(
        timing.maximum_milliseconds, milliseconds);
    unlock_statistics(timing);
}

void record_rolling_timing(AudioRollingTimingAccumulator& rolling) {
    const double milliseconds = finish_timing(rolling.timing);
    lock_statistics(rolling.timing);

    const std::uint32_t slot = rolling.window_index % rolling.window_size;
    rolling.rolling_milliseconds += milliseconds - rolling.window[slot];
    rolling.window[slot] = milliseconds;
    ++rolling.window_index;

    rolling.timing.total_milliseconds += rolling.rolling_milliseconds;
    ++rolling.timing.sample_count;
    rolling.timing.maximum_milliseconds = std::max(
        rolling.timing.maximum_milliseconds,
        rolling.rolling_milliseconds);
    unlock_statistics(rolling.timing);
}

void begin_mix(FmodAudioState& state) {
    while (audio_mix_semaphore_wait(state.mix_semaphore) != 0) {
    }

    if (state.studio_system == nullptr) {
        audio_mix_semaphore_post(state.mix_semaphore);
        return;
    }

    state.mix_in_progress = true;
    ++state.mix_sequence;
    begin_timing(state.engine_mix_timing);
    begin_timing(state.fmod_mix_timing);
    begin_timing(state.buffer_set_timing.timing);
    audio_reset_source_mix_timing(state.source_timing_entries);
    fmod_audio_dispatch_mix_buffers(state, state.mix_sequence);
    record_timing(state.engine_mix_timing);
}

void finish_mix(FmodAudioState& state) {
    if (!state.mix_in_progress) {
        return;
    }

    record_timing(state.fmod_mix_timing);
    record_rolling_timing(state.buffer_set_timing);
    audio_notify_post_mix(state.post_mix_observers);
    state.mix_in_progress = false;
    audio_mix_semaphore_post(state.mix_semaphore);
}

}  // namespace

// Reconstructed from eboot.elf at 0x2781C0.
void fmod_audio_dispatch_mix_buffers(
    FmodAudioState& state,
    std::uint64_t mix_sequence) {
    audio_mix_consumer_mutex_lock(state.mix_consumer_mutex);
    ++state.mix_consumer_dispatch_depth;
    audio_dispatch_mix_consumers(
        state.mix_consumers, state.dsp_buffer_length, mix_sequence);
    --state.mix_consumer_dispatch_depth;
    audio_mix_consumer_mutex_unlock(state.mix_consumer_mutex);

    audio_dispatch_output_blocks(
        state.output_block_dispatcher,
        state.dsp_buffer_length,
        static_cast<std::uint32_t>(mix_sequence));
}

// Reconstructed from eboot.elf at 0x2783E0.
FMOD_RESULT fmod_system_callback(
    FMOD_SYSTEM* system,
    FMOD_SYSTEM_CALLBACK_TYPE type,
    void* command_data1,
    void* command_data2,
    void* user_data) {
    (void)system;
    (void)command_data1;
    (void)command_data2;

    auto* state = static_cast<FmodAudioState*>(user_data);
    if (state == nullptr || state->shutting_down ||
        state->studio_system == nullptr) {
        return FMOD_OK;
    }

    if (type == FMOD_SYSTEM_CALLBACK_PREMIX) {
        begin_mix(*state);
    } else if (type == FMOD_SYSTEM_CALLBACK_POSTMIX) {
        finish_mix(*state);
    }
    return FMOD_OK;
}

}  // namespace rb4
