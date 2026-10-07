#include "fmod_audio_system.h"

#include <array>

namespace rb4 {
FMOD_RESULT fmod_file_open(
    const char* name,
    std::uint32_t* file_size,
    void** handle,
    void* user_data);
FMOD_RESULT fmod_file_close(void* handle, void* user_data);
FMOD_RESULT fmod_file_read(
    void* handle,
    void* buffer,
    std::uint32_t size,
    std::uint32_t* bytes_read,
    void* user_data);
FMOD_RESULT fmod_file_seek(
    void* handle,
    std::uint32_t position,
    void* user_data);
FMOD_RESULT fmod_file_async_read(FMOD_ASYNCREADINFO* info, void* user_data);
FMOD_RESULT fmod_file_async_cancel(FMOD_ASYNCREADINFO* info, void* user_data);
FMOD_RESULT fmod_system_callback(
    FMOD_SYSTEM* system,
    FMOD_SYSTEM_CALLBACK_TYPE type,
    void* command_data1,
    void* command_data2,
    void* user_data);

void audio_set_sample_rate(double sample_rate);
void audio_clock_initialize(void* clock, std::uint32_t sample_rate);

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

        audio_set_sample_rate(static_cast<double>(state.sample_rate));
        audio_clock_initialize(state.audio_clock, state.sample_rate);
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

}  // namespace rb4
