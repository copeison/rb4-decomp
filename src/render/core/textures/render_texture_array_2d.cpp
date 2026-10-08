#include "render/core/textures/render_texture_array_2d.h"

#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture_adapters.h"
#include "render/core/textures/render_texture_array_2d_adapters.h"
#include "render/core/textures/render_texture_mip_chain_adapters.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x698770.
void render_texture_array_2d_descriptor_construct(
    RenderTextureArray2DDescriptor& descriptor) {
    render_texture_descriptor_construct(descriptor.texture_state);
    descriptor.mip_chains = {};
    descriptor.texture_state.descriptor_type = 5;
}

// Reconstructed from eboot.elf at 0x698160.
void render_texture_array_2d_construct(
    RenderTextureArray2D& texture,
    const RenderTextureArray2DDescriptor& descriptor) {
    render_texture_construct(texture);
    render_texture_array_2d_set_base_dispatch(texture);
    texture.descriptor_state = descriptor.texture_state;

    render_texture_mip_chain_array_construct(texture.mip_chains);
    const auto count = static_cast<std::size_t>(
        descriptor.mip_chains.end - descriptor.mip_chains.begin);
    render_texture_mip_chain_array_reserve(texture.mip_chains, count);
    for (auto* mip_chain = descriptor.mip_chains.begin;
         mip_chain != descriptor.mip_chains.end;
         ++mip_chain) {
        render_texture_mip_chain_array_append(
            texture.mip_chains,
            *mip_chain,
            render_texture_descriptor_has_source_data(descriptor.texture_state));
    }
    render_texture_array_2d_resolve_descriptor(texture.descriptor_state);

    texture.resource_index = 0;
    const auto& first = texture.mip_chains.begin->fields;
    texture.descriptor_state.data_format = first.data_format;
    texture.descriptor_state.width = first.width;
    texture.descriptor_state.height = first.height;
    texture.descriptor_state.depth = first.depth;
    texture.descriptor_state.array_size = count;
    render_texture_apply_descriptor_state(
        texture, texture.descriptor_state);
}

// Reconstructed from eboot.elf at 0x698100.
RenderTextureArray2D* render_create_texture_array_2d(
    RenderTextureArray2DDescriptor& descriptor) {
    render_texture_resolve_descriptor_fields(
        &descriptor.texture_state.usage_type,
        5,
        descriptor.texture_state.creation_state.values,
        -1);
    auto& factory = *render_system_factory(*render_system_instance());
    auto* texture = render_factory_create_texture_array_2d(
        factory, descriptor);
    render_texture_initialize_backend(*texture, nullptr);
    return texture;
}

// Reconstructed from eboot.elf at 0x6984C0.
void render_texture_array_2d_destruct(RenderTextureArray2D& texture) {
    render_texture_array_2d_set_base_dispatch(texture);
    render_texture_mip_chain_array_destruct(texture.mip_chains);
    render_texture_destruct(texture);
}

// Reconstructed from eboot.elf at 0x698540.
void render_texture_array_2d_delete(RenderTextureArray2D& texture) {
    render_texture_array_2d_destruct(texture);
    render_delete_texture_array_2d_storage(texture);
}

}  // namespace rb4
