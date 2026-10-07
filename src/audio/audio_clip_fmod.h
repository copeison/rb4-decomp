#pragma once

#include <cstdint>
#include <vector>

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

enum class AudioClipFmodRoute : std::int32_t {
    low_level = 0,
    studio_event = 1,
    studio_bus = 2,
};

struct AudioClipFmodParameter {
    std::uint64_t key = 0;
    float value = 0.0F;
};

struct AudioClipFmodPlayOptions {
    bool start_paused = false;
    bool spatialized = false;
    float spread_degrees = 0.0F;
    AudioClipFmodRoute route = AudioClipFmodRoute::low_level;
    const char* route_path = nullptr;
    std::vector<AudioClipFmodParameter> event_parameters;
};

// Semantic interface for the clip's intrusive runtime owner. Its concrete
// list and recursive-mutex layout remain in the IDA database.
class AudioClipFmodRuntimeOwner {
public:
    void attach(void* link);
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
    FMOD::Channel* channel = nullptr;
    FMOD::ChannelGroup* channel_group = nullptr;
    FMOD::Studio::Bus* bus = nullptr;
    FMOD::Studio::EventInstance* event_instance = nullptr;
    FMOD::ChannelGroup* parent_channel_group = nullptr;
    float base_frequency = 0.0F;
    bool event_ready = false;
    bool stop_in_progress = false;
};

const FMOD_DSP_DESCRIPTION* audio_clip_fmod_dsp_description();
void audio_clip_fmod_set_parameter(
    AudioClipFmod& clip,
    std::uint64_t key,
    float value);
void audio_clip_fmod_bind_event_dsp(
    AudioClipFmod& clip,
    FMOD::Studio::EventInstance& event_instance);
FMOD_RESULT audio_clip_fmod_event_callback(
    FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
    FMOD::Studio::EventInstance* event_instance,
    void* parameters);

void audio_clip_fmod_start(
    AudioClipFmod& clip,
    const AudioClipFmodPlayOptions& options);
void audio_clip_fmod_clear_dsp_user_data(AudioClipFmod& clip);
void audio_clip_fmod_defer_channel_release(AudioClipFmod& clip);
void audio_clip_fmod_release_event_instance(AudioClipFmod& clip);
void audio_clip_fmod_process_release(AudioClipFmod& clip);
void audio_clip_fmod_release_dsp(AudioClipFmod& clip);
void audio_clip_fmod_stop_and_wait(AudioClipFmod& clip);

}  // namespace rb4
