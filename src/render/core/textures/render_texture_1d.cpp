#include "render/core/textures/render_texture_1d.h"

#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture_adapters.h"
#include "render/core/textures/render_texture_1d_adapters.h"
#include "render/core/textures/render_texture_mip_chain.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x6F5A90.
void render_texture_1d_descriptor_construct(
    RenderTexture1DDescriptor& descriptor) {
    render_texture_descriptor_construct(descriptor.texture_state);
    render_texture_mip_chain_descriptor_construct(descriptor.mip_chain);
    descriptor.texture_state.descriptor_type = 0;
}

// Reconstructed from eboot.elf at 0x6F5870.
void render_texture_1d_construct(
    RenderTexture1D& texture,
    const RenderTexture1DDescriptor& descriptor) {
    render_texture_construct(texture);
    render_texture_1d_set_base_dispatch(texture);

    texture.descriptor_state = descriptor.texture_state;
    render_texture_mip_chain_construct(
        texture.mip_chain,
        descriptor.mip_chain,
        render_texture_descriptor_has_source_data(descriptor.texture_state));

    texture.descriptor_state.data_format =
        descriptor.mip_chain.fields.data_format;
    texture.descriptor_state.width = descriptor.mip_chain.fields.width;
    texture.descriptor_state.height = descriptor.mip_chain.fields.height;
    texture.descriptor_state.depth = descriptor.mip_chain.fields.depth;
    render_texture_apply_descriptor_state(
        texture, texture.descriptor_state);
}

// Reconstructed from eboot.elf at 0x6F5810.
RenderTexture1D* render_create_texture_1d(
    RenderTexture1DDescriptor& descriptor,
    RenderTexture1D* reusable_texture) {
    render_texture_resolve_descriptor_fields(
        &descriptor.texture_state.usage_type,
        0,
        descriptor.texture_state.creation_state.values,
        -1);
    auto& factory = *render_system_factory(*render_system_instance());
    auto* texture = render_factory_create_texture_1d(factory, descriptor);
    render_texture_initialize_backend(*texture, reusable_texture);
    return texture;
}

// Reconstructed from eboot.elf at 0x6F5970.
void render_texture_1d_destruct(RenderTexture1D& texture) {
    render_texture_1d_set_base_dispatch(texture);
    render_texture_mip_chain_destruct(texture.mip_chain);
    render_texture_destruct(texture);
}

// Reconstructed from eboot.elf at 0x6F59B0.
void render_texture_1d_delete(RenderTexture1D& texture) {
    render_texture_1d_destruct(texture);
    render_delete_texture_1d_storage(texture);
}

}  // namespace rb4
