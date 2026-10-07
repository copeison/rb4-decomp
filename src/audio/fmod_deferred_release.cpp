#include "fmod_deferred_release.h"

#include <cstddef>

#include "fmod_audio_system.h"

namespace rb4 {

FmodDeferredReleaseQueue::FmodDeferredReleaseQueue() {
    constexpr std::size_t initial_capacity = 64;
    for (auto& buffer : buffers) {
        buffer.reserve(initial_capacity);
    }
}

// Reconstructed from eboot.elf at 0x278A00.
void fmod_defer_channel_dsp_release(
    FmodAudioState& state,
    FMOD::ChannelControl* channel_control,
    FMOD::DSP* dsp) {
    auto& releases = state.deferred_releases;
    std::lock_guard<std::recursive_mutex> lock(releases.mutex);
    ++releases.dispatch_depth;
    if (state.studio_system != nullptr) {
        releases.buffers[releases.active_buffer].push_back({
            channel_control,
            dsp,
        });
    }
    --releases.dispatch_depth;
}

// Reconstructed from eboot.elf at 0x278DF0.
void fmod_process_deferred_releases(FmodAudioState& state) {
    auto& releases = state.deferred_releases;
    std::int32_t processing_buffer = 0;
    {
        std::lock_guard<std::recursive_mutex> lock(releases.mutex);
        processing_buffer = releases.active_buffer;
        releases.active_buffer = (releases.active_buffer & 1) == 0;
    }

    auto& buffer = releases.buffers[processing_buffer];
    for (const auto& release : buffer) {
        release.channel_control->stop();
        release.dsp->release();
    }
    buffer.clear();
}

void fmod_clear_deferred_releases(FmodAudioState& state) {
    auto& releases = state.deferred_releases;
    std::lock_guard<std::recursive_mutex> lock(releases.mutex);
    for (auto& buffer : releases.buffers) {
        buffer.clear();
    }
}

}  // namespace rb4
