#pragma once

#include <cstdint>

#include "render/core/targets/render_target.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void render_target_set_base_dispatch(RenderTarget& target);
void render_target_state_construct(
    RenderTargetState* state,
    std::uint32_t state_flags,
    RenderTargetState* source_state);
void render_target_state_delete(RenderTargetState* state);
void render_delete_target_storage(RenderTarget& target);

}  // namespace rb4
