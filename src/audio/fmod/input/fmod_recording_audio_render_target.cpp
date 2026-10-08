#include "audio/fmod/input/fmod_recording_audio_render_target.h"

#include <utility>

#include "audio/core/audio_runtime_adapters.h"
#include "audio/fmod/mixing/fmod_mix_callback.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x275E20. The original constructor also
// builds the generic file-recording base and initializes the complete FMOD
// state before installing this buffered-output callback.
FmodRecordingAudioRenderTarget::FmodRecordingAudioRenderTarget(
    FmodAudioState& audio_state,
    float* mix_buffer,
    std::uint32_t frames_per_buffer)
    : audio_state_(audio_state),
      mix_buffer_(mix_buffer),
      frames_per_buffer_(frames_per_buffer) {
    audio_state_.buffered_output_update_callback =
        [this](FMOD_OUTPUT_STATE& output_state) {
            return read_mixer_output(output_state);
        };
}

// Reconstructed from eboot.elf at 0x275FB0.
FmodRecordingAudioRenderTarget::~FmodRecordingAudioRenderTarget() {
    request_stop();
    wait_for_recording();
    audio_state_.buffered_output_update_callback = {};
}

// Reconstructed from eboot.elf at 0x276080 and 0x276110.
void FmodRecordingAudioRenderTarget::start_async_recording(
    RecordingLoop recording_loop) {
    request_stop();
    wait_for_recording();
    stop_requested_.store(false, std::memory_order_release);
    recording_thread_ = std::thread(
        [this, worker = std::move(recording_loop)]() mutable {
            worker(stop_requested_);
        });
}

void FmodRecordingAudioRenderTarget::request_stop() {
    stop_requested_.store(true, std::memory_order_release);
}

// Reconstructed from eboot.elf at 0x276120.
void FmodRecordingAudioRenderTarget::wait_for_recording() {
    if (recording_thread_.joinable()) {
        recording_thread_.join();
    }
}

// Reconstructed from eboot.elf at 0x276130.
FMOD_RESULT FmodRecordingAudioRenderTarget::suspend_mixer() {
    return audio_state_.core_system->mixerSuspend();
}

// Reconstructed from eboot.elf at 0x276140.
FMOD_RESULT FmodRecordingAudioRenderTarget::resume_mixer() {
    return audio_state_.core_system->mixerResume();
}

// Reconstructed from eboot.elf at 0x276200.
FMOD_RESULT FmodRecordingAudioRenderTarget::update() {
    if (audio_state_.studio_system == nullptr) {
        return FMOD_OK;
    }
    return audio_state_.studio_system->update();
}

// Reconstructed from eboot.elf at 0x276210.
void FmodRecordingAudioRenderTarget::register_mix_consumer(void* consumer) {
    rb4::audio_register_mix_consumer(&audio_state_, consumer);
}

// Reconstructed from eboot.elf at 0x276230.
void FmodRecordingAudioRenderTarget::unregister_mix_consumer(void* consumer) {
    rb4::audio_unregister_mix_consumer(&audio_state_, consumer);
}

// Reconstructed from eboot.elf at 0x276250.
void FmodRecordingAudioRenderTarget::dispatch_mix_buffers() {
    lock();
    fmod_audio_dispatch_mix_buffers(audio_state_, audio_state_.mix_sequence);
    unlock();
}

// Reconstructed from eboot.elf at 0x2761A0.
void FmodRecordingAudioRenderTarget::lock() {
    audio_mutex_.lock();
    target_mutex_.lock();
}

// Reconstructed from eboot.elf at 0x2761D0.
void FmodRecordingAudioRenderTarget::unlock() {
    target_mutex_.unlock();
    audio_mutex_.unlock();
}

// Reconstructed from eboot.elf at 0x276170.
void* FmodRecordingAudioRenderTarget::voice_pool() const {
    return audio_state_.voice_pool;
}

// Reconstructed from eboot.elf at 0x276290.
AudioOutputDispatcher&
FmodRecordingAudioRenderTarget::output_dispatcher() {
    return audio_state_.output_block_dispatcher;
}

// Reconstructed from eboot.elf at 0x2762B0.
FmodAudioState& FmodRecordingAudioRenderTarget::audio_state() {
    return audio_state_;
}

// Reconstructed from the bound callback at 0x276340. FMOD asks the output
// plugin to copy one buffer of floating-point mixer output into the recorder.
FMOD_RESULT FmodRecordingAudioRenderTarget::read_mixer_output(
    FMOD_OUTPUT_STATE& output_state) {
    return output_state.readfrommixer(
        &output_state, mix_buffer_, frames_per_buffer_);
}

}  // namespace rb4
