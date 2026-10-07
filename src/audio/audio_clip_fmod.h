#pragma once

#include "fmod_api.h"

namespace rb4 {

struct FmodAudioState;

enum class AudioClipFmodState : std::int32_t {
    uninitialized = 0,
    ready = 2,
    playing = 3,
    paused = 4,
    stopped = 5,
    stopping = 6,
};

// Semantic interface for the clip's intrusive runtime owner. Its concrete
// list and recursive-mutex layout remain in the IDA database.
class AudioClipFmodRuntimeOwner {
public:
    void detach(void* link);
    void lock();
    void unlock();
};

struct AudioClipFmod {
    AudioClipFmodState state = AudioClipFmodState::uninitialized;
    FmodAudioState* audio_state = nullptr;
    void* runtime_link = nullptr;
    AudioClipFmodRuntimeOwner* runtime_owner = nullptr;
    FMOD::DSP* dsp = nullptr;
    FMOD::ChannelControl* channel = nullptr;
    FMOD::ChannelControl* channel_group = nullptr;
    void* bus = nullptr;
    FMOD::Studio::EventInstance* event_instance = nullptr;
    bool event_ready = false;
    bool stop_in_progress = false;
};

void audio_clip_fmod_clear_dsp_user_data(AudioClipFmod& clip);
void audio_clip_fmod_defer_channel_release(AudioClipFmod& clip);
void audio_clip_fmod_release_event_instance(AudioClipFmod& clip);
void audio_clip_fmod_process_release(AudioClipFmod& clip);
void audio_clip_fmod_release_dsp(AudioClipFmod& clip);
void audio_clip_fmod_stop_and_wait(AudioClipFmod& clip);

}  // namespace rb4
