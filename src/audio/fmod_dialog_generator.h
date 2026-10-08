#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "audio_clip_fmod.h"
#include "fmod_studio_sound_generator.h"

namespace rb4 {

class FmodDialogGeneratorManager;

using FmodDialogGeneratorHandle = std::uint32_t;

struct FmodDialogGeneratorOptions {
    std::int32_t format = 4;
    FmodStudioSoundOptions sound;
};

std::string fmod_dialog_event_path(const std::string& path);

class FmodDialogGenerator {
public:
    void initialize_pool_slot(
        FmodDialogGeneratorManager& manager,
        std::uint32_t index);
    bool initialize_event(
        const char* event_path,
        const FmodDialogGeneratorOptions& options,
        AudioClipFmodSpatialSource* spatial_source);
    void pause();
    void resume();
    void prepare_for_audio_reset();
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

private:
    friend class FmodDialogGeneratorManager;

    static FMOD_RESULT programmer_sound_callback(
        FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
        FMOD::Studio::EventInstance* event_instance,
        void* parameters);
    FMOD_RESULT create_programmer_sound(
        FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES& properties);

    FmodDialogGeneratorManager* manager_ = nullptr;
    std::uint32_t pool_index_ = 0;
    std::atomic<std::uint32_t> reference_count_{0};
    FmodDialogGeneratorHandle handle_ = 0;
    void* sound_source_ = nullptr;
    FmodAudioState* audio_state_ = nullptr;
    FmodStudioSoundGenerator studio_generator_;
    float length_ms_ = 3600000.0F;
    bool in_free_list_ = false;
    AudioClipFmodState state_ = AudioClipFmodState::stopped;
};

class FmodDialogGeneratorManager {
public:
    FmodDialogGeneratorManager(
        std::size_t capacity,
        FmodAudioState* default_audio_state);

    std::int32_t setting() const;
    void set_setting(std::int32_t value);
    bool accepts(const FmodDialogGeneratorOptions& options) const;

    void initialize_pool();
    bool shutdown_pool();

    FmodDialogGenerator* retain(
        FmodDialogGeneratorHandle handle,
        std::uint32_t index);
    FmodDialogGenerator* acquire(
        FmodAudioState* audio_state,
        void* sound_source);
    void release(FmodDialogGenerator& generator);

    void prepare_all_for_audio_reset();
    void stop_all();
    std::vector<FmodDialogGeneratorHandle> active_handles() const;

private:
    FmodDialogGeneratorHandle activate_handle(
        FmodDialogGenerator& generator) const;

    std::size_t capacity_ = 0;
    FmodAudioState* default_audio_state_ = nullptr;
    std::int32_t setting_ = 0;
    mutable std::recursive_mutex mutex_;
    std::uint32_t lock_depth_ = 0;
    std::unique_ptr<FmodDialogGenerator[]> generators_;
    std::deque<std::uint32_t> free_indices_;
};

}  // namespace rb4
