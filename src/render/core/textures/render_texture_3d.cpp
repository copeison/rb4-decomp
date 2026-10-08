#include "render/core/textures/render_texture_3d.h"

#include <cstddef>
#include <cstring>

#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture_adapters.h"
#include "render/core/textures/render_texture_3d_adapters.h"

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
    const RenderTexture3DDescriptor& descriptor) {
    return descriptor.texture_state[124] != 0 ||
           (descriptor.texture_state[88] & 5U) != 0;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6F5CB0.
void render_texture_3d_construct(
    RenderTexture3D& texture,
    const RenderTexture3DDescriptor& descriptor) {
    render_texture_construct(texture);
    render_texture_3d_set_base_dispatch(texture);

    std::memcpy(
        texture.descriptor_state,
        descriptor.texture_state,
        sizeof(texture.descriptor_state));
    render_texture_mip_chain_construct(
        texture.mip_chain,
        &descriptor.mip_chain,
        descriptor_has_source_data(descriptor));

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

// Reconstructed from eboot.elf at 0x6F5C50.
RenderTexture3D* render_create_texture_3d(
    RenderTexture3DDescriptor& descriptor,
    RenderTexture3D* reusable_texture) {
    render_texture_resolve_descriptor_fields(
        descriptor.texture_state + 48,
        2,
        descriptor.texture_state + 4,
        -1);
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
