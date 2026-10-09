#include "mic/core/MicHwPlatform.h"

#include "mic/core/MicHwManager.h"

// Reconstructed from eboot.elf at 0xA2D70.
void MicHwPlatform::Init(int numMics) {
    if (!mInitialized) {
        mInitialized = true;
        _SetupMicArray(numMics);
    }
}

// Reconstructed from eboot.elf at 0xA2D90.
void MicHwPlatform::Poll() {
    _Poll();
}

// Reconstructed from eboot.elf at 0xA2DA0.
void MicHwPlatform::Terminate() {
    if (mInitialized) {
        ClearBusPaths();
        mInitialized = false;
    }
}

// Reconstructed from eboot.elf at 0xA2DD0.
void MicHwPlatform::CheckConnectsAndDisconnects() {
    _CheckConnectsAndDisconnects();
}

// Reconstructed from eboot.elf at 0xA2DE0.
void MicHwPlatform::MarkMicsChanged() {
    gMicHwManager.MarkMicsChanged();
}
