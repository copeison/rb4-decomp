#pragma once

class Entity;
class RndShaderCBuffer;

// A shader-graph node class (render/RndShaderNodeSceneGlobalColor.o). Only the
// scene globals it provides are declared: the scene drawer writes them into its
// scene constant buffer (RndSceneDrawer::_UpdateShaderGraphGlobals, 0x41C640).
class RndShaderNodeSceneGlobalColor {
public:
    // Writes the scene component's driven global colors into gSceneGlobalColors
    // and marks the buffer for a sync. Name not in the reference map.
    static void SetGlobalConstants(RndShaderCBuffer& cbuffer, Entity* entity);  // 0x581370
};
