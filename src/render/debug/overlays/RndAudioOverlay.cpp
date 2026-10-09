#include "render/debug/overlays/RndAudioOverlay.h"

#include "audio/core/generators/AudioGenerator.h"

// Reconstructed from eboot.elf at 0x6E0E60.
RndAudioOverlay::RndAudioOverlay() : RndOverlayTextBase("audio", 0) {}

// Reconstructed from eboot.elf at 0x6E0EA0 (deleting variant at 0x6E0EB0).
RndAudioOverlay::~RndAudioOverlay() {}

// Reconstructed from eboot.elf at 0x6E0E90.
void RndAudioOverlay::_Print(TextStream& stream) {
    theSoundManager.DumpGeneratorStats(stream);
}
