#include "fmod_studio_sound_generator.h"

#include <algorithm>
#include <cmath>

#include "fmod_audio_system.h"
#include "fmod_listener.h"

namespace rb4 {

namespace {

constexpr float kMinimumFadeMilliseconds = 25.0F;
constexpr float kMillisecondsPerSecond = 1000.0F;

float decibels_to_linear(float decibels) {
    return std::pow(10.0F, decibels * 0.05F);
}

}  // namespace

// Reconstructed from eboot.elf at 0x26FC30.
bool FmodStudioSoundGenerator::initialize_event(
    const char* event_path,
    const FmodStudioSoundOptions& options,
    FmodAudioState& audio_state,
    AudioClipFmodSpatialSource* spatial_source,
    FMOD::Studio::EventCallback callback,
    void* callback_user_data) {
    if (event_path == nullptr || audio_state.studio_system == nullptr) {
        return false;
    }

    FMOD::Studio::EventDescription* description = nullptr;
    if (audio_state.studio_system->getEvent(
            event_path, &description) != FMOD_OK ||
        description == nullptr) {
        return false;
    }

    std::int32_t event_length_ms = 0;
    description->getLength(&event_length_ms);
    length_ms_ = static_cast<float>(event_length_ms);
    if (description->createInstance(&event_instance_) != FMOD_OK ||
        event_instance_ == nullptr) {
        return false;
    }

    audio_state_ = &audio_state;
    spatial_source_ = spatial_source;
    event_callback_complete_.store(false, std::memory_order_relaxed);
    event_instance_->setUserData(
        callback_user_data != nullptr ? callback_user_data : this);

    fade_ = {};
    volume_transition_ = {};
    volume_transition_requested_ = options.start_silent;
    if (options.configure_initial_fade) {
        configure_fade(
            options.initial_fade_completion_mode,
            decibels_to_linear(options.initial_fade_db),
            options.initial_fade_seconds);
    }
    if (options.start_silent) {
        volume_transition_.start = 0.0F;
        volume_transition_.current = 0.0F;
        volume_transition_.target = 0.0F;
    }

    for (const auto& parameter : options.parameters) {
        set_event_parameter(parameter.name, parameter.value);
    }

    event_instance_->setCallback(
        callback != nullptr ? callback : event_callback,
        FMOD_STUDIO_EVENT_CALLBACK_ALL);
    if (options.start_paused) {
        event_instance_->setPaused(true);
        state_ = AudioClipFmodState::paused;
    } else {
        if (spatial_source_ != nullptr) {
            const auto attributes = audio_build_fmod_3d_attributes(
                spatial_source_->transform());
            event_instance_->set3DAttributes(&attributes);
        }
        state_ = AudioClipFmodState::playing;
    }
    event_instance_->start();
    return true;
}

// Reconstructed from eboot.elf at 0x2701D0.
void FmodStudioSoundGenerator::pause() {
    state_ = AudioClipFmodState::paused;
    if (event_instance_ != nullptr) {
        event_instance_->setPaused(true);
    }
}

// Reconstructed from eboot.elf at 0x2701F0.
void FmodStudioSoundGenerator::resume() {
    state_ = AudioClipFmodState::playing;
    if (event_instance_ != nullptr) {
        event_instance_->setPaused(false);
    }
}

// Reconstructed from eboot.elf at 0x270620.
void FmodStudioSoundGenerator::prepare_for_audio_reset() {
    if (event_instance_ == nullptr) {
        return;
    }
    const auto mode = state_ == AudioClipFmodState::paused
        ? FMOD_STUDIO_STOP_IMMEDIATE
        : FMOD_STUDIO_STOP_ALLOWFADEOUT;
    event_instance_->stop(mode);
    state_ = AudioClipFmodState::stopping;
}

// Reconstructed from eboot.elf at 0x270660.
void FmodStudioSoundGenerator::release_event() {
    if (event_instance_ == nullptr) {
        return;
    }
    event_instance_->stop(FMOD_STUDIO_STOP_IMMEDIATE);
    event_instance_->release();
    event_instance_ = nullptr;
}

// Reconstructed from eboot.elf at 0x2706A0.
void FmodStudioSoundGenerator::stop_and_wait() {
    if (event_instance_ == nullptr) {
        return;
    }

    state_ = AudioClipFmodState::stopping;
    event_instance_->setCallback(nullptr, FMOD_STUDIO_EVENT_CALLBACK_ALL);
    event_instance_->stop(FMOD_STUDIO_STOP_IMMEDIATE);
    event_callback_complete_.store(true, std::memory_order_relaxed);
    event_instance_->release();
    event_instance_ = nullptr;
    state_ = AudioClipFmodState::stopped;
}

AudioClipFmodState FmodStudioSoundGenerator::state() const {
    return state_;
}

// Reconstructed from eboot.elf at 0x270210 and 0x270260.
float FmodStudioSoundGenerator::position_ms() const {
    if (event_instance_ == nullptr) {
        return 0.0F;
    }
    std::int32_t position = 0;
    event_instance_->getTimelinePosition(&position);
    return static_cast<float>(position);
}

float FmodStudioSoundGenerator::channel_position_ms() const {
    return position_ms();
}

// Reconstructed from eboot.elf at 0x2714C0.
float FmodStudioSoundGenerator::length_ms() const {
    return length_ms_;
}

// Reconstructed from eboot.elf at 0x2702B0.
void FmodStudioSoundGenerator::set_position_ms(float position) {
    if (event_instance_ != nullptr) {
        event_instance_->setTimelinePosition(
            static_cast<std::int32_t>(position));
    }
}

// Reconstructed from eboot.elf at 0x2707D0.
bool FmodStudioSoundGenerator::set_event_parameter(
    const char* name,
    float value) {
    return event_instance_ != nullptr &&
        state_ != AudioClipFmodState::stopping &&
        event_instance_->setParameterValue(name, value) == FMOD_OK;
}

// Reconstructed from eboot.elf at 0x270800.
bool FmodStudioSoundGenerator::get_event_parameter(
    const char* name,
    float& value) const {
    if (event_instance_ == nullptr ||
        state_ == AudioClipFmodState::stopping) {
        return false;
    }
    FMOD::Studio::ParameterInstance* parameter = nullptr;
    return event_instance_->getParameter(name, &parameter) == FMOD_OK &&
        parameter != nullptr && parameter->getValue(&value) == FMOD_OK;
}

// Reconstructed from eboot.elf at 0x270880.
void FmodStudioSoundGenerator::configure_fade(
    std::int32_t completion_mode,
    float target,
    float duration_seconds) {
    fade_.start = fade_.current;
    fade_.target = target;
    fade_.duration_ms = std::max(
        duration_seconds * kMillisecondsPerSecond,
        kMinimumFadeMilliseconds);
    fade_.progress = 0.0F;
    fade_.elapsed_ms = 0.0F;
    fade_.stop_when_complete = completion_mode == 1;
    if (duration_seconds == 0.0F) {
        fade_.current = target;
        fade_.progress = 1.0F;
    }
}

// Reconstructed from eboot.elf at 0x2709A0.
float FmodStudioSoundGenerator::fade_value() const {
    return fade_.current;
}

// Reconstructed from eboot.elf at 0x2709B0.
void FmodStudioSoundGenerator::configure_volume_transition(
    bool fade_out,
    bool immediate) {
    volume_transition_requested_ = fade_out;
    volume_transition_.start = volume_transition_.current;
    volume_transition_.target = fade_out ? 0.0F : 1.0F;
    volume_transition_.duration_ms = kMinimumFadeMilliseconds;
    volume_transition_.progress = 0.0F;
    volume_transition_.elapsed_ms = 0.0F;
    if (immediate) {
        volume_transition_.current = volume_transition_.target;
        volume_transition_.progress = 1.0F;
    }

    if (state_ != AudioClipFmodState::playing &&
        event_instance_ != nullptr) {
        event_instance_->setVolume(
            fade_.current * volume_transition_.current);
    }
}

bool FmodStudioSoundGenerator::volume_transition_requested() const {
    return volume_transition_requested_;
}

// Reconstructed from eboot.elf at 0x2702C0.
bool FmodStudioSoundGenerator::update() {
    if (event_instance_ == nullptr) {
        state_ = AudioClipFmodState::stopped;
        return false;
    }

    FMOD_STUDIO_PLAYBACK_STATE playback_state = FMOD_STUDIO_PLAYBACK_STOPPED;
    if (event_instance_->getPlaybackState(&playback_state) ==
            FMOD_ERR_INVALID_HANDLE ||
        playback_state == FMOD_STUDIO_PLAYBACK_STOPPED) {
        event_instance_->release();
        event_instance_ = nullptr;
        state_ = AudioClipFmodState::stopped;
        event_callback_complete_.store(true, std::memory_order_relaxed);
        return false;
    }

    if (spatial_source_ != nullptr) {
        const auto attributes = audio_build_fmod_3d_attributes(
            spatial_source_->transform());
        event_instance_->set3DAttributes(&attributes);
    }

    const auto current_position_ms = position_ms();
    const auto elapsed_ms = std::max(
        current_position_ms - last_position_ms_, 0.0F);
    const auto fade_completed = advance_fade(fade_, elapsed_ms);
    advance_fade(volume_transition_, elapsed_ms);
    last_position_ms_ = current_position_ms;

    if (fade_completed && fade_.stop_when_complete) {
        prepare_for_audio_reset();
        return true;
    }
    if (state_ == AudioClipFmodState::playing) {
        event_instance_->setVolume(
            fade_.current * volume_transition_.current);
    }
    return true;
}

void FmodStudioSoundGenerator::mark_event_callback_complete() {
    event_callback_complete_.store(true, std::memory_order_relaxed);
}

void FmodStudioSoundGenerator::notify_event_started() {
    if (spatial_source_ != nullptr) {
        spatial_source_->notify_event_dsp_attached();
    }
}

FMOD_RESULT FmodStudioSoundGenerator::event_callback(
    FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
    FMOD::Studio::EventInstance* event_instance,
    void*) {
    void* user_data = nullptr;
    event_instance->getUserData(&user_data);
    auto* generator = static_cast<FmodStudioSoundGenerator*>(user_data);
    if (generator == nullptr) {
        return FMOD_OK;
    }

    if (type == FMOD_STUDIO_EVENT_CALLBACK_DESTROYED ||
        type == FMOD_STUDIO_EVENT_CALLBACK_STOPPED) {
        generator->mark_event_callback_complete();
    } else if (type == FMOD_STUDIO_EVENT_CALLBACK_STARTED) {
        generator->notify_event_started();
    }
    return FMOD_OK;
}

bool FmodStudioSoundGenerator::advance_fade(
    Fade& fade,
    float elapsed_ms) {
    if (fade.progress >= 1.0F) {
        return false;
    }

    fade.elapsed_ms += elapsed_ms;
    fade.progress = std::clamp(
        fade.elapsed_ms / fade.duration_ms, 0.0F, 1.0F);
    fade.current = fade.start +
        (fade.target - fade.start) * fade.progress;
    return fade.progress >= 1.0F;
}

}  // namespace rb4
