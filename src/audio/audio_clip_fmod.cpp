#include "audio_clip_fmod.h"

#include <chrono>
#include <thread>

#include "fmod_audio_system.h"
#include "fmod_deferred_release.h"

namespace rb4 {

namespace {

constexpr FMOD_STUDIO_EVENT_CALLBACK_TYPE kAudioClipEventCallbacks =
    0x02 | 0x80 | 0x100;
constexpr FMOD_STUDIO_EVENT_CALLBACK_TYPE kEventDestroyed = 0x02;
constexpr FMOD_STUDIO_EVENT_CALLBACK_TYPE kCreateProgrammerSound = 0x80;
constexpr FMOD_STUDIO_EVENT_CALLBACK_TYPE kDestroyProgrammerSound = 0x100;

}  // namespace

// Reconstructed from eboot.elf at 0x266FD0 after base clip initialization.
void audio_clip_fmod_start(
    AudioClipFmod& clip,
    const AudioClipFmodPlayOptions& options) {
    clip.event_ready = false;
    clip.base_frequency = 0.0F;
    clip.dsp = nullptr;
    clip.channel = nullptr;
    clip.channel_group = nullptr;
    clip.bus = nullptr;
    clip.event_instance = nullptr;
    clip.stop_in_progress = false;
    clip.state = options.start_paused
        ? AudioClipFmodState::paused
        : AudioClipFmodState::playing;

    auto& audio = *clip.audio_state;
    audio.core_system->createDSP(
        audio_clip_fmod_dsp_description(), &clip.dsp);
    clip.dsp->setUserData(&clip);

    if (options.route == AudioClipFmodRoute::studio_event) {
        FMOD::Studio::EventDescription* description = nullptr;
        bool oneshot = true;
        if (audio.studio_system->getEvent(
                options.route_path, &description) == FMOD_OK) {
            description->isOneshot(&oneshot);
        }
        if (description != nullptr && !oneshot) {
            description->createInstance(&clip.event_instance);
            clip.event_instance->setUserData(&clip);
            for (const auto& parameter : options.event_parameters) {
                audio_clip_fmod_set_parameter(
                    clip, parameter.key, parameter.value);
            }
            clip.event_instance->setCallback(
                audio_clip_fmod_event_callback,
                kAudioClipEventCallbacks);
            clip.event_instance->setPaused(options.start_paused);
            clip.event_instance->start();
            return;
        }
    }

    audio.core_system->playDSP(
        clip.dsp, nullptr, true, &clip.channel);

    if (options.route == AudioClipFmodRoute::studio_bus &&
        audio.studio_system->getBus(options.route_path, &clip.bus) == FMOD_OK) {
        clip.bus->getChannelGroup(&clip.channel_group);
        clip.channel->setChannelGroup(clip.channel_group);
    }

    if (options.spatialized) {
        clip.channel->setMode(FMOD_3D);
        clip.channel->set3DSpread(options.spread_degrees);
    }
    if (clip.parent_channel_group != nullptr) {
        if (clip.channel_group != nullptr) {
            clip.parent_channel_group->addGroup(
                clip.channel_group, true, nullptr);
        } else {
            clip.channel->setChannelGroup(clip.parent_channel_group);
        }
    }

    clip.channel->getFrequency(&clip.base_frequency);
    clip.channel->setPaused(options.start_paused);
    if (clip.runtime_owner != nullptr) {
        clip.runtime_owner->attach(clip.runtime_link);
    }
}

// Reconstructed from eboot.elf at 0x267310. The DSP enumeration and plugin
// payload layout are kept behind a named adapter until those DSP types are
// reconstructed.
FMOD_RESULT audio_clip_fmod_event_callback(
    FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
    FMOD::Studio::EventInstance* event_instance,
    void* parameters) {
    (void)parameters;

    void* user_data = nullptr;
    event_instance->getUserData(&user_data);
    auto* clip = static_cast<AudioClipFmod*>(user_data);
    if (clip == nullptr) {
        return FMOD_OK;
    }

    if (type == kCreateProgrammerSound && !clip->event_ready &&
        clip->state != AudioClipFmodState::stopping) {
        audio_clip_fmod_bind_event_dsp(*clip, *event_instance);
    }
    if (type == kEventDestroyed || type == kCreateProgrammerSound ||
        type == kDestroyProgrammerSound) {
        clip->event_ready = true;
    }
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x2681E0.
void audio_clip_fmod_clear_dsp_user_data(AudioClipFmod& clip) {
    if (clip.runtime_owner != nullptr) {
        clip.runtime_owner->detach(clip.runtime_link);
        if (clip.runtime_owner != nullptr) {
            clip.runtime_owner->lock();
            clip.dsp->setUserData(nullptr);
            clip.runtime_owner->unlock();
            return;
        }
    }
    clip.dsp->setUserData(nullptr);
}

// Reconstructed from eboot.elf at 0x267FB0.
void audio_clip_fmod_defer_channel_release(AudioClipFmod& clip) {
    if (clip.channel == nullptr) {
        return;
    }

    clip.state = AudioClipFmodState::stopping;
    audio_clip_fmod_clear_dsp_user_data(clip);
    fmod_defer_channel_dsp_release(
        *clip.audio_state, clip.channel, clip.dsp);

    clip.channel = nullptr;
    clip.channel_group = nullptr;
    clip.bus = nullptr;
    clip.dsp = nullptr;
    clip.state = AudioClipFmodState::stopped;
}

// Reconstructed from eboot.elf at 0x268080.
void audio_clip_fmod_release_event_instance(AudioClipFmod& clip) {
    if (clip.event_instance == nullptr || !clip.event_ready) {
        return;
    }

    clip.state = AudioClipFmodState::stopping;
    audio_clip_fmod_clear_dsp_user_data(clip);

    FMOD::ChannelGroup* channel_group = nullptr;
    const auto result =
        clip.event_instance->getChannelGroup(&channel_group);
    if (result == FMOD_ERR_INVALID_HANDLE) {
        clip.channel_group = nullptr;
        clip.bus = nullptr;
        clip.event_instance = nullptr;
        clip.dsp = nullptr;
        clip.state = AudioClipFmodState::stopped;
        return;
    }
    if (result == FMOD_OK) {
        channel_group->removeDSP(clip.dsp);
    }

    clip.event_instance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
    clip.event_instance->release();
    clip.channel_group = nullptr;
    clip.bus = nullptr;
    clip.event_instance = nullptr;
    if (clip.dsp != nullptr) {
        clip.dsp->release();
    }
    clip.dsp = nullptr;
    clip.state = AudioClipFmodState::stopped;
}

// Reconstructed from eboot.elf at 0x267E50.
void audio_clip_fmod_process_release(AudioClipFmod& clip) {
    audio_clip_fmod_defer_channel_release(clip);
    audio_clip_fmod_release_event_instance(clip);
}

// Reconstructed from eboot.elf at 0x268260.
void audio_clip_fmod_release_dsp(AudioClipFmod& clip) {
    clip.channel_group = nullptr;
    clip.bus = nullptr;
    if (clip.dsp != nullptr) {
        clip.dsp->release();
    }
    clip.dsp = nullptr;
}

// Reconstructed from eboot.elf at 0x2682A0.
void audio_clip_fmod_stop_and_wait(AudioClipFmod& clip) {
    clip.stop_in_progress = true;
    audio_clip_fmod_process_release(clip);

    if (clip.audio_state != nullptr &&
        clip.audio_state->studio_system != nullptr) {
        while (clip.state != AudioClipFmodState::stopped) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            clip.audio_state->studio_system->update();
            audio_clip_fmod_process_release(clip);
        }
    } else {
        clip.state = AudioClipFmodState::stopping;
        if (clip.runtime_owner != nullptr) {
            clip.runtime_owner->detach(clip.runtime_link);
        }
        clip.dsp = nullptr;
        clip.channel = nullptr;
        clip.channel_group = nullptr;
        clip.bus = nullptr;
        clip.event_instance = nullptr;
        clip.state = AudioClipFmodState::stopped;
    }

    clip.stop_in_progress = false;
}

}  // namespace rb4
