#include "audio/fmod/platform/orbis/fmod_orbis_platform.h"

#include <kernel.h>

#include "audio/fmod/api/fmod_api.h"
#include "utl/threading/Thread.h"

namespace rb4 {

namespace {

constexpr std::size_t kFmodThreadCount = 11;
constexpr std::size_t kRecordingThreadIndex = 6;

void load_module(const char* path) {
    int start_result = 0;
    sceKernelLoadStartModule(
        path,
        0,
        nullptr,
        0,
        nullptr,
        &start_result);
}

std::uint32_t affinity_mask(const ThreadMap::TaskDesc& group) {
    return static_cast<std::uint32_t>(ThreadMap::BuildAffinityMask(
        group.mProcessor, group.mAffinityMask));
}

}  // namespace

// Reconstructed from eboot.elf at 0x261F60.
void fmod_load_modules_and_set_thread_affinity() {
    load_module("/app0/libfmod.prx");
    load_module("/app0/libfmodstudio.prx");

    const auto* audio_render = ThreadMap::GetTaskSettings("audio_render");
    const auto* mic_reader = ThreadMap::GetTaskSettings("mic_reader");
    const auto audio_render_mask = affinity_mask(*audio_render);

    FMOD_ORBIS_THREAD_AFFINITY affinity{};
    for (std::size_t index = 0; index < kFmodThreadCount; ++index) {
        affinity.masks[index] = audio_render_mask;
    }
    affinity.masks[kRecordingThreadIndex] = affinity_mask(*mic_reader);

    FMOD_Orbis_SetThreadAffinity(&affinity);
}

}  // namespace rb4
