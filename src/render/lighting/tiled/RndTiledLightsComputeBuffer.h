#pragma once

#include <cstddef>

class RndComputeBuffer;

// One kind of light uploaded for the tiled-lighting shaders: the buffer and
// the number of lights that add and that subtract light. The tiled shaders
// read an array of them for point, spot and directional lights. The name
// comes from the map's RndVolumtericScatteringCom::BeginAsyncUpdate
// signature; field names are not in the reference map.
struct RndTiledLightsComputeBuffer {
    RndComputeBuffer* mBuffer;
    unsigned long mNumPosLights;
    unsigned long mNumNegLights;
};

static_assert(sizeof(RndTiledLightsComputeBuffer) == 24);

// Indices into the array of light buffers. Names not in the reference map.
enum RndTiledLightsBufferType : unsigned long {
    kTiledLightsPoint = 0,
    kTiledLightsSpot = 1,
    kTiledLightsDirectional = 2,
    kNumTiledLightsBufferTypes = 3,
};
