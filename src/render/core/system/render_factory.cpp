#include "render/core/system/render_factory.h"

namespace rb4 {

namespace {

struct RenderFactoryDispatchTail {
    std::uint8_t reserved[11 * sizeof(void*)];
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

const RenderFactoryDispatchTail& dispatch(const RenderFactory& factory) {
    return *static_cast<const RenderFactoryDispatchTail*>(factory.vtable);
}

}  // namespace

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
