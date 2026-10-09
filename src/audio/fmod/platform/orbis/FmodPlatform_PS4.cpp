#include "audio/fmod/platform/orbis/FmodPlatform_PS4.h"

#include <cstdio>
#include <kernel.h>

#include "audio/fmod/api/fmod_api.h"
#include "audio/fmod/system/FmodPlatform.h"
#include "utl/threading/Thread.h"

namespace {

// FMOD's PS4 runtime runs eleven threads; index 6 is its recording thread.
constexpr int kFmodThreadCount = 11;
constexpr int kFmodRecordThread = 6;

void LoadModule(const char* name) {
    char path[64];
    std::snprintf(path, sizeof(path), "/app0/%s.prx", name);
    int result = 0;
    sceKernelLoadStartModule(path, 0, nullptr, 0, nullptr, &result);
}

unsigned int TaskAffinity(const char* taskName) {
    const ThreadMap::TaskDesc& task = *ThreadMap::GetTaskSettings(taskName);
    return static_cast<unsigned int>(
        ThreadMap::BuildAffinityMask(task.mProcessor, task.mAffinityMask));
}

}  // namespace

// Reconstructed from eboot.elf at 0x261F60. Every FMOD thread follows the
// "audio_render" task except the recording thread, which follows
// "mic_reader".
void FmodLoadModules() {
    LoadModule("libfmod");
    LoadModule("libfmodstudio");

    const unsigned int renderMask = TaskAffinity("audio_render");
    const unsigned int micMask = TaskAffinity("mic_reader");
    FMOD_ORBIS_THREAD_AFFINITY affinity;
    for (int index = 0; index < kFmodThreadCount; ++index) {
        affinity.masks[index] = renderMask;
    }
    affinity.masks[kFmodRecordThread] = micMask;
    FMOD_Orbis_SetThreadAffinity(&affinity);
}

// Reconstructed from eboot.elf at 0x262300.
void FmodSetListenerXfm(bool fmodEnabled, const Transform& xfm) {
    FModSystem* system = FModSystem::Get();
    if (!fmodEnabled || system == nullptr || system->mStudioSystem == nullptr) {
        return;
    }
    FMOD_3D_ATTRIBUTES attributes;
    Convert(xfm, attributes);
    system->mStudioSystem->setListenerAttributes(0, &attributes);
}
