#include "renderps4/context/PS4RenderStateUtl.h"

namespace PS4RenderStateUtl {

// Reconstructed from the sampler setup in PS4Context::_SetSamplerImpl
// (eboot.elf at 0x8EA830).
void InitSampler(
    rb4::OrbisSamplerDescriptor& sampler,
    rb4::OrbisSamplerAddressMode wrap,
    unsigned int filter) {
    sampler = {
        {0, 0x00FFF000, 0x05000000, 0},
    };

    switch (wrap) {
    case rb4::OrbisSamplerAddressMode::kClamp:
        sampler.registers[0] = 0x92;
        break;
    case rb4::OrbisSamplerAddressMode::kWrap:
        break;
    case rb4::OrbisSamplerAddressMode::kClampOpaqueBlack:
        sampler.registers[0] = 0x1B6;
        sampler.registers[3] = 0x40000000;
        break;
    case rb4::OrbisSamplerAddressMode::kClampOpaqueWhite:
        sampler.registers[0] = 0x1B6;
        sampler.registers[3] = 0x80000000;
        break;
    case rb4::OrbisSamplerAddressMode::kMirror:
        sampler.registers[0] = 0x49;
        break;
    }

    switch (filter) {
    case 1:
        sampler.registers[2] = 0x05000000;
        break;
    case 2:
        sampler.registers[2] = 0x06500000;
        break;
    case 3:
        sampler.registers[2] = 0x0A500000;
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        sampler.registers[0] |= (filter - 3) << 9;
        sampler.registers[2] = 0x0AF00000;
        break;
    default:
        break;
    }
}

}  // namespace PS4RenderStateUtl
