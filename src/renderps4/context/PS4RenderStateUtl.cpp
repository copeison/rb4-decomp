#include "renderps4/context/PS4RenderStateUtl.h"

namespace PS4RenderStateUtl {

// Reconstructed from eboot.elf at 0x8EC6A0, called by
// PS4Context::_SetSamplerImpl.
void InitSampler(sce::Gnm::Sampler& sampler, WrapMode wrap, unsigned int filter) {
    sampler = {
        {0, 0x00FFF000, 0x05000000, 0},
    };

    switch (wrap) {
    case WrapMode::kClamp:
        sampler.m_regs[0] = 0x92;
        break;
    case WrapMode::kWrap:
        break;
    case WrapMode::kClampOpaqueBlack:
        sampler.m_regs[0] = 0x1B6;
        sampler.m_regs[3] = 0x40000000;
        break;
    case WrapMode::kClampOpaqueWhite:
        sampler.m_regs[0] = 0x1B6;
        sampler.m_regs[3] = 0x80000000;
        break;
    case WrapMode::kMirror:
        sampler.m_regs[0] = 0x49;
        break;
    }

    switch (filter) {
    case 1:
        sampler.m_regs[2] = 0x05000000;
        break;
    case 2:
        sampler.m_regs[2] = 0x06500000;
        break;
    case 3:
        sampler.m_regs[2] = 0x0A500000;
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        sampler.m_regs[0] |= (filter - 3) << 9;
        sampler.m_regs[2] = 0x0AF00000;
        break;
    default:
        break;
    }
}

}  // namespace PS4RenderStateUtl
