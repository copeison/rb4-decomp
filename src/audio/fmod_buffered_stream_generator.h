#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

#include "audio_clip_fmod.h"

namespace rb4 {

class FmodAudioBusGenerator;
class FmodBufferedStreamGeneratorManager;

using FmodBufferedStreamGeneratorHandle = std::uint32_t;

struct FmodBufferedStreamOptions {
    std::int32_t format = 0;
    bool streaming = false;
};

struct FmodBufferedStereoSample {
    float left = 0.0F;
    float right = 0.0F;
};

// The six samples are ordered y[-2], y[-1], y[0], y[1], y[2], y[3].
float fmod_buffered_stream_interpolate_optimal32_6p5o(
    const std::array<float, 6>& samples,
    float fraction);

FmodBufferedStereoSample fmod_buffered_stream_interpolate_pcm16_stereo(
    const std::int16_t* interleaved_samples,
    std::size_t frame_count,
    float frame_position,
    float gain);

std::size_t fmod_buffered_stream_render_linear_pcm16_stereo(
    const std::int16_t* interleaved_samples,
    std::size_t source_frame_count,
    float& source_frame_position,
    float source_frames_per_output_frame,
    float gain,
    float* output_left,
    float* output_right,
    std::size_t output_frame_count);

class FmodBufferedStreamGenerator {
public:
    void initialize_pool_slot(
        FmodBufferedStreamGeneratorManager& manager,
        std::uint32_t index);
    void pause();
    void resume();
    void prepare_for_audio_reset();
    void stop_and_wait();
    void set_position_ms(float position);
    float position_ms() const;
    void set_gain(float gain);
    float gain() const;

private:
    friend class FmodBufferedStreamGeneratorManager;

    FmodBufferedStreamGeneratorManager* manager_ = nullptr;
    std::uint32_t pool_index_ = 0;
    std::atomic<std::uint32_t> reference_count_{0};
    FmodBufferedStreamGeneratorHandle handle_ = 0;
    void* sound_source_ = nullptr;
    FmodAudioState* audio_state_ = nullptr;
    FmodAudioBusGenerator* bus_generator_ = nullptr;
    FMOD::Sound* sound_ = nullptr;
    float gain_ = 1.0F;
    float position_ms_ = 0.0F;
    bool sound_open_pending_ = false;
    bool in_free_list_ = false;
    AudioClipFmodState state_ = AudioClipFmodState::stopped;
};

class FmodBufferedStreamGeneratorManager {
public:
    FmodBufferedStreamGeneratorManager(
        std::size_t capacity,
        FmodAudioState* default_audio_state);

    std::int32_t setting() const;
    void set_setting(std::int32_t value);
    bool accepts(const FmodBufferedStreamOptions& options) const;

    void initialize_pool();
    bool shutdown_pool();

    FmodBufferedStreamGenerator* retain(
        FmodBufferedStreamGeneratorHandle handle,
        std::uint32_t index);
    FmodBufferedStreamGenerator* acquire(
        FmodAudioState* audio_state,
        void* sound_source);
    void release(FmodBufferedStreamGenerator& generator);

    void prepare_all_for_audio_reset();
    void stop_all();
    std::vector<FmodBufferedStreamGeneratorHandle> active_handles() const;

private:
    FmodBufferedStreamGeneratorHandle activate_handle(
        FmodBufferedStreamGenerator& generator) const;

    std::size_t capacity_ = 0;
    FmodAudioState* default_audio_state_ = nullptr;
    std::int32_t setting_ = 0;
    mutable std::recursive_mutex mutex_;
    std::uint32_t lock_depth_ = 0;
    std::unique_ptr<FmodBufferedStreamGenerator[]> generators_;
    std::deque<std::uint32_t> free_indices_;
};

}  // namespace rb4
