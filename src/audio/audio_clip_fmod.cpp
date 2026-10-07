#include "audio_clip_fmod.h"

#include <chrono>
#include <thread>

#include "fmod_audio_system.h"
#include "fmod_deferred_release.h"

namespace rb4 {

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

    FMOD::ChannelControl* channel_group = nullptr;
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
