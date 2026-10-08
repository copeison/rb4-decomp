#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderComputeBuffer;
struct RenderComputeBufferDescriptor;
struct RenderConstantBuffer;
struct RenderConstantBufferDescriptor;
struct RenderMesh;
struct RenderOcclusionQuery;
struct RenderParticleBuffer;
struct RenderShader;
struct RenderTexture2D;
struct RenderTexture2DDescriptor;
struct RenderTexture3D;
struct RenderTexture3DDescriptor;
struct RenderTextureArray2D;
struct RenderTextureArray2DDescriptor;

enum class RenderShaderStage : std::uint32_t;
enum class RenderMeshFormat : std::uint32_t;

struct RenderFactory {
    const void* vtable;
};

static_assert(sizeof(RenderFactory) == 8);

RenderMesh* render_factory_create_mesh(
    RenderFactory& factory,
    RenderMeshFormat format,
    const char* name);
RenderTexture2D* render_factory_create_texture_2d(
    RenderFactory& factory,
    const RenderTexture2DDescriptor& descriptor);
RenderTexture3D* render_factory_create_texture_3d(
    RenderFactory& factory,
    const RenderTexture3DDescriptor& descriptor);
RenderTextureArray2D* render_factory_create_texture_array_2d(
    RenderFactory& factory,
    const RenderTextureArray2DDescriptor& descriptor);
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
