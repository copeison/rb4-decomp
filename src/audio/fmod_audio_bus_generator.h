#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

#include "audio_clip_fmod.h"

namespace rb4 {

class FmodAudioBusGeneratorManager;

using FmodAudioBusGeneratorHandle = std::uint32_t;

struct FmodAudioBusSound {
    const char* path = nullptr;
    bool spatialized = false;
};

struct FmodAudioBusGeneratorOptions {
    bool start_paused = false;
    bool start_silent = false;
    AudioClipFmodRoute route = AudioClipFmodRoute::low_level;
    const char* route_path = nullptr;
};

class FmodAudioBusGenerator : public AudioClipFmod {
public:
    void initialize_pool_slot(
        FmodAudioBusGeneratorManager& manager,
        std::uint32_t index);
    void prepare_for_manager_update();
    void stop_and_wait();
    bool initialize_sound(
        const FmodAudioBusSound& sound,
        const FmodAudioBusGeneratorOptions& options);
    bool try_start_sound();
    bool update();
    void request_paused(bool paused);
    void set_position_ms(std::uint32_t position);

private:
    friend class FmodAudioBusGeneratorManager;

    FmodAudioBusGeneratorManager* manager_ = nullptr;
    std::uint32_t pool_index_ = 0;
    std::atomic<std::uint32_t> reference_count_{0};
    FmodAudioBusGeneratorHandle handle_ = 0;
    void* sound_source_ = nullptr;
    FmodAudioState* assigned_audio_state_ = nullptr;
    FMOD::Sound* sound_ = nullptr;
    FMOD::Channel* playback_channel_ = nullptr;
    FMOD::Studio::Bus* playback_bus_ = nullptr;
    std::uint32_t length_ms_ = 0;
    std::uint32_t length_pcm_ = 0;
    std::uint32_t current_position_ms_ = 0;
    std::uint32_t pending_position_ms_ = UINT32_MAX;
    std::int32_t bus_group_retries_ = 10;
    float volume_ = 1.0F;
    bool start_paused_ = false;
    bool in_free_list_ = false;
};

class FmodAudioBusGeneratorManager {
public:
    FmodAudioBusGeneratorManager(
        std::size_t capacity,
        FmodAudioState* default_audio_state);

    std::int32_t setting() const;
    void set_setting(std::int32_t value);

    void initialize_pool();
    bool shutdown_pool();

    FmodAudioBusGenerator* retain(
        FmodAudioBusGeneratorHandle handle,
        std::uint32_t index);
    FmodAudioBusGenerator* acquire(
        FmodAudioState* audio_state,
        void* sound_source);
    void release(FmodAudioBusGenerator& generator);

    void prepare_all();
    void stop_all();
    std::vector<FmodAudioBusGeneratorHandle> active_handles() const;

private:
    FmodAudioBusGeneratorHandle activate_handle(
        FmodAudioBusGenerator& generator) const;

    std::size_t capacity_ = 0;
    FmodAudioState* default_audio_state_ = nullptr;
    std::int32_t setting_ = 0;
    mutable std::recursive_mutex mutex_;
    std::uint32_t lock_depth_ = 0;
    std::unique_ptr<FmodAudioBusGenerator[]> generators_;
    std::deque<std::uint32_t> free_indices_;
};

}  // namespace rb4
