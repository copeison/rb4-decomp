#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

#include "audio/fmod/playback/audio_clip_fmod.h"

namespace rb4 {

class FmodAudioStreamGeneratorManager;

using FmodAudioStreamGeneratorHandle = std::uint32_t;

class FmodAudioStreamGenerator {
public:
    void initialize_pool_slot(
        FmodAudioStreamGeneratorManager& manager,
        std::uint32_t index);
    void prepare_for_audio_reset();
    void stop_and_wait();
    bool try_start_sound();
    bool update();
    void request_paused(bool paused);
    void set_position_ms(std::uint32_t position);
    void set_playback_rate(float rate);
    float playback_rate() const;
    void clear_loop_points();
    void set_loop_points(float start_ms, float end_ms);

private:
    friend class FmodAudioStreamGeneratorManager;

    FmodAudioStreamGeneratorManager* manager_ = nullptr;
    std::uint32_t pool_index_ = 0;
    std::atomic<std::uint32_t> reference_count_{0};
    FmodAudioStreamGeneratorHandle handle_ = 0;
    void* sound_source_ = nullptr;
    FmodAudioState* audio_state_ = nullptr;
    AudioClipFmodSpatialSource* spatial_source_ = nullptr;
    FMOD::Sound* sound_ = nullptr;
    FMOD::Channel* channel_ = nullptr;
    FMOD::Studio::Bus* bus_ = nullptr;
    std::int32_t bus_group_retries_ = 10;
    std::uint32_t length_ms_ = 0;
    std::uint32_t current_position_ms_ = 0;
    std::uint32_t pending_position_ms_ = UINT32_MAX;
    std::uint32_t length_pcm_ = 0;
    float base_frequency_ = 0.0F;
    float gain_ = 1.0F;
    float playback_rate_ = 1.0F;
    float loop_start_ms_ = -1.0F;
    float loop_end_ms_ = -1.0F;
    bool playback_rate_dirty_ = false;
    bool loop_points_dirty_ = false;
    bool pause_requested_ = false;
    bool in_free_list_ = false;
    AudioClipFmodState state_ = AudioClipFmodState::stopped;
};

class FmodAudioStreamGeneratorManager {
public:
    FmodAudioStreamGeneratorManager(
        std::size_t capacity,
        FmodAudioState* default_audio_state);

    std::int32_t setting() const;
    void set_setting(std::int32_t value);

    void initialize_pool();
    bool shutdown_pool();

    FmodAudioStreamGenerator* retain(
        FmodAudioStreamGeneratorHandle handle,
        std::uint32_t index);
    FmodAudioStreamGenerator* acquire(
        FmodAudioState* audio_state,
        void* sound_source);
    void release(FmodAudioStreamGenerator& generator);

    void prepare_all_for_audio_reset();
    void stop_all();
    std::vector<FmodAudioStreamGeneratorHandle> active_handles() const;

private:
    FmodAudioStreamGeneratorHandle activate_handle(
        FmodAudioStreamGenerator& generator) const;

    std::size_t capacity_ = 0;
    FmodAudioState* default_audio_state_ = nullptr;
    std::int32_t setting_ = 0;
    mutable std::recursive_mutex mutex_;
    std::uint32_t lock_depth_ = 0;
    std::unique_ptr<FmodAudioStreamGenerator[]> generators_;
    std::deque<std::uint32_t> free_indices_;
};

}  // namespace rb4
