#include "render/debug/overlays/RndCheatsOverlay.h"

#include "os/debug/Cheats.h"

// Reconstructed from eboot.elf at 0x6E0EF0.
RndCheatsOverlay::RndCheatsOverlay() : RndOverlayTextBase("cheats", kFlagStripedLines) {
    SetShowing(true);
}

// Reconstructed from eboot.elf at 0x6E0F30 (deleting variant at 0x6E0F40).
RndCheatsOverlay::~RndCheatsOverlay() {}

// Reconstructed from eboot.elf at 0x6E0F60. Takes no room without a
// message.
int RndCheatsOverlay::Draw(RndContext& context, int y) {
    if (theCheatsManager != nullptr && theCheatsManager->mMessage.c_str()[0] != '\0') {
        return RndOverlayTextBase::Draw(context, y);
    }
    return y;
}

// Reconstructed from eboot.elf at 0x6E0F90.
void RndCheatsOverlay::_Print(TextStream& stream) {
    const char* message = theCheatsManager->mMessage.c_str();
    if (message[0] != '\0') {
        stream.Print(message);
    }
}
