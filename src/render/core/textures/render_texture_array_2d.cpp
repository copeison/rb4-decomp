#include "render/core/textures/render_texture_array_2d.h"

#include <cstddef>
#include <cstring>

#include "render/core/textures/render_texture_array_2d_adapters.h"

namespace rb4 {

namespace {

template <typename T>
T read_value(const std::uint8_t* source, std::size_t offset) {
    T value;
    std::memcpy(&value, source + offset, sizeof(value));
    return value;
}

template <typename T>
void write_value(std::uint8_t* destination, std::size_t offset, T value) {
    std::memcpy(destination + offset, &value, sizeof(value));
}

}  // namespace

// Reconstructed from eboot.elf at 0x698160.
void render_texture_array_2d_construct(
    RenderTextureArray2D& texture,
    const RenderTextureArray2DDescriptor& descriptor) {
    render_texture_construct(texture);
    render_texture_array_2d_set_base_dispatch(texture);
    std::memcpy(
        texture.descriptor_state,
        &descriptor.texture_state,
        sizeof(texture.descriptor_state));

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
    const auto& first = texture.mip_chains.begin->storage;
    write_value(
        texture.descriptor_state,
        92,
        read_value<std::uint32_t>(first, 20));
    write_value(
        texture.descriptor_state,
        96,
        read_value<std::uint64_t>(first, 8));
    write_value(
        texture.descriptor_state,
        104,
        read_value<std::uint32_t>(first, 16));
    write_value(
        texture.descriptor_state,
        112,
        static_cast<std::uint64_t>(count));
    std::memcpy(
        &texture.descriptor_type,
        texture.descriptor_state,
        sizeof(texture.descriptor_state));
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
