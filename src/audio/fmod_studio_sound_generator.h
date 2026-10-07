#pragma once

#include <atomic>
#include <cstdint>
#include <vector>

#include "audio_clip_fmod.h"

namespace rb4 {

struct FmodStudioSoundOptions {
    bool start_paused = false;
    bool start_silent = false;
    bool configure_initial_fade = false;
    float initial_fade_db = 0.0F;
    float initial_fade_seconds = 0.0F;
    std::int32_t initial_fade_completion_mode = 0;
    std::vector<AudioClipFmodParameter> parameters;
};

class FmodStudioSoundGenerator {
public:
    bool initialize_event(
        const char* event_path,
        const FmodStudioSoundOptions& options,
        FmodAudioState& audio_state,
        AudioClipFmodSpatialSource* spatial_source,
        FMOD::Studio::EventCallback callback = nullptr,
        void* callback_user_data = nullptr);

    void pause();
    void resume();
    void prepare_for_audio_reset();
    void release_event();
    void stop_and_wait();
    bool update();

    AudioClipFmodState state() const;
    float position_ms() const;
    float channel_position_ms() const;
    float length_ms() const;
    void set_position_ms(float position);

    bool set_event_parameter(const char* name, float value);
    bool get_event_parameter(const char* name, float& value) const;

    void configure_fade(
        std::int32_t completion_mode,
        float target,
        float duration_seconds);
    float fade_value() const;
    void configure_volume_transition(bool fade_out, bool immediate);
    bool volume_transition_requested() const;

    void mark_event_callback_complete();
    void notify_event_started();

private:
    struct Fade {
        float start = 1.0F;
        float current = 1.0F;
        float target = 1.0F;
        float duration_ms = 0.0F;
        float progress = 1.0F;
        float elapsed_ms = 0.0F;
        bool stop_when_complete = false;
    };

    static FMOD_RESULT event_callback(
        FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
        FMOD::Studio::EventInstance* event_instance,
        void* parameters);
    static bool advance_fade(Fade& fade, float elapsed_ms);

    FmodAudioState* audio_state_ = nullptr;
    AudioClipFmodSpatialSource* spatial_source_ = nullptr;
    AudioClipFmodState state_ = AudioClipFmodState::uninitialized;
    FMOD::Studio::EventInstance* event_instance_ = nullptr;
    float length_ms_ = 0.0F;
    float last_position_ms_ = 0.0F;
    Fade fade_;
    Fade volume_transition_;
    bool volume_transition_requested_ = false;
    std::atomic_bool event_callback_complete_{true};
};

}  // namespace rb4
