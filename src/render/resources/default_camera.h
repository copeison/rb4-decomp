#pragma once

#include "render/core/render_runtime_adapters.h"

namespace rb4 {

struct DefaultCameraState {
    RndCameraCom* camera = nullptr;
};

void render_create_default_camera(DefaultCameraState& state, RndScene& scene);

}  // namespace rb4
