#include "render/platform/orbis/system/orbis_render_factory.h"

#include <cstddef>
#include <cstdint>

#include "os/memory/MemMgr.h"
#include "render/platform/orbis/buffers/orbis_compute_buffer.h"
#include "render/platform/orbis/buffers/orbis_constant_buffer.h"
#include "render/platform/orbis/buffers/orbis_particle_buffer.h"
#include "render/platform/orbis/meshes/orbis_mesh.h"
#include "render/platform/orbis/shaders/orbis_shader.h"
#include "render/platform/orbis/synchronization/orbis_fence.h"
#include "render/platform/orbis/synchronization/orbis_occlusion_query.h"
#include "render/platform/orbis/textures/orbis_texture_1d.h"
#include "render/platform/orbis/textures/orbis_texture_2d.h"
#include "render/platform/orbis/textures/orbis_texture_3d.h"
#include "render/platform/orbis/textures/orbis_texture_array_1d.h"
#include "render/platform/orbis/textures/orbis_texture_array_2d.h"
#include "render/platform/orbis/textures/orbis_texture_array_cube.h"
#include "render/platform/orbis/textures/orbis_texture_cube.h"

namespace rb4 {

namespace {

struct OrbisRenderFactoryVtable {
    void (*destruct)(OrbisRenderFactory& factory);
    void (*destroy)(OrbisRenderFactory& factory);
    OrbisFence* (*create_fence)(OrbisRenderFactory& factory);
    OrbisMesh* (*create_mesh)(
        OrbisRenderFactory& factory,
        RenderMeshFormat format,
        const char* name);
    OrbisTexture1D* (*create_texture_1d)(
        OrbisRenderFactory& factory,
        const RenderTexture1DDescriptor& descriptor);
    OrbisTexture2D* (*create_texture_2d)(
        OrbisRenderFactory& factory,
        const RenderTexture2DDescriptor& descriptor);
    OrbisTexture3D* (*create_texture_3d)(
        OrbisRenderFactory& factory,
        const RenderTexture3DDescriptor& descriptor);
    OrbisTextureCube* (*create_texture_cube)(
        OrbisRenderFactory& factory,
        const RenderTextureCubeDescriptor& descriptor);
    OrbisTextureArray1D* (*create_texture_array_1d)(
        OrbisRenderFactory& factory,
        const RenderTextureArray1DDescriptor& descriptor);
    OrbisTextureArray2D* (*create_texture_array_2d)(
        OrbisRenderFactory& factory,
        const RenderTextureArray2DDescriptor& descriptor);
    OrbisTextureArrayCube* (*create_texture_array_cube)(
        OrbisRenderFactory& factory,
        const RenderTextureArrayCubeDescriptor& descriptor);
    OrbisConstantBuffer* (*create_constant_buffer)(
        RenderFactory& factory,
        const RenderConstantBufferDescriptor& descriptor,
        std::uint32_t flags,
        std::size_t element_count);
    OrbisShader* (*create_shader)(
        RenderFactory& factory,
        RenderShaderStage stage);
    OrbisComputeBuffer* (*create_compute_buffer)(
        RenderFactory& factory,
        const RenderComputeBufferDescriptor& descriptor);
    OrbisParticleBuffer* (*create_particle_buffer)(
        RenderFactory& factory,
        std::uint32_t particle_count,
        void* context);
    OrbisOcclusionQuery* (*create_occlusion_query)(
        RenderFactory& factory,
        void* owner);
};

static_assert(sizeof(OrbisRenderFactoryVtable) == 16 * sizeof(void*));

void factory_destruct(OrbisRenderFactory&) {}

void factory_destroy(OrbisRenderFactory& factory) {
    operator delete(&factory);
}

OrbisFence* factory_create_fence(OrbisRenderFactory&) {
    return orbis_create_fence();
}

OrbisMesh* factory_create_mesh(
    OrbisRenderFactory&,
    RenderMeshFormat format,
    const char* name) {
    return orbis_create_mesh(format, name);
}

OrbisTexture1D* factory_create_texture_1d(
    OrbisRenderFactory&,
    const RenderTexture1DDescriptor& descriptor) {
    return orbis_create_texture_1d(descriptor);
}

OrbisTexture2D* factory_create_texture_2d(
    OrbisRenderFactory&,
    const RenderTexture2DDescriptor& descriptor) {
    return orbis_create_texture_2d(descriptor);
}

OrbisTexture3D* factory_create_texture_3d(
    OrbisRenderFactory&,
    const RenderTexture3DDescriptor& descriptor) {
    return orbis_create_texture_3d(descriptor);
}

OrbisTextureCube* factory_create_texture_cube(
    OrbisRenderFactory&,
    const RenderTextureCubeDescriptor& descriptor) {
    return orbis_create_texture_cube(descriptor);
}

OrbisTextureArray1D* factory_create_texture_array_1d(
    OrbisRenderFactory&,
    const RenderTextureArray1DDescriptor& descriptor) {
    return orbis_create_texture_array_1d(descriptor);
}

OrbisTextureArray2D* factory_create_texture_array_2d(
    OrbisRenderFactory&,
    const RenderTextureArray2DDescriptor& descriptor) {
    return orbis_create_texture_array_2d(descriptor);
}

OrbisTextureArrayCube* factory_create_texture_array_cube(
    OrbisRenderFactory&,
    const RenderTextureArrayCubeDescriptor& descriptor) {
    return orbis_create_texture_array_cube(descriptor);
}

OrbisConstantBuffer* factory_create_constant_buffer(
    RenderFactory&,
    const RenderConstantBufferDescriptor& descriptor,
    std::uint32_t flags,
    std::size_t element_count) {
    return orbis_create_constant_buffer(descriptor, flags, element_count);
}

OrbisShader* factory_create_shader(
    RenderFactory&,
    RenderShaderStage stage) {
    return orbis_create_shader(stage);
}

OrbisComputeBuffer* factory_create_compute_buffer(
    RenderFactory&,
    const RenderComputeBufferDescriptor& descriptor) {
    return orbis_create_compute_buffer(descriptor);
}

OrbisParticleBuffer* factory_create_particle_buffer(
    RenderFactory&,
    std::uint32_t particle_count,
    void* context) {
    return orbis_create_particle_buffer(particle_count, context);
}

OrbisOcclusionQuery* factory_create_occlusion_query(
    RenderFactory&,
    void* owner) {
    return orbis_create_occlusion_query(owner);
}

const OrbisRenderFactoryVtable kOrbisRenderFactoryVtable = {
    factory_destruct,
    factory_destroy,
    factory_create_fence,
    factory_create_mesh,
    factory_create_texture_1d,
    factory_create_texture_2d,
    factory_create_texture_3d,
    factory_create_texture_cube,
    factory_create_texture_array_1d,
    factory_create_texture_array_2d,
    factory_create_texture_array_cube,
    factory_create_constant_buffer,
    factory_create_shader,
    factory_create_compute_buffer,
    factory_create_particle_buffer,
    factory_create_occlusion_query,
};

}  // namespace

OrbisRenderFactory* orbis_render_factory_create() {
    auto* storage = operator new(sizeof(OrbisRenderFactory));
    auto* factory = static_cast<OrbisRenderFactory*>(storage);
    factory->vtable = &kOrbisRenderFactoryVtable;
    return factory;
}

}  // namespace rb4
