#pragma once

class Entity;
class RndShaderCBuffer;

// A shader-graph node class (render/RndShaderNodeSceneGlobalFloat.o). Only the
// scene globals it provides are declared: the scene drawer writes them into its
// scene constant buffer (RndSceneDrawer::_UpdateShaderGraphGlobals, 0x41C640).
class RndShaderNodeSceneGlobalFloat {
public:
    // Writes the scene component's driven global floats into gSceneGlobalFloats
    // and marks the buffer for a sync. Name not in the reference map.
    static void SetGlobalConstants(RndShaderCBuffer& cbuffer, Entity* entity);  // 0x583220
};
