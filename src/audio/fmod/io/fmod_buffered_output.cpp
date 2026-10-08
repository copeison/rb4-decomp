#include "audio/fmod/io/fmod_buffered_output.h"

#include <cstddef>
#include <cstdio>

#include "audio/core/audio_mix_format.h"
#include "audio/fmod/system/fmod_audio_system.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kFmodOutputPluginVersion = 3;
constexpr std::uint32_t kBufferedOutputVersion = 1;
constexpr std::int32_t kBufferedOutputDriverCount = 128;

// Reconstructed from eboot.elf at 0x2763F0.
FMOD_RESULT fmod_buffered_output_get_num_drivers(
    FMOD_OUTPUT_STATE* output_state,
    std::int32_t* driver_count) {
    (void)output_state;
    *driver_count = kBufferedOutputDriverCount;
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x276400.
FMOD_RESULT fmod_buffered_output_get_driver_info(
    FMOD_OUTPUT_STATE* output_state,
    std::int32_t id,
    char* name,
    std::int32_t name_length,
    void* guid,
    std::int32_t* system_rate,
    FMOD_SPEAKERMODE* speaker_mode,
    std::int32_t* speaker_mode_channels) {
    (void)output_state;
    (void)guid;

    if (name != nullptr && name_length > 0) {
        std::snprintf(
            name,
            static_cast<std::size_t>(name_length),
            "null_output_%d",
            id);
        name[name_length - 1] = '\0';
    }
    *system_rate = static_cast<std::int32_t>(audio_get_sample_rate());
    *speaker_mode = FMOD_SPEAKERMODE_STEREO;
    *speaker_mode_channels = 2;
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x276480.
FMOD_RESULT fmod_buffered_output_initialize(
    FMOD_OUTPUT_STATE* output_state,
    std::int32_t selected_driver,
    FMOD_INITFLAGS flags,
    std::int32_t* output_rate,
    FMOD_SPEAKERMODE* speaker_mode,
    std::int32_t* speaker_mode_channels,
    FMOD_SOUND_FORMAT* output_format,
    std::int32_t dsp_buffer_length,
    std::int32_t dsp_buffer_count,
    void* extra_driver_data) {
    (void)selected_driver;
    (void)flags;
    (void)output_rate;
    (void)dsp_buffer_length;
    (void)dsp_buffer_count;

    auto* state = static_cast<FmodAudioState*>(extra_driver_data);
    output_state->plugindata = state;
    *output_format = FMOD_SOUND_FORMAT_PCMFLOAT;
    *speaker_mode = state->speaker_mode;
    *speaker_mode_channels = state->raw_speaker_count;
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x2764B0.
FMOD_RESULT fmod_buffered_output_close(FMOD_OUTPUT_STATE* output_state) {
    (void)output_state;
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x2764C0.
FMOD_RESULT fmod_buffered_output_update(FMOD_OUTPUT_STATE* output_state) {
    auto* state = static_cast<FmodAudioState*>(output_state->plugindata);
    return state->buffered_output_update_callback(*output_state);
}

// Reconstructed from eboot.elf at 0x276520.
FMOD_RESULT fmod_buffered_output_get_handle(
    FMOD_OUTPUT_STATE* output_state,
    void** handle) {
    (void)output_state;
    (void)handle;
    return FMOD_OK;
}

}  // namespace

const FMOD_OUTPUT_DESCRIPTION kFmodBufferedOutputDescription{
    kFmodOutputPluginVersion,
    "HMX.BufferedOutput",
    kBufferedOutputVersion,
    FMOD_OUTPUT_METHOD_MIX_DIRECT,
    fmod_buffered_output_get_num_drivers,
    fmod_buffered_output_get_driver_info,
    fmod_buffered_output_initialize,
    nullptr,
    nullptr,
    fmod_buffered_output_close,
    fmod_buffered_output_update,
    fmod_buffered_output_get_handle,
    {},
};

}  // namespace rb4
