#pragma once

class RndDevice;
struct RndInitParams;

// Render subsystem startup and shutdown (RndInit.o).
namespace Rnd {
// Creates the device, initializes it and the render-dependent subsystems, and
// registers Terminate as an exit callback. The map's signature is Init(); this
// build passes the startup params to Init. The map's PostInit(RndInitParams*)
// is not located in this build, so its work is assumed folded into Init.
void Init(const RndInitParams& params);  // 0x402C30
// Shuts down the render-dependent subsystems and destroys the device.
void Terminate();                        // 0x402D30
// Creates the platform device. Defined by each platform's Init object.
RndDevice* PlatformCreateDevice();
}  // namespace Rnd
