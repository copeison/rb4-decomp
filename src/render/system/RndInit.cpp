#include "render/system/RndInit.h"

#include "render/core/platform/render_platform.h"
#include "render/system/RndDevice.h"

// The callees below are not identified yet. Names not in the reference map.
using RndExitCallback = void (*)();

void RndLightingPostInit(RndDevice& device, const RndInitParams& params);
void RndDependentsInit(const RndInitParams& params);
void RndAddExitCallback(RndExitCallback callback);

bool RndTerminating();
void RndSetTerminating(bool terminating);
void RndDependentsTerminate();

// Reconstructed from eboot.elf at 0x402C30.
void Rnd::Init(const RndInitParams& params) {
    RndDevice& device = *PlatformCreateDevice();

    (void)rb4::orbis_render_api();
    (void)rb4::render_api_for_platform(rb4::RenderPlatform::kPlayStation4);
    device.Init(params);
    device.mDefaults.Init(params.mInitRendering);
    RndLightingPostInit(device, params);
    RndDependentsInit(params);
    RndAddExitCallback(Terminate);
}

// Reconstructed from eboot.elf at 0x402D30.
void Rnd::Terminate() {
    RndDevice* device = TheRndDevice();
    if (device == nullptr || RndTerminating()) {
        return;
    }

    RndSetTerminating(true);
    RndDependentsTerminate();
    device->Terminate();
    delete device;
    gRndDevice = nullptr;
    RndSetTerminating(false);
}
