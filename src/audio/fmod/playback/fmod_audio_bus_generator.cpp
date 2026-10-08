#include "audio/fmod/playback/fmod_audio_bus_generator.h"

#include <algorithm>

#include "audio/fmod/system/fmod_audio_system.h"
#include "audio/fmod/system/fmod_listener.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kActiveHandleBit = 0x80000000;
constexpr std::uint32_t kGenerationMask = 0x00003FFF;
constexpr std::uint32_t kPoolIndexMask = 0x00FFC000;
constexpr std::uint32_t kPoolIndexShift = 14;
constexpr FMOD_MODE kBusSoundMode2d = 0x00014080;
constexpr FMOD_MODE kBusSoundMode3d = 0x00014090;

}  // namespace

void FmodAudioBusGenerator::initialize_pool_slot(
    FmodAudioBusGeneratorManager& manager,
    std::uint32_t index) {
    manager_ = &manager;
    pool_index_ = index;
    reference_count_.store(0, std::memory_order_relaxed);
    handle_ = 0;
    sound_source_ = nullptr;
    assigned_audio_state_ = nullptr;
}

void FmodAudioBusGenerator::stop_and_wait() {
    if (playback_channel_ != nullptr) {
        playback_channel_->stop();
        playback_channel_ = nullptr;
    }
    state = AudioClipFmodState::stopped;
}

// Reconstructed from the default-mode branch at 0x267F10. Mode 1 keeps the
// voice alive and recalculates source timing; the bus generator starts in the
// default mode and is stopped before an audio-system reset.
void FmodAudioBusGenerator::prepare_for_audio_reset() {
    if (state != AudioClipFmodState::stopped) {
        state = AudioClipFmodState::stopping;
    }
}

// Reconstructed from eboot.elf at 0x268EF0.
bool FmodAudioBusGenerator::initialize_sound(
    const FmodAudioBusSound& sound,
    const FmodAudioBusGeneratorOptions& options) {
    sound_ = nullptr;
    playback_channel_ = nullptr;
    playback_bus_ = nullptr;
    bus_group_retries_ = 10;
    length_ms_ = 0;
    length_pcm_ = 0;
    base_frequency = 0.0F;
    start_paused_ = options.start_paused;
    volume_ = options.start_silent ? 0.0F : 1.0F;
    state = AudioClipFmodState::uninitialized;

    const auto mode = sound.spatialized
        ? kBusSoundMode3d
        : kBusSoundMode2d;
    if (assigned_audio_state_->core_system->createSound(
            sound.path, mode, nullptr, &sound_) != FMOD_OK) {
        return false;
    }

    state = AudioClipFmodState::ready;
    if (options.route == AudioClipFmodRoute::studio_bus &&
        assigned_audio_state_->studio_system->getBus(
            options.route_path, &playback_bus_) != FMOD_OK) {
        playback_bus_ = nullptr;
    }
    return true;
}

FmodAudioBusGeneratorManager::FmodAudioBusGeneratorManager(
    std::size_t capacity,
    FmodAudioState* default_audio_state)
    : capacity_(capacity), default_audio_state_(default_audio_state) {}

// Reconstructed from eboot.elf at 0x268510.
std::int32_t FmodAudioBusGeneratorManager::setting() const {
    return setting_;
}

// Reconstructed from eboot.elf at 0x268900.
void FmodAudioBusGeneratorManager::set_setting(std::int32_t value) {
    setting_ = value;
}

// Reconstructed from eboot.elf at 0x268910.
void FmodAudioBusGeneratorManager::initialize_pool() {
    std::lock_guard lock(mutex_);
    ++lock_depth_;

    generators_ = std::make_unique<FmodAudioBusGenerator[]>(capacity_);
    free_indices_.clear();
    for (std::uint32_t index = 0; index < capacity_; ++index) {
        generators_[index].initialize_pool_slot(*this, index);
        generators_[index].in_free_list_ = true;
        free_indices_.push_back(index);
    }

    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x268A60.
bool FmodAudioBusGeneratorManager::shutdown_pool() {
    std::lock_guard lock(mutex_);
    ++lock_depth_;

    if (free_indices_.size() != capacity_) {
        --lock_depth_;
        return false;
    }

    generators_.reset();
    free_indices_.clear();
    --lock_depth_;
    return true;
}

// Reconstructed from eboot.elf at 0x268660.
FmodAudioBusGenerator* FmodAudioBusGeneratorManager::retain(
    FmodAudioBusGeneratorHandle handle,
    std::uint32_t index) {
    std::lock_guard lock(mutex_);
    ++lock_depth_;

    FmodAudioBusGenerator* result = nullptr;
    if (generators_ != nullptr && index < capacity_) {
        auto& generator = generators_[index];
        if (generator.handle_ == handle) {
            generator.reference_count_.fetch_add(
                1, std::memory_order_relaxed);
            result = &generator;
        }
    }

    --lock_depth_;
    return result;
}

// Reconstructed from eboot.elf at 0x268B40.
FmodAudioBusGenerator* FmodAudioBusGeneratorManager::acquire(
    FmodAudioState* audio_state,
    void* sound_source) {
    if (audio_state == nullptr) {
        audio_state = default_audio_state_;
    }

    std::lock_guard lock(mutex_);
    ++lock_depth_;

    if (free_indices_.empty()) {
        --lock_depth_;
        return nullptr;
    }

    const auto index = free_indices_.front();
    free_indices_.pop_front();

    auto& generator = generators_[index];
    generator.in_free_list_ = false;
    generator.sound_source_ = sound_source;
    generator.assigned_audio_state_ = audio_state;
    generator.handle_ = activate_handle(generator);

    --lock_depth_;
    return &generator;
}

// Reconstructed from eboot.elf at 0x268380.
void FmodAudioBusGeneratorManager::release(
    FmodAudioBusGenerator& generator) {
    std::lock_guard lock(mutex_);
    ++lock_depth_;

    generator.handle_ &= ~kActiveHandleBit;
    generator.sound_source_ = nullptr;
    if (!generator.in_free_list_) {
        generator.in_free_list_ = true;
        free_indices_.push_back(generator.pool_index_);
    }

    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x2686D0.
void FmodAudioBusGeneratorManager::prepare_all_for_audio_reset() {
    std::lock_guard lock(mutex_);
    ++lock_depth_;
    for (std::size_t index = 0; index < capacity_; ++index) {
        generators_[index].prepare_for_audio_reset();
    }
    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x268740.
void FmodAudioBusGeneratorManager::stop_all() {
    std::lock_guard lock(mutex_);
    ++lock_depth_;
    for (std::size_t index = 0; index < capacity_; ++index) {
        generators_[index].stop_and_wait();
    }
    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x2687B0.
std::vector<FmodAudioBusGeneratorHandle>
FmodAudioBusGeneratorManager::active_handles() const {
    std::vector<FmodAudioBusGeneratorHandle> handles;
    handles.reserve(capacity_ - free_indices_.size());
    for (std::size_t index = 0; index < capacity_; ++index) {
        const auto handle = generators_[index].handle_;
        if (static_cast<std::int32_t>(handle) < 0) {
            handles.push_back(handle);
        }
    }
    return handles;
}

FmodAudioBusGeneratorHandle
FmodAudioBusGeneratorManager::activate_handle(
    FmodAudioBusGenerator& generator) const {
    const auto generation = (generator.handle_ + 1) & kGenerationMask;
    const auto pool_index =
        (generator.pool_index_ << kPoolIndexShift) & kPoolIndexMask;
    return kActiveHandleBit | pool_index | generation;
}

}  // namespace rb4
