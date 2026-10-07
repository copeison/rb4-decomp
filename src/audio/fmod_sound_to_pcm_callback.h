#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "fmod_api.h"

namespace rb4 {

class FmodAudioStreamResource;

class FmodPcmDecodeConsumer {
public:
    virtual ~FmodPcmDecodeConsumer() = default;
    virtual std::uint32_t configure(
        std::uint32_t sample_rate,
        FMOD_SOUND_FORMAT format,
        std::uint32_t channels,
        std::uint32_t total_frames) = 0;
    virtual bool consume(const void* data, std::size_t byte_count) = 0;
    virtual void complete() = 0;
    virtual void cancel() = 0;
};

class FMODSoundToPCMCallback {
public:
    FMODSoundToPCMCallback(
        FmodAudioStreamResource& resource,
        FmodPcmDecodeConsumer& consumer,
        FMOD::Sound* sound);
    ~FMODSoundToPCMCallback();

    void run();
    void request_cancel();

private:
    FmodAudioStreamResource* resource_ = nullptr;
    FmodPcmDecodeConsumer* consumer_ = nullptr;
    FMOD::Sound* sound_ = nullptr;
    std::vector<std::byte> buffer_;
    std::atomic_bool cancel_requested_{false};
};

}  // namespace rb4
