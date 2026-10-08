#include "audio/fmod/system/fmod_audio_system.h"
#include "audio/core/format/audio_mix_format.h"
#include "audio/core/output/audio_output_dispatcher.h"
#include "audio/core/runtime/audio_runtime_adapters.h"
#include "audio/fmod/io/fmod_buffered_output.h"
#include "audio/fmod/system/fmod_deferred_release.h"
#include "audio/fmod/io/fmod_file_io.h"
#include "audio/fmod/mixing/fmod_mix_callback.h"

#include <array>

namespace rb4 {
const FMOD_DSP_DESCRIPTION* fmod_get_analysis_dsp_description();
const FMOD_DSP_DESCRIPTION* fmod_get_bitcrusher_dsp_description();
const FMOD_DSP_DESCRIPTION* fmod_get_delay_dsp_description();
const FMOD_DSP_DESCRIPTION* fmod_get_filter_dsp_description();
const FMOD_DSP_DESCRIPTION* fmod_get_gain_dsp_description();
const FMOD_DSP_DESCRIPTION* fmod_get_signal_tap_dsp_description();
const FMOD_DSP_DESCRIPTION* fmod_get_pitch_shift_dsp_description();
const FMOD_DSP_DESCRIPTION* fmod_get_stutter_dsp_description();
const FMOD_DSP_DESCRIPTION* fmod_get_sound_clash_slot_dsp_description();
const FMOD_DSP_DESCRIPTION* fmod_get_tremolo_dsp_description();
const FMOD_DSP_DESCRIPTION* fmod_get_vibe_dsp_description();
const FMOD_DSP_DESCRIPTION* fmod_get_wah_dsp_description();

namespace {

using DspDescriptionGetter = const FMOD_DSP_DESCRIPTION* (*)();

constexpr std::array<DspDescriptionGetter, 12> kCustomDspDescriptions{
    fmod_get_analysis_dsp_description,
    fmod_get_bitcrusher_dsp_description,
    fmod_get_delay_dsp_description,
    fmod_get_filter_dsp_description,
    fmod_get_gain_dsp_description,
    fmod_get_signal_tap_dsp_description,
    fmod_get_pitch_shift_dsp_description,
    fmod_get_stutter_dsp_description,
    fmod_get_sound_clash_slot_dsp_description,
    fmod_get_tremolo_dsp_description,
    fmod_get_vibe_dsp_description,
    fmod_get_wah_dsp_description,
};

}  // namespace

// Reconstructed from eboot.elf at 0x2773C0.
FMOD_RESULT fmod_audio_initialize(
    FmodAudioState& state,
    FMOD_OUTPUTTYPE requested_output,
    std::int32_t software_channels) {
    [[maybe_unused]] const auto creation_result = FMOD::Studio::System::create(
        &state.studio_system, kFmodHeaderVersion);

    // On FMOD_ERR_HEADER_MISMATCH, the original formats the temporary message
    // "header version is 1.10.4" and then continues through common startup.
    // It has no observable consumer, so the reconstruction omits that object.

    state.studio_system->setUserData(&state);
    state.studio_system->getLowLevelSystem(&state.core_system);
    state.core_system->setUserData(&state);

    if (state.core_system->setOutput(requested_output) != FMOD_OK) {
        state.core_system->setOutput(FMOD_OUTPUTTYPE_AUTODETECT);
    }

    FMOD_ADVANCEDSETTINGS core_settings{};
    core_settings.cbSize = sizeof(core_settings);
    state.core_system->getAdvancedSettings(&core_settings);
    core_settings.stackSizeMixer += 16 * 1024;
    core_settings.commandQueueSize *= 4;
    state.core_system->setAdvancedSettings(&core_settings);

    FMOD_STUDIO_ADVANCEDSETTINGS studio_settings{};
    studio_settings.cbsize = sizeof(studio_settings);
    state.studio_system->getAdvancedSettings(&studio_settings);
    studio_settings.commandqueuesize *= 4;
    state.studio_system->setAdvancedSettings(&studio_settings);

    state.core_system->setSoftwareChannels(software_channels);
    const auto result = state.studio_system->initialize(
        state.max_channels,
        FMOD_STUDIO_INIT_NORMAL,
        FMOD_INIT_NORMAL,
        nullptr);

    if (result == FMOD_OK) {
        state.core_system->setFileSystem(
            fmod_file_open,
            fmod_file_close,
            fmod_file_read,
            fmod_file_seek,
            fmod_file_async_read,
            fmod_file_async_cancel,
            -1);

        std::int32_t driver = 0;
        std::array<char, 256> driver_name{};
        state.core_system->getDriver(&driver);
        state.core_system->getDriverInfo(
            driver,
            driver_name.data(),
            static_cast<std::int32_t>(driver_name.size()),
            nullptr,
            &state.sample_rate,
            nullptr,
            nullptr);
        state.core_system->getSoftwareFormat(
            &state.sample_rate, nullptr, nullptr);

        std::int32_t dsp_buffer_count = 0;
        state.core_system->getDSPBufferSize(
            &state.dsp_buffer_length, &dsp_buffer_count);

        audio_set_mix_format(
            static_cast<double>(state.sample_rate),
            static_cast<std::int32_t>(state.dsp_buffer_length));
        audio_output_dispatcher_set_sample_rate(
            state.output_block_dispatcher, state.sample_rate);
        fmod_register_custom_dsp_plugins(state);

        constexpr auto callback_mask =
            FMOD_SYSTEM_CALLBACK_PREMIX | FMOD_SYSTEM_CALLBACK_POSTMIX;
        state.core_system->setCallback(fmod_system_callback, callback_mask);
    }

    (void)state.studio_system->isValid();
    state.studio_system->update();
    return result;
}

// Reconstructed from eboot.elf at 0x278270.
void fmod_register_custom_dsp_plugins(FmodAudioState& state) {
    for (const auto get_description : kCustomDspDescriptions) {
        state.studio_system->registerPlugin(get_description());
    }
}

// Reconstructed from eboot.elf at 0x2783A0.
void fmod_audio_configure_speakers(
    FmodAudioState& state,
    AudioSpeakerConfiguration configuration) {
    switch (configuration) {
        case AudioSpeakerConfiguration::mono:
            state.speaker_mode = FMOD_SPEAKERMODE_MONO;
            state.raw_speaker_count = 1;
            break;
        case AudioSpeakerConfiguration::stereo:
            state.speaker_mode = FMOD_SPEAKERMODE_STEREO;
            state.raw_speaker_count = 2;
            break;
        case AudioSpeakerConfiguration::surround_5_1:
            state.speaker_mode = FMOD_SPEAKERMODE_5POINT1;
            state.raw_speaker_count = 6;
            break;
        case AudioSpeakerConfiguration::surround_7_1:
            state.speaker_mode = FMOD_SPEAKERMODE_7POINT1;
            state.raw_speaker_count = 8;
            break;
    }
}

// Reconstructed from eboot.elf at 0x2786D0.
std::uint32_t fmod_audio_initialize_custom_output(FmodAudioState& state) {
    FMOD::Studio::System::create(&state.studio_system, kFmodHeaderVersion);
    state.studio_system->getLowLevelSystem(&state.core_system);
    state.studio_system->setUserData(&state);
    state.core_system->setUserData(&state);

    std::uint32_t output_handle = 0;
    state.core_system->registerOutput(
        &kFmodBufferedOutputDescription, &output_handle);
    state.core_system->setOutputByPlugin(output_handle);
    state.core_system->setDSPBufferSize(
        state.requested_dsp_buffer_length,
        state.requested_dsp_buffer_count);
    state.core_system->setSoftwareChannels(state.max_channels);
    state.core_system->setSoftwareFormat(
        state.sample_rate, state.speaker_mode, state.raw_speaker_count);

    constexpr auto studio_flags = FMOD_STUDIO_INIT_SYNCHRONOUS_UPDATE;
    constexpr auto core_flags =
        FMOD_INIT_STREAM_FROM_UPDATE |
        FMOD_INIT_MIX_FROM_UPDATE |
        FMOD_INIT_3D_RIGHTHANDED;
    const auto result = state.studio_system->initialize(
        state.max_channels, studio_flags, core_flags, &state);

    std::int32_t actual_sample_rate = 0;
    FMOD_SPEAKERMODE actual_speaker_mode = FMOD_SPEAKERMODE_DEFAULT;
    std::int32_t actual_raw_speakers = 0;
    state.core_system->getSoftwareFormat(
        &actual_sample_rate, &actual_speaker_mode, &actual_raw_speakers);

    std::int32_t dsp_buffer_count = 0;
    state.core_system->getDSPBufferSize(
        &state.dsp_buffer_length, &dsp_buffer_count);
    fmod_register_custom_dsp_plugins(state);

    if (result == FMOD_OK) {
        state.core_system->setFileSystem(
            fmod_file_open,
            fmod_file_close,
            fmod_file_read,
            fmod_file_seek,
            fmod_file_async_read,
            fmod_file_async_cancel,
            -1);
        constexpr auto callback_mask =
            FMOD_SYSTEM_CALLBACK_PREMIX | FMOD_SYSTEM_CALLBACK_POSTMIX;
        state.core_system->setCallback(fmod_system_callback, callback_mask);
        return 0;
    }

    return 1;
}

// Reconstructed from eboot.elf at 0x277840.
void fmod_audio_attach_studio_system(
    FmodAudioState& state,
    FMOD::Studio::System* studio_system) {
    if (state.studio_system == studio_system) {
        return;
    }
    if (studio_system == nullptr) {
        fmod_audio_detach_studio_system(state);
        return;
    }

    state.studio_system = studio_system;
    state.studio_system->getLowLevelSystem(&state.core_system);

    std::uint32_t version = 0;
    void* previous_user_data = nullptr;
    state.core_system->getVersion(&version);
    state.studio_system->getUserData(&previous_user_data);
    state.studio_system->setUserData(&state);
    state.core_system->getUserData(&previous_user_data);
    state.core_system->setUserData(&state);

    std::int32_t driver = 0;
    std::array<char, 256> driver_name{};
    state.core_system->getDriver(&driver);
    state.core_system->getDriverInfo(
        driver,
        driver_name.data(),
        static_cast<std::int32_t>(driver_name.size()),
        nullptr,
        &state.sample_rate,
        nullptr,
        nullptr);
    state.core_system->getSoftwareFormat(&state.sample_rate, nullptr, nullptr);

    std::int32_t dsp_buffer_count = 0;
    state.core_system->getDSPBufferSize(
        &state.dsp_buffer_length, &dsp_buffer_count);
    audio_output_dispatcher_set_sample_rate(
        state.output_block_dispatcher, state.sample_rate);
    fmod_register_custom_dsp_plugins(state);

    state.shutting_down = false;
    audio_mix_semaphore_post(state.mix_semaphore);
    constexpr auto callback_mask =
        FMOD_SYSTEM_CALLBACK_PREMIX | FMOD_SYSTEM_CALLBACK_POSTMIX;
    state.core_system->setCallback(fmod_system_callback, callback_mask);
    (void)state.studio_system->isValid();
    state.studio_system->update();
}

// Reconstructed from eboot.elf at 0x277A80 and the null branch of 0x277840.
void fmod_audio_detach_studio_system(FmodAudioState& state) {
    while (audio_mix_semaphore_wait(state.mix_semaphore) != 0) {
    }
    state.shutting_down = true;
    fmod_clear_deferred_releases(state);
    state.studio_system = nullptr;
    state.core_system = nullptr;
    audio_mix_semaphore_post(state.mix_semaphore);
}

}  // namespace rb4
