#include "render/debug/overlays/RndMemOverlay.h"

#include "audio/fmod/platform/orbis/FmodPlatform_PS4.h"
#include "os/memory/MemMgr.h"
#include "render/system/RndDevice.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"

// Reconstructed from eboot.elf at 0x6E54A0.
RndMemOverlay::RndMemOverlay() : RndOverlayTextBase("mem", kFlagStripedLines) {
    mDisplayName = "memory";
}

// Reconstructed from eboot.elf at 0x6E5520 (deleting variant at 0x6E5530).
RndMemOverlay::~RndMemOverlay() {}

// Reconstructed from eboot.elf at 0x6E5550. The video memory counts are
// shown in thousands of bytes and the FMOD figures in millions.
void RndMemOverlay::_Print(TextStream& stream) {
    StackString<2048> overview;
    MemPrintOverview(-3, overview);
    stream.Print(overview.c_str());

    if (TheRndDevice() != nullptr) {
        const RndDeviceMemoryUsage usage = TheRndDevice()->_GetMemoryUsageImpl();
        if (usage.mValid) {
            char local[32] = {};
            char nonlocal[32] = {};
            stream << " [    VRAM] " << "local used: "
                   << PrintIntWithCommas(usage.mLocalUsed / 1000, local, sizeof(local))
                   << " KB " << "nonlocal used: "
                   << PrintIntWithCommas(usage.mNonlocalUsed / 1000, nonlocal, sizeof(nonlocal))
                   << " KB" << "\n";
        }
    }

    if (gFmodPlatformInterface != nullptr) {
        unsigned long current;
        unsigned long highest;
        gFmodPlatformInterface->GetMemoryStats(&current, &highest);
        const unsigned long heapSize = MemHeapSize(static_cast<unsigned long>(MemFindHeap("fmod")));
        const float currentMB = static_cast<float>(current) * 1e-6F;
        const float heapMB = static_cast<float>(heapSize) * 1e-6F;
        const float highestMB = static_cast<float>(highest) * 1e-6F;
        // The text outlives the formatter.
        const char* text;
        {
            FormatString format("FMOD memory usage: %6.1f/%6.1fMB  (highest: %6.1fMB)\n");
            format << currentMB << heapMB << highestMB;
            text = format.Str();
        }
        stream << text;
    }
}
