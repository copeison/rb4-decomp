#include "render/core/textures/render_texture_2d.h"

#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture_adapters.h"
#include "render/core/textures/render_texture_2d_adapters.h"
#include "render/core/textures/render_texture_mip_chain.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x690330.
void render_texture_2d_descriptor_construct(
    RenderTexture2DDescriptor& descriptor) {
    render_texture_descriptor_construct(descriptor.texture_state);
    render_texture_mip_chain_descriptor_construct(descriptor.mip_chain);
    descriptor.texture_state.descriptor_type = 1;
}

// Reconstructed from eboot.elf at 0x6900B0.
void render_texture_2d_construct(
    RenderTexture2D& texture,
    const RenderTexture2DDescriptor& descriptor) {
    render_texture_construct(texture);
    render_texture_2d_set_base_dispatch(texture);

    texture.descriptor_state = descriptor.texture_state;
    render_texture_mip_chain_construct(
        texture.mip_chain,
        descriptor.mip_chain,
        render_texture_descriptor_has_source_data(descriptor.texture_state));

    texture.linked_resource = nullptr;
    texture.linked_resource_index = -1;
    texture.resource_index = 0;
    texture.descriptor_state.data_format =
        descriptor.mip_chain.fields.data_format;
    texture.descriptor_state.width = descriptor.mip_chain.fields.width;
    texture.descriptor_state.height = descriptor.mip_chain.fields.height;
    texture.descriptor_state.depth = descriptor.mip_chain.fields.depth;
    render_texture_apply_descriptor_state(
        texture, texture.descriptor_state);
}

// Reconstructed from eboot.elf at 0x68FE80.
RenderTexture2D* render_create_texture_2d(
    RenderTexture2DDescriptor& descriptor) {
    render_texture_resolve_descriptor_fields(
        &descriptor.texture_state.usage_type,
        1,
        descriptor.texture_state.creation_state.values,
        -1);
    auto& factory = *render_system_factory(*render_system_instance());
    auto* texture = render_factory_create_texture_2d(factory, descriptor);
    render_texture_initialize_backend(*texture, nullptr);
    return texture;
}

// Reconstructed from eboot.elf at 0x6901D0.
void render_texture_2d_destruct(RenderTexture2D& texture) {
    render_texture_2d_set_base_dispatch(texture);
    render_texture_mip_chain_destruct(texture.mip_chain);
    render_texture_destruct(texture);
}

// Reconstructed from eboot.elf at 0x690210.
void render_texture_2d_delete(RenderTexture2D& texture) {
    render_texture_2d_destruct(texture);
    render_delete_texture_2d_storage(texture);
}

// Reconstructed from eboot.elf at 0x690250.
void render_texture_2d_set_linked_resource(
    RenderTexture2D& texture,
    void* resource,
    std::int64_t resource_index) {
    texture.linked_resource = resource;
    texture.linked_resource_index = resource_index;
}

}  // namespace rb4
