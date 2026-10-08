#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>

#include "audio/fmod/system/fmod_audio_system.h"

namespace rb4 {

// Clean semantic view of the FMOD-backed recording target. The executable
// embeds FmodAudioState at offset 0x1E0 in its concrete target object.
class FmodRecordingAudioRenderTarget {
public:
    FmodRecordingAudioRenderTarget(
        FmodAudioState& audio_state,
        float* mix_buffer,
        std::uint32_t frames_per_buffer);
    ~FmodRecordingAudioRenderTarget();

    using RecordingLoop =
        std::function<void(const std::atomic_bool& stop_requested)>;

    void start_async_recording(RecordingLoop recording_loop);
    void request_stop();
    void wait_for_recording();

    FMOD_RESULT suspend_mixer();
    FMOD_RESULT resume_mixer();
    FMOD_RESULT update();

    void register_mix_consumer(void* consumer);
    void unregister_mix_consumer(void* consumer);
    void dispatch_mix_buffers();

    void lock();
    void unlock();

    void* voice_pool() const;
    AudioOutputDispatcher& output_dispatcher();
    FmodAudioState& audio_state();

private:
    FMOD_RESULT read_mixer_output(FMOD_OUTPUT_STATE& output_state);

    FmodAudioState& audio_state_;
    float* mix_buffer_ = nullptr;
    std::uint32_t frames_per_buffer_ = 0;
    std::thread recording_thread_;
    std::atomic_bool stop_requested_{false};
    std::recursive_mutex target_mutex_;
    std::recursive_mutex audio_mutex_;
};

}  // namespace rb4
