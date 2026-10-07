#include "fmod_dialog_generator.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kActiveHandleBit = 0x80000000;
constexpr std::uint32_t kGenerationMask = 0x00003FFF;
constexpr std::uint32_t kPoolIndexMask = 0x00FFC000;
constexpr std::uint32_t kPoolIndexShift = 14;
constexpr std::int32_t kDialogFormat = 4;
constexpr std::string_view kEventPrefix = "event:/";

}  // namespace

// Reconstructed from the path handling at eboot.elf 0x26EC60.
std::string fmod_dialog_event_path(std::string_view path) {
    if (path.size() >= kEventPrefix.size() &&
        path.substr(0, kEventPrefix.size()) == kEventPrefix) {
        return std::string(path);
    }
    return std::string(kEventPrefix) + std::string(path);
}

void FmodDialogGenerator::initialize_pool_slot(
    FmodDialogGeneratorManager& manager,
    std::uint32_t index) {
    manager_ = &manager;
    pool_index_ = index;
    reference_count_.store(0, std::memory_order_relaxed);
    handle_ = 0;
    sound_source_ = nullptr;
    audio_state_ = nullptr;
    state_ = AudioClipFmodState::stopped;
}

void FmodDialogGenerator::prepare_for_audio_reset() {
    if (state_ != AudioClipFmodState::stopped) {
        state_ = AudioClipFmodState::stopping;
    }
}

void FmodDialogGenerator::stop_and_wait() {
    state_ = AudioClipFmodState::stopped;
}

FmodDialogGeneratorManager::FmodDialogGeneratorManager(
    std::size_t capacity,
    FmodAudioState* default_audio_state)
    : capacity_(capacity), default_audio_state_(default_audio_state) {}

// Reconstructed from eboot.elf at 0x26EE80.
std::int32_t FmodDialogGeneratorManager::setting() const {
    return setting_;
}

// Reconstructed from eboot.elf at 0x26F270.
void FmodDialogGeneratorManager::set_setting(std::int32_t value) {
    setting_ = value;
}

// Reconstructed from the resource gate at eboot.elf 0x26EC60.
bool FmodDialogGeneratorManager::accepts(
    const FmodDialogGeneratorOptions& options) const {
    return options.format == kDialogFormat;
}

// Reconstructed from eboot.elf at 0x26F280.
void FmodDialogGeneratorManager::initialize_pool() {
    std::lock_guard lock(mutex_);
    ++lock_depth_;

    generators_ = std::make_unique<FmodDialogGenerator[]>(capacity_);
    free_indices_.clear();
    for (std::uint32_t index = 0; index < capacity_; ++index) {
        generators_[index].initialize_pool_slot(*this, index);
        generators_[index].in_free_list_ = true;
        free_indices_.push_back(index);
    }

    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x26F420.
bool FmodDialogGeneratorManager::shutdown_pool() {
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

// Reconstructed from eboot.elf at 0x26EFD0.
FmodDialogGenerator* FmodDialogGeneratorManager::retain(
    FmodDialogGeneratorHandle handle,
    std::uint32_t index) {
    std::lock_guard lock(mutex_);
    ++lock_depth_;

    FmodDialogGenerator* result = nullptr;
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

FmodDialogGenerator* FmodDialogGeneratorManager::acquire(
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

// Reconstructed from eboot.elf at 0x26EBE0.
void FmodDialogGeneratorManager::release(
    FmodDialogGenerator& generator) {
    std::lock_guard lock(mutex_);
    ++lock_depth_;
    generator.handle_ &= ~kActiveHandleBit;
    generator.sound_source_ = nullptr;
    generator.audio_state_ = nullptr;
    generator.state_ = AudioClipFmodState::stopped;
    if (!generator.in_free_list_) {
        generator.in_free_list_ = true;
        free_indices_.push_back(generator.pool_index_);
    }
    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x26F040.
void FmodDialogGeneratorManager::prepare_all_for_audio_reset() {
    std::lock_guard lock(mutex_);
    ++lock_depth_;
    for (std::size_t index = 0; index < capacity_; ++index) {
        generators_[index].prepare_for_audio_reset();
    }
    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x26F0B0.
void FmodDialogGeneratorManager::stop_all() {
    std::lock_guard lock(mutex_);
    ++lock_depth_;
    for (std::size_t index = 0; index < capacity_; ++index) {
        generators_[index].stop_and_wait();
    }
    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x26F120.
std::vector<FmodDialogGeneratorHandle>
FmodDialogGeneratorManager::active_handles() const {
    std::vector<FmodDialogGeneratorHandle> handles;
    handles.reserve(capacity_ - free_indices_.size());
    for (std::size_t index = 0; index < capacity_; ++index) {
        const auto handle = generators_[index].handle_;
        if (static_cast<std::int32_t>(handle) < 0) {
            handles.push_back(handle);
        }
    }
    return handles;
}

FmodDialogGeneratorHandle FmodDialogGeneratorManager::activate_handle(
    FmodDialogGenerator& generator) const {
    const auto generation = (generator.handle_ + 1) & kGenerationMask;
    const auto pool_index =
        (generator.pool_index_ << kPoolIndexShift) & kPoolIndexMask;
    return kActiveHandleBit | pool_index | generation;
}

}  // namespace rb4
