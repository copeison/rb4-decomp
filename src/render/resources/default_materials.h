#pragma once

#include "render/core/render_runtime_adapters.h"

namespace rb4 {

struct DefaultMaterialSet {
    RndMaterial* unlit = nullptr;
    RndMaterial* additive = nullptr;
    RndMaterial* lit = nullptr;
    RndMaterial* text = nullptr;
    RndMaterial* particle = nullptr;
    RndMaterial* decal = nullptr;
};

void render_create_default_materials(
    DefaultMaterialSet& materials,
    RndScene& scene);

}  // namespace rb4
