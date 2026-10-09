#pragma once

#include <cstddef>

class RndShaderFogDeferred;

// The atmosphere's device-wide resources (render/RndAtmosphereGlobals.o,
// 0x451C70-0x451CEF): the deferred fog shader that RndFogCom draws with.
// RndDevice embeds it after the lighting globals.
class RndAtmosphereGlobals {
public:
    RndAtmosphereGlobals();   // 0x451C70
    ~RndAtmosphereGlobals();  // 0x451C80, empty

    // Creates and registers the fog shader. RndDevice::Init calls it.
    void Init();  // 0x451C90
    // Deletes the fog shader. RndDevice::Terminate calls it.
    void Terminate();  // 0x451CC0

    // Field name not in the reference map.
    RndShaderFogDeferred* mFogDeferred;
};

static_assert(offsetof(RndAtmosphereGlobals, mFogDeferred) == 0);
static_assert(sizeof(RndAtmosphereGlobals) == 8);
