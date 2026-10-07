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

class FmodDialogGeneratorManager;

using FmodDialogGeneratorHandle = std::uint32_t;

struct FmodDialogGeneratorOptions {
    std::int32_t format = 4;
};

std::string fmod_dialog_event_path(std::string_view path);

class FmodDialogGenerator {
public:
    void initialize_pool_slot(
        FmodDialogGeneratorManager& manager,
        std::uint32_t index);
    void prepare_for_audio_reset();
    void stop_and_wait();

private:
    friend class FmodDialogGeneratorManager;

    FmodDialogGeneratorManager* manager_ = nullptr;
    std::uint32_t pool_index_ = 0;
    std::atomic<std::uint32_t> reference_count_{0};
    FmodDialogGeneratorHandle handle_ = 0;
    void* sound_source_ = nullptr;
    FmodAudioState* audio_state_ = nullptr;
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
