#include "render/core/system/render_factory.h"

namespace rb4 {

namespace {

struct RenderFactoryDispatch {
    std::uint8_t reserved_0[3 * sizeof(void*)];
    RenderMesh* (*create_mesh)(
        RenderFactory& factory,
        RenderMeshFormat format,
        const char* name);
    std::uint8_t reserved_32[7 * sizeof(void*)];
    RenderConstantBuffer* (*create_constant_buffer)(
        RenderFactory& factory,
        const RenderConstantBufferDescriptor& descriptor,
        std::uint32_t flags,
        std::size_t element_count);
    RenderShader* (*create_shader)(
        RenderFactory& factory,
        RenderShaderStage stage);
    RenderComputeBuffer* (*create_compute_buffer)(
        RenderFactory& factory,
        const RenderComputeBufferDescriptor& descriptor);
    RenderParticleBuffer* (*create_particle_buffer)(
        RenderFactory& factory,
        std::uint32_t particle_count,
        void* context);
    RenderOcclusionQuery* (*create_occlusion_query)(
        RenderFactory& factory,
        void* owner);
};

const RenderFactoryDispatch& dispatch(const RenderFactory& factory) {
    return *static_cast<const RenderFactoryDispatch*>(factory.vtable);
}

static_assert(offsetof(RenderFactoryDispatch, create_mesh) == 24);
static_assert(
    offsetof(RenderFactoryDispatch, create_constant_buffer) == 88);

}  // namespace

RenderMesh* render_factory_create_mesh(
    RenderFactory& factory,
    RenderMeshFormat format,
    const char* name) {
    return dispatch(factory).create_mesh(factory, format, name);
}

RenderConstantBuffer* render_factory_create_constant_buffer(
    RenderFactory& factory,
    const RenderConstantBufferDescriptor& descriptor,
    std::uint32_t flags,
    std::size_t element_count) {
    return dispatch(factory).create_constant_buffer(
        factory, descriptor, flags, element_count);
}

RenderShader* render_factory_create_shader(
    RenderFactory& factory,
    RenderShaderStage stage) {
    return dispatch(factory).create_shader(factory, stage);
}

RenderComputeBuffer* render_factory_create_compute_buffer(
    RenderFactory& factory,
    const RenderComputeBufferDescriptor& descriptor) {
    return dispatch(factory).create_compute_buffer(factory, descriptor);
}

RenderParticleBuffer* render_factory_create_particle_buffer(
    RenderFactory& factory,
    std::uint32_t particle_count,
    void* context) {
    return dispatch(factory).create_particle_buffer(
        factory, particle_count, context);
}

RenderOcclusionQuery* render_factory_create_occlusion_query(
    RenderFactory& factory,
    void* owner) {
    return dispatch(factory).create_occlusion_query(factory, owner);
}

}  // namespace rb4
