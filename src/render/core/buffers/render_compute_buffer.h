#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderComputeBufferDescriptor {
    std::size_t element_stride;
    std::size_t element_count;
    const void* initial_data;
    void* external_gpu_data;
    std::uint32_t reserved;
    std::uint32_t flags;
    const char* name;
};

struct RenderComputeBuffer {
    void* implementation;
    std::int64_t frame_stamp;
    std::size_t element_stride;
    std::size_t element_count;
    const void* initial_data;
    void* external_gpu_data;
    std::uint32_t reserved;
    std::uint32_t flags;
    const char* name;
    void* staging_data;
    std::size_t staging_size;
};

static_assert(sizeof(RenderComputeBufferDescriptor) == 48);
static_assert(offsetof(RenderComputeBufferDescriptor, element_stride) == 0);
static_assert(offsetof(RenderComputeBufferDescriptor, element_count) == 8);
static_assert(sizeof(RenderComputeBuffer) == 80);
static_assert(offsetof(RenderComputeBuffer, element_stride) == 16);
static_assert(offsetof(RenderComputeBuffer, element_count) == 24);

RenderComputeBuffer* render_create_compute_buffer(
    const RenderComputeBufferDescriptor& descriptor);
void render_compute_buffer_construct(
    RenderComputeBuffer& buffer,
    const RenderComputeBufferDescriptor& descriptor);
void render_compute_buffer_destruct(RenderComputeBuffer& buffer);
void render_compute_buffer_delete(RenderComputeBuffer& buffer);
void render_compute_buffer_release_dynamic(RenderComputeBuffer& buffer);
void render_compute_buffer_initialize_backend(RenderComputeBuffer& buffer);
void render_delete_compute_buffer_storage(RenderComputeBuffer& buffer);
std::uint32_t render_compute_buffer_type(
    const RenderComputeBuffer& buffer);

}  // namespace rb4
