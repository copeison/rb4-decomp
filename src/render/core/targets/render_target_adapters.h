#pragma once

#include <cstdint>

#include "render/core/targets/render_target.h"

namespace rb4 {

void render_target_set_base_dispatch(RenderTarget& target);
void render_delete_target_storage(RenderTarget& target);

}  // namespace rb4
