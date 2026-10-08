#pragma once

#include <cstddef>
#include <cstdint>

#include "render/core/buffers/render_constant_buffer.h"

namespace rb4 {

void render_constant_buffer_set_base_dispatch(RenderConstantBuffer& buffer);
void render_delete_constant_buffer_storage(RenderConstantBuffer& buffer);

}  // namespace rb4
