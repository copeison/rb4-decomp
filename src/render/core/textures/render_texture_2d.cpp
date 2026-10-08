#include "render/core/textures/render_texture_2d.h"

#include <cstddef>
#include <cstring>

#include "render/core/textures/render_texture_2d_adapters.h"

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

bool descriptor_has_source_data(
    const RenderTexture2DDescriptor& descriptor) {
    return descriptor.texture_state.backend_initialized ||
           (descriptor.texture_state.flags & 5U) != 0;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6900B0.
void render_texture_2d_construct(
    RenderTexture2D& texture,
    const RenderTexture2DDescriptor& descriptor) {
    render_texture_construct(texture);
    render_texture_2d_set_base_dispatch(texture);

    std::memcpy(
        texture.descriptor_state,
        &descriptor.texture_state,
        sizeof(texture.descriptor_state));
    render_texture_mip_chain_construct(
        texture.mip_chain,
        &descriptor.mip_chain,
        descriptor_has_source_data(descriptor));

    texture.linked_resource = nullptr;
    texture.linked_resource_index = -1;
    texture.resource_index = 0;
    write_value(
        texture.descriptor_state,
        92,
        descriptor.mip_chain.fields.data_format);
    write_value(
        texture.descriptor_state,
        96,
        read_value<std::uint64_t>(descriptor.mip_chain.storage, 8));
    write_value(
        texture.descriptor_state,
        104,
        descriptor.mip_chain.fields.depth);
    std::memcpy(
        &texture.descriptor_type,
        texture.descriptor_state,
        sizeof(texture.descriptor_state));
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
