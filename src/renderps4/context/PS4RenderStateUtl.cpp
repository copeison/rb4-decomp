#include "renderps4/context/PS4RenderStateUtl.h"

namespace PS4RenderStateUtl {

// Reconstructed from eboot.elf at 0x8EC6A0, called by
// PS4Context::_SetSamplerImpl.
void InitSampler(GnmSampler& sampler, WrapMode wrap, unsigned int filter) {
    sampler = {
        {0, 0x00FFF000, 0x05000000, 0},
    };

    switch (wrap) {
    case WrapMode::kClamp:
        sampler.mRegisters[0] = 0x92;
        break;
    case WrapMode::kWrap:
        break;
    case WrapMode::kClampOpaqueBlack:
        sampler.mRegisters[0] = 0x1B6;
        sampler.mRegisters[3] = 0x40000000;
        break;
    case WrapMode::kClampOpaqueWhite:
        sampler.mRegisters[0] = 0x1B6;
        sampler.mRegisters[3] = 0x80000000;
        break;
    case WrapMode::kMirror:
        sampler.mRegisters[0] = 0x49;
        break;
    }

    switch (filter) {
    case 1:
        sampler.mRegisters[2] = 0x05000000;
        break;
    case 2:
        sampler.mRegisters[2] = 0x06500000;
        break;
    case 3:
        sampler.mRegisters[2] = 0x0A500000;
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        sampler.mRegisters[0] |= (filter - 3) << 9;
        sampler.mRegisters[2] = 0x0AF00000;
        break;
    default:
        break;
    }
}

}  // namespace PS4RenderStateUtl
