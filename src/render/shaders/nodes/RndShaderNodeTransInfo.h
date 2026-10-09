#pragma once

class Entity;
class RndShaderCBuffer;

// A shader-graph node class (render/RndShaderNodeTransInfo.o). Only the scene
// globals it provides are declared: the scene drawer writes them into its scene
// constant buffer (RndSceneDrawer::_UpdateShaderGraphGlobals, 0x41C640).
class RndShaderNodeTransInfo {
public:
    // Writes the transforms of the scene component's "trans_0" to "trans_3"
    // objects into gSGraphTransInfos and marks the buffer for a sync. Name not
    // in the reference map.
    static void SetGlobalConstants(RndShaderCBuffer& cbuffer, Entity* entity);  // 0x5A6980
};
