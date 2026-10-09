#include "render/video/BinkRenderMgr.h"

// Reconstructed from eboot.elf at 0x5F2B40.
void BinkRenderMgr::PrepareFrame(RndContext& context) {
    PrepareContext(context);
    for (auto* video : mConversions) {
        if (video != nullptr && video->mConversionPending) {
            video->ConvertFrame(context);
            video->mConversionPending = false;
        }
    }
}
