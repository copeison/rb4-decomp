#include "fmod_buffered_stream_generator.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kActiveHandleBit = 0x80000000;
constexpr std::uint32_t kGenerationMask = 0x00003FFF;
constexpr std::uint32_t kPoolIndexMask = 0x00FFC000;
constexpr std::uint32_t kPoolIndexShift = 14;
constexpr std::int32_t kBufferedStreamFormat = 3;

}  // namespace

// Reconstructed from eboot.elf at 0x26AFF0.
void FmodBufferedStreamGenerator::initialize_pool_slot(
    FmodBufferedStreamGeneratorManager& manager,
    std::uint32_t index) {
    manager_ = &manager;
    pool_index_ = index;
    reference_count_.store(0, std::memory_order_relaxed);
    handle_ = 0;
    sound_source_ = nullptr;
    audio_state_ = nullptr;
    bus_generator_ = nullptr;
    sound_ = nullptr;
    gain_ = 1.0F;
    position_seconds_ = 0.0F;
    sound_open_pending_ = false;
    state_ = AudioClipFmodState::stopped;
}

// Reconstructed from eboot.elf at 0x26BD40.
void FmodBufferedStreamGenerator::pause() {
    state_ = AudioClipFmodState::paused;
}

// Reconstructed from eboot.elf at 0x26BD70.
void FmodBufferedStreamGenerator::resume() {
    state_ = AudioClipFmodState::playing;
}

// Reconstructed from eboot.elf at 0x26C150.
void FmodBufferedStreamGenerator::prepare_for_audio_reset() {
    if (state_ != AudioClipFmodState::stopped) {
        state_ = AudioClipFmodState::stopping;
    }
}

// Reconstructed from eboot.elf at 0x26C180.
void FmodBufferedStreamGenerator::stop_and_wait() {
    state_ = AudioClipFmodState::stopped;
}

// Reconstructed from eboot.elf at 0x26BDC0.
void FmodBufferedStreamGenerator::set_position_seconds(float position) {
    position_seconds_ = position;
}

// Reconstructed from eboot.elf at 0x26BDB0.
float FmodBufferedStreamGenerator::position_seconds() const {
    return position_seconds_;
}

// Reconstructed from eboot.elf at 0x26C040.
void FmodBufferedStreamGenerator::set_gain(float gain) {
    gain_ = gain;
}

// Reconstructed from eboot.elf at 0x26C080.
float FmodBufferedStreamGenerator::gain() const {
    return gain_;
}

FmodBufferedStreamGeneratorManager::FmodBufferedStreamGeneratorManager(
    std::size_t capacity,
    FmodAudioState* default_audio_state)
    : capacity_(capacity), default_audio_state_(default_audio_state) {}

// Reconstructed from eboot.elf at 0x26DC00.
std::int32_t FmodBufferedStreamGeneratorManager::setting() const {
    return setting_;
}

// Reconstructed from eboot.elf at 0x26DFF0.
void FmodBufferedStreamGeneratorManager::set_setting(std::int32_t value) {
    setting_ = value;
}

// Reconstructed from the resource gate at 0x26AB60.
bool FmodBufferedStreamGeneratorManager::accepts(
    const FmodBufferedStreamOptions& options) const {
    return options.format == kBufferedStreamFormat && options.streaming;
}

// Reconstructed from eboot.elf at 0x26E000.
void FmodBufferedStreamGeneratorManager::initialize_pool() {
    std::lock_guard lock(mutex_);
    ++lock_depth_;

    generators_ =
        std::make_unique<FmodBufferedStreamGenerator[]>(capacity_);
    free_indices_.clear();
    for (std::uint32_t index = 0; index < capacity_; ++index) {
        generators_[index].initialize_pool_slot(*this, index);
        generators_[index].in_free_list_ = true;
        free_indices_.push_back(index);
    }

    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x26E270.
bool FmodBufferedStreamGeneratorManager::shutdown_pool() {
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

// Reconstructed from eboot.elf at 0x26DD50.
FmodBufferedStreamGenerator* FmodBufferedStreamGeneratorManager::retain(
    FmodBufferedStreamGeneratorHandle handle,
    std::uint32_t index) {
    std::lock_guard lock(mutex_);
    ++lock_depth_;

    FmodBufferedStreamGenerator* result = nullptr;
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

FmodBufferedStreamGenerator* FmodBufferedStreamGeneratorManager::acquire(
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
    generator.handle_ = activate_handle(generator);
    generator.state_ = AudioClipFmodState::uninitialized;

    --lock_depth_;
    return &generator;
}

// Reconstructed from eboot.elf at 0x26C1B0.
void FmodBufferedStreamGeneratorManager::release(
    FmodBufferedStreamGenerator& generator) {
    std::lock_guard lock(mutex_);
    ++lock_depth_;
    generator.handle_ &= ~kActiveHandleBit;
    generator.sound_source_ = nullptr;
    generator.bus_generator_ = nullptr;
    generator.sound_ = nullptr;
    generator.state_ = AudioClipFmodState::stopped;
    if (!generator.in_free_list_) {
        generator.in_free_list_ = true;
        free_indices_.push_back(generator.pool_index_);
    }
    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x26DDC0.
void FmodBufferedStreamGeneratorManager::prepare_all_for_audio_reset() {
    std::lock_guard lock(mutex_);
    ++lock_depth_;
    for (std::size_t index = 0; index < capacity_; ++index) {
        generators_[index].prepare_for_audio_reset();
    }
    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x26DE30.
void FmodBufferedStreamGeneratorManager::stop_all() {
    std::lock_guard lock(mutex_);
    ++lock_depth_;
    for (std::size_t index = 0; index < capacity_; ++index) {
        generators_[index].stop_and_wait();
    }
    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x26DEA0.
std::vector<FmodBufferedStreamGeneratorHandle>
FmodBufferedStreamGeneratorManager::active_handles() const {
    std::vector<FmodBufferedStreamGeneratorHandle> handles;
    handles.reserve(capacity_ - free_indices_.size());
    for (std::size_t index = 0; index < capacity_; ++index) {
        const auto handle = generators_[index].handle_;
        if (static_cast<std::int32_t>(handle) < 0) {
            handles.push_back(handle);
        }
    }
    return handles;
}

FmodBufferedStreamGeneratorHandle
FmodBufferedStreamGeneratorManager::activate_handle(
    FmodBufferedStreamGenerator& generator) const {
    const auto generation = (generator.handle_ + 1) & kGenerationMask;
    const auto pool_index =
        (generator.pool_index_ << kPoolIndexShift) & kPoolIndexMask;
    return kActiveHandleBit | pool_index | generation;
}

}  // namespace rb4
