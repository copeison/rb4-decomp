#include "render/system/RndInit.h"
#include "renderps4/system/PS4Device.h"

// Reconstructed from eboot.elf at 0x8D5DE0.
HxGfxApi Rnd::PlatformGfxApi() {
    return kGfxApiPS4;
}

// Reconstructed from eboot.elf at 0x8D5DF0.
RndDevice* Rnd::PlatformCreateDevice() {
    return new PS4Device;
}
