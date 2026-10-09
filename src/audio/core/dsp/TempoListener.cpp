#include "audio/core/dsp/TempoListener.h"

#include "audio/core/generators/AudioGenerator.h"

// This build's object for TempoListener (0x5B2A0 to 0x5B56C) is not in the
// reference map, so the file is named after the class. Its static
// initializer (0x5B4D0) builds sCritSec; it also sets the word at 0x19C87A0
// to -1, as many audio objects' initializers set a word of their own, which
// is not modelled.
CritSec TempoListener::sCritSec;

// Reconstructed from eboot.elf at 0x5B2A0.
TempoListener::TempoListener() : mOwner(nullptr) {}

// Reconstructed from eboot.elf at 0x5B2D0.
TempoListener::~TempoListener() {
    Unregister();
}

// Reconstructed from eboot.elf at 0x5B390.
void TempoListener::Unregister() {
    ScopedCritSec lock(sCritSec);
    if (mOwner != nullptr) {
        mOwner->UnregisterTempoListener(this);
    }
    mOwner = nullptr;
}

// Reconstructed from eboot.elf at 0x5B4C0.
CritSec* TempoListener::GetCritSec() {
    return &sCritSec;
}
