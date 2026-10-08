#include "render/core/textures/render_texture_3d.h"

#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture_adapters.h"
#include "render/core/textures/render_texture_3d_adapters.h"
#include "render/core/textures/render_texture_mip_chain.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x6F5ED0.
void render_texture_3d_descriptor_construct(
    RenderTexture3DDescriptor& descriptor) {
    render_texture_descriptor_construct(descriptor.texture_state);
    render_texture_mip_chain_descriptor_construct(descriptor.mip_chain);
    descriptor.texture_state.descriptor_type = 2;
}

// Reconstructed from eboot.elf at 0x6F5CB0.
void render_texture_3d_construct(
    RenderTexture3D& texture,
    const RenderTexture3DDescriptor& descriptor) {
    render_texture_construct(texture);
    render_texture_3d_set_base_dispatch(texture);

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

// Reconstructed from eboot.elf at 0x6F5C50.
RenderTexture3D* render_create_texture_3d(
    RenderTexture3DDescriptor& descriptor,
    RenderTexture3D* reusable_texture) {
    render_texture_resolve_descriptor_fields(
        descriptor.texture_state, 2, -1);
    auto& factory = *render_system_factory(*render_system_instance());
    auto* texture = render_factory_create_texture_3d(factory, descriptor);
    render_texture_initialize_backend(*texture, reusable_texture);
    return texture;
}

// Reconstructed from eboot.elf at 0x6F5DB0.
void render_texture_3d_destruct(RenderTexture3D& texture) {
    render_texture_3d_set_base_dispatch(texture);
    render_texture_mip_chain_destruct(texture.mip_chain);
    render_texture_destruct(texture);
}

// Reconstructed from eboot.elf at 0x6F5DF0.
void render_texture_3d_delete(RenderTexture3D& texture) {
    render_texture_3d_destruct(texture);
    render_delete_texture_3d_storage(texture);
}

}  // namespace rb4
