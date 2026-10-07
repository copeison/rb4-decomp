#pragma once

#include <cstdint>

namespace rb4 {

struct FmodAudioState;

std::uint64_t performance_counter_read();
double performance_counter_ticks_to_seconds(std::uint64_t ticks);

int audio_mix_semaphore_wait(void* semaphore);
void audio_mix_semaphore_post(void* semaphore);

void audio_mix_consumer_mutex_lock(void* mutex);
void audio_mix_consumer_mutex_unlock(void* mutex);
void audio_dispatch_mix_consumers(
    void* consumers,
    std::uint32_t buffer_length,
    std::uint64_t mix_sequence);

void audio_reset_source_mix_timing(void* timing_entries);
void audio_notify_post_mix(void* observers);

}  // namespace rb4
