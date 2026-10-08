#include "audio/fmod/resources/fmod_sound_to_pcm_callback.h"

#include <thread>

#include "audio/fmod/resources/fmod_audio_stream_resource.h"

namespace rb4 {

// Reconstructed from the object consumed by eboot.elf at 0x272F60.
FMODSoundToPCMCallback::FMODSoundToPCMCallback(
    FmodAudioStreamResource& resource,
    FmodPcmDecodeConsumer& consumer,
    FMOD::Sound* sound)
    : resource_(&resource), consumer_(&consumer), sound_(sound) {}

// Reconstructed from eboot.elf at 0x272E60.
FMODSoundToPCMCallback::~FMODSoundToPCMCallback() {
    if (sound_ != nullptr) {
        sound_->release();
        sound_ = nullptr;
    }
    buffer_.clear();
    consumer_ = nullptr;
    resource_ = nullptr;
}

void FMODSoundToPCMCallback::request_cancel() {
    cancel_requested_.store(true, std::memory_order_relaxed);
}

// Reconstructed from eboot.elf at 0x272F60.
void FMODSoundToPCMCallback::run() {
    if (sound_ == nullptr || consumer_ == nullptr) {
        return;
    }

    std::int32_t open_state = 0;
    std::uint32_t percent_buffered = 0;
    bool starving = false;
    bool disk_busy = false;
    do {
        sound_->getOpenState(
            &open_state, &percent_buffered, &starving, &disk_busy);
        if (open_state != 0 &&
            !cancel_requested_.load(std::memory_order_relaxed)) {
            std::this_thread::yield();
        }
    } while (open_state != 0 &&
             !cancel_requested_.load(std::memory_order_relaxed));

    if (cancel_requested_.load(std::memory_order_relaxed)) {
        consumer_->cancel();
        consumer_ = nullptr;
        return;
    }

    FMOD_SOUND_TYPE type{};
    FMOD_SOUND_FORMAT format{};
    std::int32_t channels = 0;
    std::int32_t bits = 0;
    float frequency = 0.0F;
    std::int32_t priority = 0;
    std::uint32_t total_frames = 0;
    sound_->getFormat(&type, &format, &channels, &bits);
    sound_->getDefaults(&frequency, &priority);
    sound_->getLength(&total_frames, FMOD_TIMEUNIT_PCM);

    const auto frames_per_block = consumer_->configure(
        static_cast<std::uint32_t>(frequency),
        FMOD_SOUND_FORMAT_PCM16,
        static_cast<std::uint32_t>(channels),
        total_frames);
    const auto bytes_per_frame =
        static_cast<std::uint32_t>(channels) * sizeof(std::int16_t);
    buffer_.resize(frames_per_block * bytes_per_frame);

    sound_->seekData(0);
    std::uint32_t frames_read = 0;
    while (frames_read < total_frames &&
           !cancel_requested_.load(std::memory_order_relaxed)) {
        std::uint32_t bytes_read = 0;
        sound_->readData(
            buffer_.data(),
            static_cast<std::uint32_t>(buffer_.size()),
            &bytes_read);
        if (bytes_read == 0) {
            break;
        }
        if (!consumer_->consume(buffer_.data(), bytes_read)) {
            if (resource_ != nullptr) {
                resource_->mark_decode_failed();
            }
            request_cancel();
            break;
        }
        frames_read += bytes_read / bytes_per_frame;
    }

    sound_->seekData(0);
    if (cancel_requested_.load(std::memory_order_relaxed)) {
        consumer_->cancel();
    } else {
        consumer_->complete();
    }
    consumer_ = nullptr;
}

}  // namespace rb4
