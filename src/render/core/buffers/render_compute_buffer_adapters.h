#pragma once

#include <cstddef>

#include "render/core/buffers/render_compute_buffer.h"

namespace rb4 {

void render_compute_buffer_set_base_dispatch(RenderComputeBuffer& buffer);
void* render_allocate_compute_buffer_staging(std::size_t size);
void render_free_compute_buffer_staging(void* allocation);
void render_compute_buffer_initialize_backend(RenderComputeBuffer& buffer);
void render_delete_compute_buffer_storage(RenderComputeBuffer& buffer);
void render_compute_buffer_release_dynamic(RenderComputeBuffer& buffer);

}  // namespace rb4
