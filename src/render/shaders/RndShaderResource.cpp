#include "render/shaders/RndShaderResource.h"

#include "render/context/RndContext.h"
#include "render/system/RndDevice.h"

// Reconstructed from eboot.elf at 0x505C20, the out-of-line copy; most
// draws inline it.
void RndShaderResource::Select(
    RndContext& context,
    RndShaderProgramType type,
    unsigned long slot,
    unsigned int flags,
    unsigned long extra) {
    mFrameStamp = static_cast<long>(TheRndDevice()->mFrameCount);
    auto& limit = (flags & kSelectReadWrite) != 0
        ? context.mOutputSlotLimits[type]
        : context.mInputSlotLimits[type];
    if (limit < slot + 1) {
        limit = slot + 1;
    }
    switch (type) {
    case kShaderProgramVertex:
        _SelectForVSImpl(context, slot, flags);
        break;
    case kShaderProgramHull:
        _SelectForHSImpl(context, slot, flags);
        break;
    case kShaderProgramDomain:
        _SelectForDSImpl(context, slot, flags);
        break;
    case kShaderProgramGeometry:
        _SelectForGSImpl(context, slot, flags);
        break;
    case kShaderProgramPixel:
        _SelectForPSImpl(context, slot, flags);
        break;
    case kShaderProgramCompute:
        _SelectForCSImpl(context, slot, flags, extra);
        break;
    }
}
