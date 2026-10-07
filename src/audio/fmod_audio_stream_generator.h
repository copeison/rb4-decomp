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

class FmodAudioStreamGeneratorManager;

using FmodAudioStreamGeneratorHandle = std::uint32_t;

class FmodAudioStreamGenerator {
public:
    void initialize_pool_slot(
        FmodAudioStreamGeneratorManager& manager,
        std::uint32_t index);
    void prepare_for_audio_reset();
    void stop_and_wait();

private:
    friend class FmodAudioStreamGeneratorManager;

    FmodAudioStreamGeneratorManager* manager_ = nullptr;
    std::uint32_t pool_index_ = 0;
    std::atomic<std::uint32_t> reference_count_{0};
    FmodAudioStreamGeneratorHandle handle_ = 0;
    void* sound_source_ = nullptr;
    FmodAudioState* audio_state_ = nullptr;
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
