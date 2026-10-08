#pragma once

#include "render/core/textures/render_texture.h"

namespace rb4 {

void render_texture_set_base_dispatch(RenderTexture& texture);
void render_delete_texture_storage(RenderTexture& texture);
void render_texture_release_dynamic(RenderTexture& texture);
void render_texture_resolve_descriptor_fields(
    void* resolved_fields,
    std::int32_t descriptor_type,
    const void* creation_fields,
    std::int64_t fallback_mode);
bool render_texture_backend_initialization_disabled();

}  // namespace rb4
