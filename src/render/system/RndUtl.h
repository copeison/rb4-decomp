#pragma once

class Entity;

// Renderer helpers (render/RndUtl.o). Only the functions the reconstructed
// code calls are declared.
class RndUtl {
public:
    // The scene's level-of-detail mask and the distances of the three
    // levels: with RndConfig::mUseLod, the scene component's "enable_lod"
    // band (or its "isolate_lod" level alone), else the RndEntity
    // component's level; one level at the largest float otherwise. The
    // map's signature takes an EntityPtr const&.
    static void ExtractLodSettings(Entity* entity, unsigned int& lodMask, float* distances);  // 0x4423C0
};
