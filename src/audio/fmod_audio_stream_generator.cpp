#include "fmod_audio_stream_generator.h"

#include "fmod_audio_system.h"
#include "fmod_listener.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kActiveHandleBit = 0x80000000;
constexpr std::uint32_t kGenerationMask = 0x00003FFF;
constexpr std::uint32_t kPoolIndexMask = 0x00FFC000;
constexpr std::uint32_t kPoolIndexShift = 14;
constexpr std::int32_t kOpenStateReady = 0;
constexpr FMOD_RESULT kBusGroupNotReady =
    static_cast<FMOD_RESULT>(76);

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
    spatial_source_ = nullptr;
    sound_ = nullptr;
    channel_ = nullptr;
    bus_ = nullptr;
    bus_group_retries_ = 10;
    current_position_ms_ = 0;
    pending_position_ms_ = UINT32_MAX;
    gain_ = 1.0F;
    playback_rate_ = 1.0F;
    loop_start_ms_ = -1.0F;
    loop_end_ms_ = -1.0F;
    playback_rate_dirty_ = false;
    loop_points_dirty_ = false;
    pause_requested_ = false;
    state_ = AudioClipFmodState::stopped;
}

void FmodAudioStreamGenerator::prepare_for_audio_reset() {
    if (state_ != AudioClipFmodState::stopped) {
        state_ = AudioClipFmodState::stopping;
    }
}

void FmodAudioStreamGenerator::stop_and_wait() {
    if (channel_ != nullptr) {
        channel_->stop();
        channel_ = nullptr;
    }
    if (sound_ != nullptr) {
        sound_->release();
        sound_ = nullptr;
    }
    state_ = AudioClipFmodState::stopped;
}

// Reconstructed from eboot.elf at 0x269980.
bool FmodAudioStreamGenerator::try_start_sound() {
    if (sound_ == nullptr || audio_state_ == nullptr ||
        audio_state_->core_system == nullptr) {
        return false;
    }

    std::int32_t open_state = 0;
    std::uint32_t percent_buffered = 0;
    bool starving = false;
    bool disk_busy = false;
    sound_->getOpenState(
        &open_state, &percent_buffered, &starving, &disk_busy);

    FMOD::ChannelGroup* channel_group = nullptr;
    if (bus_ != nullptr) {
        const auto result = bus_->getChannelGroup(&channel_group);
        if (result != FMOD_OK) {
            if (result == kBusGroupNotReady && --bus_group_retries_ > 0) {
                return false;
            }
            channel_group = nullptr;
        }
    }
    if (open_state != kOpenStateReady) {
        return false;
    }

    sound_->getLength(&length_ms_, FMOD_TIMEUNIT_MS);
    sound_->getLength(&length_pcm_, FMOD_TIMEUNIT_PCM);
    audio_state_->core_system->playSound(
        sound_, channel_group, true, &channel_);
    channel_->setMode(FMOD_LOOP_NORMAL);
    channel_->setLoopCount(0);
    channel_->getFrequency(&base_frequency_);
    channel_->setVolume(gain_);

    if (pause_requested_) {
        state_ = AudioClipFmodState::paused;
    } else {
        channel_->setPaused(false);
        state_ = AudioClipFmodState::playing;
    }
    return true;
}

// Reconstructed from eboot.elf at 0x2694D0.
bool FmodAudioStreamGenerator::update() {
    if (state_ == AudioClipFmodState::stopped) {
        return false;
    }

    if (channel_ != nullptr) {
        bool playing = false;
        const auto result = channel_->isPlaying(&playing);
        if (result == FMOD_ERR_INVALID_HANDLE) {
            channel_ = nullptr;
            state_ = AudioClipFmodState::stopping;
        } else if (result != FMOD_OK) {
            state_ = AudioClipFmodState::stopping;
        }
    }

    if (state_ == AudioClipFmodState::ready) {
        try_start_sound();
    } else if (state_ == AudioClipFmodState::stopping) {
        stop_and_wait();
        return false;
    }

    if (channel_ == nullptr) {
        return true;
    }

    if (playback_rate_dirty_) {
        if (playback_rate_ > 0.0F) {
            channel_->setFrequency(base_frequency_ * playback_rate_);
        }
        playback_rate_dirty_ = false;
    }

    if (loop_points_dirty_) {
        if (loop_end_ms_ >= 0.0F) {
            channel_->setLoopPoints(
                static_cast<std::uint32_t>(loop_start_ms_),
                FMOD_TIMEUNIT_MS,
                static_cast<std::uint32_t>(loop_end_ms_),
                FMOD_TIMEUNIT_MS);
            channel_->setLoopCount(-1);
        } else if (loop_start_ms_ >= 0.0F) {
            channel_->setLoopPoints(
                static_cast<std::uint32_t>(loop_start_ms_),
                FMOD_TIMEUNIT_MS,
                length_pcm_ - 1,
                FMOD_TIMEUNIT_PCM);
            channel_->setLoopCount(-1);
        } else {
            channel_->setLoopCount(0);
        }
        loop_points_dirty_ = false;
    }

    if (state_ == AudioClipFmodState::playing) {
        channel_->setVolume(gain_);
        channel_->getPosition(&current_position_ms_, FMOD_TIMEUNIT_MS);
        if (pending_position_ms_ != UINT32_MAX &&
            pending_position_ms_ != current_position_ms_ &&
            channel_->setPosition(
                pending_position_ms_, FMOD_TIMEUNIT_MS) == FMOD_OK) {
            current_position_ms_ = pending_position_ms_;
            pending_position_ms_ = UINT32_MAX;
        }
    }

    bool paused = false;
    if (channel_->getPaused(&paused) == FMOD_OK &&
        paused != pause_requested_ &&
        channel_->setPaused(pause_requested_) == FMOD_OK) {
        state_ = pause_requested_
            ? AudioClipFmodState::paused
            : AudioClipFmodState::playing;
    }

    if (spatial_source_ != nullptr) {
        const auto attributes = audio_build_fmod_3d_attributes(
            spatial_source_->transform());
        channel_->set3DAttributes(
            &attributes.position, &attributes.velocity, nullptr);
    }
    return true;
}

void FmodAudioStreamGenerator::request_paused(bool paused) {
    pause_requested_ = paused;
}

void FmodAudioStreamGenerator::set_position_ms(std::uint32_t position) {
    pending_position_ms_ = position;
}

void FmodAudioStreamGenerator::set_playback_rate(float rate) {
    if (playback_rate_ != rate) {
        playback_rate_ = rate;
        playback_rate_dirty_ = true;
    }
}

float FmodAudioStreamGenerator::playback_rate() const {
    return playback_rate_;
}

void FmodAudioStreamGenerator::clear_loop_points() {
    loop_start_ms_ = -1.0F;
    loop_end_ms_ = -1.0F;
    loop_points_dirty_ = true;
}

void FmodAudioStreamGenerator::set_loop_points(
    float start_ms,
    float end_ms) {
    loop_start_ms_ = start_ms;
    loop_end_ms_ = end_ms;
    loop_points_dirty_ = true;
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
