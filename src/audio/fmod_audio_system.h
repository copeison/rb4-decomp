#pragma once

#include <cstdint>

#include "fmod_api.h"

namespace rb4 {

constexpr std::uint32_t kFmodHeaderVersion = 0x00011004;

// Semantic state used by the cleaned reconstruction. The original audio object
// contains other timing, synchronization, stream, and callback state around
// these fields.
struct FmodAudioState {
    std::int32_t sample_rate = 0;
    FMOD::Studio::System* studio_system = nullptr;
    FMOD::System* core_system = nullptr;
    std::uint32_t dsp_buffer_length = 0;
    std::int32_t max_channels = 0;
    void* audio_clock = nullptr;
};

FMOD_RESULT fmod_audio_initialize(
    FmodAudioState& state,
    FMOD_OUTPUTTYPE requested_output,
    std::int32_t software_channels);

void fmod_register_custom_dsp_plugins(FmodAudioState& state);

}  // namespace rb4

