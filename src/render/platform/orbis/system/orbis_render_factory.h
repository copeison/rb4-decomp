#pragma once

#include "render/core/system/render_factory.h"

namespace rb4 {

struct OrbisRenderFactory : RenderFactory {};

static_assert(sizeof(OrbisRenderFactory) == 8);

OrbisRenderFactory* orbis_render_factory_create();

}  // namespace rb4
