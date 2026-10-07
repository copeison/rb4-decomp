#pragma once

#include <cstdint>

#include "audio_output_dispatcher.h"
#include "fmod_api.h"
#include "fmod_mix_callback.h"

namespace rb4 {

constexpr std::uint32_t kFmodHeaderVersion = 0x00011004;

// Semantic state used by the cleaned reconstruction. The original audio object
// contains other timing, synchronization, stream, and callback state around
// these fields.
struct FmodAudioState {
    std::int32_t sample_rate = 0;
    std::uint32_t requested_dsp_buffer_length = 0;
    std::int32_t requested_dsp_buffer_count = 0;
    std::int32_t raw_speaker_count = 0;
    FMOD::Studio::System* studio_system = nullptr;
    FMOD::System* core_system = nullptr;
    std::uint32_t dsp_buffer_length = 0;
    FMOD_SPEAKERMODE speaker_mode = FMOD_SPEAKERMODE_DEFAULT;
    std::int32_t max_channels = 0;
    void* buffered_output_update_callback = nullptr;
    bool shutting_down = false;
    bool mix_in_progress = false;
    std::uint64_t mix_sequence = 0;
    AudioTimingAccumulator engine_mix_timing;
    AudioTimingAccumulator fmod_mix_timing;
    AudioRollingTimingAccumulator buffer_set_timing;
    void* mix_semaphore = nullptr;
    void* mix_consumer_mutex = nullptr;
    void* mix_consumers = nullptr;
    std::uint32_t mix_consumer_dispatch_depth = 0;
    AudioOutputDispatcher output_block_dispatcher;
    void* source_timing_entries = nullptr;
    void* post_mix_observers = nullptr;
};

FMOD_RESULT fmod_audio_initialize(
    FmodAudioState& state,
    FMOD_OUTPUTTYPE requested_output,
    std::int32_t software_channels);

void fmod_register_custom_dsp_plugins(FmodAudioState& state);

std::uint32_t fmod_audio_initialize_custom_output(FmodAudioState& state);

void fmod_audio_attach_studio_system(
    FmodAudioState& state,
    FMOD::Studio::System* studio_system);

void fmod_audio_detach_studio_system(FmodAudioState& state);

}  // namespace rb4
