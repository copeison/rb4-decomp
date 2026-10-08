#pragma once

#include <cstdint>
#include <vector>

#include "audio/fmod/api/fmod_api.h"

namespace rb4 {

struct FmodAudioState;
struct EngineTransform;

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
    const char* name = nullptr;
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

class AudioClipFmodSpatialSource {
public:
    virtual ~AudioClipFmodSpatialSource() = default;
    virtual void notify_event_dsp_attached() {}
    virtual const EngineTransform& transform() const = 0;
};

struct AudioClipFmod {
    AudioClipFmodState state = AudioClipFmodState::uninitialized;
    FmodAudioState* audio_state = nullptr;
    AudioClipFmodSpatialSource* spatial_source = nullptr;
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
void audio_clip_fmod_pause(AudioClipFmod& clip);
void audio_clip_fmod_resume(AudioClipFmod& clip);
float audio_clip_fmod_get_channel_position_ms(const AudioClipFmod& clip);
float audio_clip_fmod_get_position_ms(const AudioClipFmod& clip);
void audio_clip_fmod_set_position_ms(
    AudioClipFmod& clip,
    float milliseconds);
void audio_clip_fmod_update_3d_attributes(AudioClipFmod& clip);
bool audio_clip_fmod_set_event_parameter(
    AudioClipFmod& clip,
    const char* name,
    float value);
bool audio_clip_fmod_get_event_parameter(
    const AudioClipFmod& clip,
    const char* name,
    float& value);

}  // namespace rb4
