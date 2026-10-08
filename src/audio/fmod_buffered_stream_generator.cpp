#include "fmod_buffered_stream_generator.h"

#include <algorithm>
#include <cmath>

namespace rb4 {

namespace {

constexpr std::uint32_t kActiveHandleBit = 0x80000000;
constexpr std::uint32_t kGenerationMask = 0x00003FFF;
constexpr std::uint32_t kPoolIndexMask = 0x00FFC000;
constexpr std::uint32_t kPoolIndexShift = 14;
constexpr std::int32_t kBufferedStreamFormat = 3;
constexpr float kPcm16ToFloat = 1.0F / 32768.0F;

}  // namespace

// Reconstructed from eboot.elf at 0x26D940. These float-rounded coefficients
// implement Olli Niemitalo's Optimal 32x, six-point, fifth-order z-form
// interpolator. The synchronized render path applies it to each channel.
float fmod_buffered_stream_interpolate_optimal32_6p5o(
    const std::array<float, 6>& samples,
    float fraction) {
    const float z = fraction - 0.5F;

    const float even1 = samples[3] + samples[2];
    const float odd1 = samples[3] - samples[2];
    const float even2 = samples[4] + samples[1];
    const float odd2 = samples[4] - samples[1];
    const float even3 = samples[5] + samples[0];
    const float odd3 = samples[5] - samples[0];

    const float c0 = even1 * 0.426859825850F +
                     even2 * 0.0723812356591F +
                     even3 * 0.000758930807933F;
    const float c1 = odd1 * 0.358317732811F +
                     odd2 * 0.204516440630F +
                     odd3 * 0.00562658812851F;
    const float c2 = even1 * -0.217009171844F +
                     even2 * 0.200513765216F +
                     even3 * 0.0164954103529F;
    const float c3 = odd1 * -0.251127153635F +
                     odd2 * 0.0422302596271F +
                     odd3 * 0.0248872749507F;
    const float c4 = even1 * 0.0416694656014F +
                     even2 * -0.0625042021275F +
                     even3 * 0.0208347346634F;
    const float c5 = odd1 * 0.0834979936481F +
                     odd2 * -0.0417491272092F +
                     odd3 * 0.00834987871349F;

    return ((((c5 * z + c4) * z + c3) * z + c2) * z + c1) * z + c0;
}

// The normal path in the render callback at 0x26C720 performs linear
// interpolation over interleaved signed 16-bit stereo frames.
FmodBufferedStereoSample fmod_buffered_stream_interpolate_pcm16_stereo(
    const std::int16_t* interleaved_samples,
    std::size_t frame_count,
    float frame_position,
    float gain) {
    if (interleaved_samples == nullptr || frame_count == 0) {
        return {};
    }

    const auto clamped_position = std::max(
        0.0F,
        std::min(frame_position, static_cast<float>(frame_count - 1)));
    const auto current_frame =
        static_cast<std::size_t>(std::floor(clamped_position));
    const auto next_frame = std::min(current_frame + 1, frame_count - 1);
    const auto fraction = clamped_position - current_frame;

    const auto* current = interleaved_samples + current_frame * 2;
    const auto* next = interleaved_samples + next_frame * 2;
    const auto scale = gain * kPcm16ToFloat;
    return {
        (current[0] + (next[0] - current[0]) * fraction) * scale,
        (current[1] + (next[1] - current[1]) * fraction) * scale,
    };
}

std::size_t fmod_buffered_stream_render_linear_pcm16_stereo(
    const std::int16_t* interleaved_samples,
    std::size_t source_frame_count,
    float& source_frame_position,
    float source_frames_per_output_frame,
    float gain,
    float* output_left,
    float* output_right,
    std::size_t output_frame_count) {
    if (interleaved_samples == nullptr || output_left == nullptr ||
        output_right == nullptr || source_frames_per_output_frame <= 0.0F) {
        return 0;
    }

    std::size_t frames_written = 0;
    while (frames_written < output_frame_count &&
           source_frame_position < source_frame_count) {
        const auto sample = fmod_buffered_stream_interpolate_pcm16_stereo(
            interleaved_samples,
            source_frame_count,
            source_frame_position,
            gain);
        output_left[frames_written] = sample.left;
        output_right[frames_written] = sample.right;
        source_frame_position += source_frames_per_output_frame;
        ++frames_written;
    }

    std::fill(
        output_left + frames_written,
        output_left + output_frame_count,
        0.0F);
    std::fill(
        output_right + frames_written,
        output_right + output_frame_count,
        0.0F);
    return frames_written;
}

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
    position_ms_ = 0.0F;
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
void FmodBufferedStreamGenerator::set_position_ms(float position) {
    position_ms_ = position;
}

// Reconstructed from eboot.elf at 0x26BDB0.
float FmodBufferedStreamGenerator::position_ms() const {
    return position_ms_;
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
