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

class FmodAudioBusGenerator : public AudioClipFmod {
public:
    void initialize_pool_slot(
        FmodAudioBusGeneratorManager& manager,
        std::uint32_t index);
    void prepare_for_manager_update();
    void stop_and_wait();

private:
    friend class FmodAudioBusGeneratorManager;

    FmodAudioBusGeneratorManager* manager_ = nullptr;
    std::uint32_t pool_index_ = 0;
    std::atomic<std::uint32_t> reference_count_{0};
    FmodAudioBusGeneratorHandle handle_ = 0;
    void* sound_source_ = nullptr;
    FmodAudioState* assigned_audio_state_ = nullptr;
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
