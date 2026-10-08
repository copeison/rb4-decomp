#pragma once

#include <cstdint>

#include "render/core/render_target.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void render_target_set_base_dispatch(RenderTarget& target);
void render_target_state_construct(
    void* state,
    std::uint32_t state_flags,
    void* source_state);
void render_target_state_delete(void* state);
void render_delete_target_storage(RenderTarget& target);

}  // namespace rb4
