#pragma once

class RndShaderCBuffer;

// A shader-graph node class (render/RndShaderNodeTime.o). Only the scene
// globals it provides are declared: the scene drawer writes them into its scene
// constant buffer (RndSceneDrawer::_UpdateShaderGraphGlobals, 0x41C640).
class RndShaderNodeTime {
public:
    // Writes the time, UI time and beat of the render clock and a value of the
    // sound manager's default 2D emitter into gTime and marks the buffer for a
    // sync. Name not in the reference map.
    static void SetGlobalConstants(RndShaderCBuffer& cbuffer);  // 0x5A52D0
};
