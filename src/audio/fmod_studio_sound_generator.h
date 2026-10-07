#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

#include "audio_clip_fmod.h"

namespace rb4 {

class FmodStudioSoundGeneratorManager;

using FmodStudioSoundGeneratorHandle = std::uint32_t;

struct FmodStudioSoundOptions {
    std::int32_t format = 0;
    bool start_paused = false;
    bool start_silent = false;
    bool configure_initial_fade = false;
    float initial_fade_db = 0.0F;
    float initial_fade_seconds = 0.0F;
    std::int32_t initial_fade_completion_mode = 0;
    std::vector<AudioClipFmodParameter> parameters;
};

std::string fmod_studio_event_path(std::string_view path);

class FmodStudioSoundGenerator {
public:
    void initialize_pool_slot(
        FmodStudioSoundGeneratorManager& manager,
        std::uint32_t index);
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
    friend class FmodStudioSoundGeneratorManager;

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

    FmodStudioSoundGeneratorManager* manager_ = nullptr;
    std::uint32_t pool_index_ = 0;
    std::atomic<std::uint32_t> reference_count_{0};
    FmodStudioSoundGeneratorHandle handle_ = 0;
    void* sound_source_ = nullptr;
    FmodAudioState* audio_state_ = nullptr;
    AudioClipFmodSpatialSource* spatial_source_ = nullptr;
    AudioClipFmodState state_ = AudioClipFmodState::uninitialized;
    FMOD::Studio::EventInstance* event_instance_ = nullptr;
    float length_ms_ = 0.0F;
    float last_position_ms_ = 0.0F;
    Fade fade_;
    Fade volume_transition_;
    bool volume_transition_requested_ = false;
    bool in_free_list_ = false;
    std::atomic_bool event_callback_complete_{true};
};

class FmodStudioSoundGeneratorManager {
public:
    FmodStudioSoundGeneratorManager(
        std::size_t capacity,
        FmodAudioState* default_audio_state);

    std::int32_t setting() const;
    void set_setting(std::int32_t value);
    bool accepts(const FmodStudioSoundOptions& options) const;

    void initialize_pool();
    bool shutdown_pool();

    FmodStudioSoundGenerator* create(
        const char* event_path,
        const FmodStudioSoundOptions& options,
        FmodAudioState* audio_state,
        void* sound_source,
        AudioClipFmodSpatialSource* spatial_source);
    FmodStudioSoundGenerator* retain(
        FmodStudioSoundGeneratorHandle handle,
        std::uint32_t index);
    FmodStudioSoundGenerator* acquire(
        FmodAudioState* audio_state,
        void* sound_source);
    void release(FmodStudioSoundGenerator& generator);

    void prepare_all_for_audio_reset();
    void stop_all();
    std::vector<FmodStudioSoundGeneratorHandle> active_handles() const;

private:
    FmodStudioSoundGeneratorHandle activate_handle(
        FmodStudioSoundGenerator& generator) const;

    std::size_t capacity_ = 0;
    FmodAudioState* default_audio_state_ = nullptr;
    std::int32_t setting_ = 0;
    mutable std::recursive_mutex mutex_;
    std::uint32_t lock_depth_ = 0;
    std::unique_ptr<FmodStudioSoundGenerator[]> generators_;
    std::deque<std::uint32_t> free_indices_;
};

}  // namespace rb4
