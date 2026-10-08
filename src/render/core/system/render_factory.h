#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderComputeBuffer;
struct RenderComputeBufferDescriptor;
struct RenderConstantBuffer;
struct RenderConstantBufferDescriptor;
struct RenderOcclusionQuery;
struct RenderParticleBuffer;
struct RenderShader;

enum class RenderShaderStage : std::uint32_t;

struct RenderFactory {
    const void* vtable;
};

static_assert(sizeof(RenderFactory) == 8);

RenderConstantBuffer* render_factory_create_constant_buffer(
    RenderFactory& factory,
    const RenderConstantBufferDescriptor& descriptor,
    std::uint32_t flags,
    std::size_t element_count);
RenderShader* render_factory_create_shader(
    RenderFactory& factory,
    RenderShaderStage stage);
RenderComputeBuffer* render_factory_create_compute_buffer(
    RenderFactory& factory,
    const RenderComputeBufferDescriptor& descriptor);
RenderParticleBuffer* render_factory_create_particle_buffer(
    RenderFactory& factory,
    std::uint32_t particle_count,
    void* context);
RenderOcclusionQuery* render_factory_create_occlusion_query(
    RenderFactory& factory,
    void* owner);

}  // namespace rb4
