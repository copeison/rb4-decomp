#include "fmod_audio_stream_generator.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kActiveHandleBit = 0x80000000;
constexpr std::uint32_t kGenerationMask = 0x00003FFF;
constexpr std::uint32_t kPoolIndexMask = 0x00FFC000;
constexpr std::uint32_t kPoolIndexShift = 14;

}  // namespace

void FmodAudioStreamGenerator::initialize_pool_slot(
    FmodAudioStreamGeneratorManager& manager,
    std::uint32_t index) {
    manager_ = &manager;
    pool_index_ = index;
    reference_count_.store(0, std::memory_order_relaxed);
    handle_ = 0;
    sound_source_ = nullptr;
    audio_state_ = nullptr;
    state_ = AudioClipFmodState::stopped;
}

void FmodAudioStreamGenerator::prepare_for_audio_reset() {
    if (state_ != AudioClipFmodState::stopped) {
        state_ = AudioClipFmodState::stopping;
    }
}

void FmodAudioStreamGenerator::stop_and_wait() {
    state_ = AudioClipFmodState::stopped;
}

FmodAudioStreamGeneratorManager::FmodAudioStreamGeneratorManager(
    std::size_t capacity,
    FmodAudioState* default_audio_state)
    : capacity_(capacity), default_audio_state_(default_audio_state) {}

// Reconstructed from eboot.elf at 0x26A280.
std::int32_t FmodAudioStreamGeneratorManager::setting() const {
    return setting_;
}

// Reconstructed from eboot.elf at 0x26A680.
void FmodAudioStreamGeneratorManager::set_setting(std::int32_t value) {
    setting_ = value;
}

// Reconstructed from eboot.elf at 0x26A690.
void FmodAudioStreamGeneratorManager::initialize_pool() {
    std::lock_guard lock(mutex_);
    ++lock_depth_;

    generators_ = std::make_unique<FmodAudioStreamGenerator[]>(capacity_);
    free_indices_.clear();
    for (std::uint32_t index = 0; index < capacity_; ++index) {
        generators_[index].initialize_pool_slot(*this, index);
        generators_[index].in_free_list_ = true;
        free_indices_.push_back(index);
    }

    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x26A850.
bool FmodAudioStreamGeneratorManager::shutdown_pool() {
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

// Reconstructed from eboot.elf at 0x26A3D0.
FmodAudioStreamGenerator* FmodAudioStreamGeneratorManager::retain(
    FmodAudioStreamGeneratorHandle handle,
    std::uint32_t index) {
    std::lock_guard lock(mutex_);
    ++lock_depth_;

    FmodAudioStreamGenerator* result = nullptr;
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

// Pool removal recovered as part of the creation path at 0x26AC30.
FmodAudioStreamGenerator* FmodAudioStreamGeneratorManager::acquire(
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
    generator.audio_state_ = audio_state;
    generator.state_ = AudioClipFmodState::uninitialized;
    generator.handle_ = activate_handle(generator);

    --lock_depth_;
    return &generator;
}

void FmodAudioStreamGeneratorManager::release(
    FmodAudioStreamGenerator& generator) {
    std::lock_guard lock(mutex_);
    ++lock_depth_;
    generator.handle_ &= ~kActiveHandleBit;
    generator.sound_source_ = nullptr;
    generator.state_ = AudioClipFmodState::stopped;
    if (!generator.in_free_list_) {
        generator.in_free_list_ = true;
        free_indices_.push_back(generator.pool_index_);
    }
    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x26A450.
void FmodAudioStreamGeneratorManager::prepare_all_for_audio_reset() {
    std::lock_guard lock(mutex_);
    ++lock_depth_;
    for (std::size_t index = 0; index < capacity_; ++index) {
        generators_[index].prepare_for_audio_reset();
    }
    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x26A4C0.
void FmodAudioStreamGeneratorManager::stop_all() {
    std::lock_guard lock(mutex_);
    ++lock_depth_;
    for (std::size_t index = 0; index < capacity_; ++index) {
        generators_[index].stop_and_wait();
    }
    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x26A530.
std::vector<FmodAudioStreamGeneratorHandle>
FmodAudioStreamGeneratorManager::active_handles() const {
    std::vector<FmodAudioStreamGeneratorHandle> handles;
    handles.reserve(capacity_ - free_indices_.size());
    for (std::size_t index = 0; index < capacity_; ++index) {
        const auto handle = generators_[index].handle_;
        if (static_cast<std::int32_t>(handle) < 0) {
            handles.push_back(handle);
        }
    }
    return handles;
}

FmodAudioStreamGeneratorHandle
FmodAudioStreamGeneratorManager::activate_handle(
    FmodAudioStreamGenerator& generator) const {
    const auto generation = (generator.handle_ + 1) & kGenerationMask;
    const auto pool_index =
        (generator.pool_index_ << kPoolIndexShift) & kPoolIndexMask;
    return kActiveHandleBit | pool_index | generation;
}

}  // namespace rb4
