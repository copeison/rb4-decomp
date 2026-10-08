#include "audio/fmod/system/fmod_timing_report.h"

#include "audio/core/audio_mix_format.h"
#include "audio/fmod/system/fmod_audio_system.h"

namespace rb4 {

namespace {

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

AudioTimingPercentages consume_timing(
    AudioTimingAccumulator& timing,
    double buffer_count = 1.0) {
    lock_statistics(timing);

    const double measured_buffers =
        g_audio_mix_format.milliseconds_per_buffer * buffer_count;
    AudioTimingPercentages result;
    if (timing.sample_count != 0 && measured_buffers > 0.0) {
        const double average_milliseconds =
            timing.total_milliseconds / timing.sample_count;
        result.average = average_milliseconds / measured_buffers * 100.0;
    }
    if (measured_buffers > 0.0) {
        result.maximum =
            timing.maximum_milliseconds / measured_buffers * 100.0;
    }

    timing.total_milliseconds = 0.0;
    timing.sample_count = 0;
    timing.maximum_milliseconds = 0.0;
    unlock_statistics(timing);
    return result;
}

}  // namespace

// Reconstructed from eboot.elf at 0x277BA0.
void fmod_audio_consume_timing_report(
    FmodAudioState& state,
    FmodAudioTimingReport& report,
    bool append_source_keys) {
    report.engine_mix = consume_timing(state.engine_mix_timing);
    report.fmod_mix = consume_timing(state.fmod_mix_timing);
    report.buffer_set = consume_timing(
        state.buffer_set_timing.timing,
        static_cast<double>(state.buffer_set_timing.window_size));

    for (auto* entry = state.source_timing_entries;
         entry != nullptr;
         entry = entry->next) {
        const auto source = consume_timing(entry->timing);
        report.source_average[entry->timing.key] += source.average;
        report.source_maximum[entry->timing.key] += source.maximum;
        if (append_source_keys) {
            report.source_keys.push_back(entry->timing.key);
        }
    }
}

}  // namespace rb4
