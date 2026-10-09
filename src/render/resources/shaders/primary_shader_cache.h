#pragma once

#include <cstdint>

namespace rb4 {

struct RenderPrimaryShaderResource;

std::uint32_t render_primary_shader_layout_hash(
    RenderPrimaryShaderResource& shader);

}  // namespace rb4
