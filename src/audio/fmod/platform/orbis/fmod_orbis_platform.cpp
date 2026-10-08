#include "audio/fmod/platform/orbis/fmod_orbis_platform.h"

#include <kernel.h>

#include "audio/fmod/api/fmod_api.h"
#include "core/threading/thread_affinity.h"
#include "core/threading/thread_affinity_adapters.h"

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

std::uint32_t affinity_mask(const ThreadAffinityGroup& group) {
    return static_cast<std::uint32_t>(thread_affinity_build_cpu_mask(
        group.primary_processor,
        group.additional_processor_mask));
}

}  // namespace

// Reconstructed from eboot.elf at 0x261F60.
void fmod_load_modules_and_set_thread_affinity() {
    load_module("/app0/libfmod.prx");
    load_module("/app0/libfmodstudio.prx");

    const auto* audio_render = thread_affinity_find_group("audio_render");
    const auto* mic_reader = thread_affinity_find_group("mic_reader");
    const auto audio_render_mask = affinity_mask(*audio_render);

    FMOD_ORBIS_THREAD_AFFINITY affinity{};
    for (std::size_t index = 0; index < kFmodThreadCount; ++index) {
        affinity.masks[index] = audio_render_mask;
    }
    affinity.masks[kRecordingThreadIndex] = affinity_mask(*mic_reader);

    FMOD_Orbis_SetThreadAffinity(&affinity);
}

}  // namespace rb4
