#pragma once

#include <cstdint>

#include "utl/time/Timer.h"

namespace rb4 {

struct FmodAudioState;

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

void audio_register_mix_consumer(void* state, void* consumer);
void audio_unregister_mix_consumer(void* state, void* consumer);

}  // namespace rb4
