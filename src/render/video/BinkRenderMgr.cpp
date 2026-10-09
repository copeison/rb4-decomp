#include "render/video/BinkRenderMgr.h"

#include "render/context/RndContext.h"

// Reconstructed from eboot.elf at 0x5F2B40. The conversions draw without a
// camera.
void BinkRenderMgr::PrepareFrame(RndContext& context) {
    context.SetCamera(nullptr);
    for (auto* video : mConversions) {
        if (video != nullptr && video->mConversionPending) {
            video->ConvertFrame(context);
            video->mConversionPending = false;
        }
    }
}
