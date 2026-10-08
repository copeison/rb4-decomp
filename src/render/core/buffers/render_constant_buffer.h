#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

namespace rb4 {

struct RenderConstantBuffer;

struct RenderConstantBufferDispatch {
    void (*destruct)(RenderConstantBuffer& buffer);
    void (*delete_buffer)(RenderConstantBuffer& buffer);
    void (*initialize_backend)(RenderConstantBuffer& buffer);
};

enum RenderConstantBufferStageMask : std::uint32_t {
    kConstantBufferVertexStage = 1U << 0,
    kConstantBufferTessellationStages = 1U << 1,
    kConstantBufferGeometryStage = 1U << 2,
    kConstantBufferPixelStage = 1U << 3,
    kConstantBufferComputeStage = 1U << 4,
};

struct RenderConstantBufferDescriptor {
    void* owner;
    std::uint32_t slot;
    std::uint32_t stage_mask;
    std::uint8_t reserved[8];
    std::size_t element_count;
};

struct RenderConstantBuffer {
    const RenderConstantBufferDispatch* dispatch;
    void* owner;
    std::uint32_t flags;
    std::uint32_t slot;
    std::uint32_t stage_mask;
    std::uint32_t reserved;
    std::size_t descriptor_element_count;
    std::size_t element_count;
    void* data;
    bool upload_pending;
    std::uint8_t trailing_reserved[7];
};

static_assert(sizeof(RenderConstantBufferDescriptor) == 32);
static_assert(sizeof(RenderConstantBuffer) == 64);

constexpr std::size_t kUseConstantBufferDescriptorCount =
    std::numeric_limits<std::size_t>::max();

RenderConstantBuffer* render_create_constant_buffer(
    const RenderConstantBufferDescriptor& descriptor,
    std::uint32_t flags,
    std::size_t element_count = kUseConstantBufferDescriptorCount);
void render_constant_buffer_construct(
    RenderConstantBuffer& buffer,
    const RenderConstantBufferDescriptor& descriptor,
    std::uint32_t flags,
    std::size_t element_count,
    void* data);
void render_constant_buffer_destruct(RenderConstantBuffer& buffer);
void render_constant_buffer_delete(RenderConstantBuffer& buffer);
void render_delete_constant_buffer_storage(RenderConstantBuffer& buffer);
void render_constant_buffer_initialize_backend(RenderConstantBuffer& buffer);
void render_constant_buffer_release_dynamic(RenderConstantBuffer& buffer);

}  // namespace rb4
