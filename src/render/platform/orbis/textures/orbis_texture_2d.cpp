#include "render/platform/orbis/textures/orbis_texture_2d.h"

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_2d_adapters.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x8D89B0.
OrbisTexture2D* orbis_create_texture_2d(
    const RenderTexture2DDescriptor& descriptor) {
    auto* storage = render_allocate(sizeof(OrbisTexture2D));
    auto* texture = reinterpret_cast<OrbisTexture2D*>(storage);
    orbis_texture_2d_construct(*texture, descriptor);
    return texture;
}

// Reconstructed from eboot.elf at 0x8D62C0.
void orbis_texture_2d_construct(
    OrbisTexture2D& texture,
    const RenderTexture2DDescriptor& descriptor) {
    render_texture_2d_construct(texture, descriptor);
    orbis_texture_2d_install_vtable(texture);
    texture.color_textures[0] = nullptr;
    texture.color_textures[1] = nullptr;
    texture.plane_texture = nullptr;
    texture.depth_memory = {};
    texture.stencil_memory = {};
    texture.metadata_memory = {};
    texture.active_storage = 0;
    texture.allocation_regions = nullptr;
    texture.allocation_control = nullptr;
    texture.render_targets[0] = nullptr;
    texture.render_targets[1] = nullptr;
    texture.depth_target = nullptr;
    texture.auxiliary_backend = nullptr;
}

// Reconstructed from eboot.elf at 0x8D6310.
void orbis_texture_2d_destruct(OrbisTexture2D& texture) {
    orbis_texture_2d_install_vtable(texture);

    for (auto*& target : texture.render_targets) {
        if (target != nullptr) {
            render_release(target);
            target = nullptr;
        }
    }
    for (auto*& color_texture : texture.color_textures) {
        if (color_texture != nullptr) {
            render_release(color_texture);
            color_texture = nullptr;
        }
    }
    if (texture.plane_texture != nullptr) {
        render_release(texture.plane_texture);
        texture.plane_texture = nullptr;
    }
    if (texture.depth_target != nullptr) {
        render_release(texture.depth_target);
        texture.depth_target = nullptr;
    }
    if (texture.auxiliary_backend != nullptr) {
        orbis_texture_2d_release_auxiliary(texture.auxiliary_backend);
        texture.auxiliary_backend = nullptr;
    }
    orbis_texture_2d_release_allocation(texture.allocation_control);
    texture.allocation_control = nullptr;
    texture.allocation_regions = nullptr;
    render_texture_2d_destruct(texture);
}

// Reconstructed from eboot.elf at 0x8D6440.
void orbis_texture_2d_delete(OrbisTexture2D& texture) {
    orbis_texture_2d_destruct(texture);
    render_delete_texture_2d_storage(texture);
}

// Reconstructed from eboot.elf at 0x8D6460.
void orbis_texture_2d_initialize_backend(
    OrbisTexture2D& texture,
    const OrbisTexture2D* storage_source) {
    if (texture.usage_type == RenderTextureUsage::kDepth) {
        orbis_texture_2d_initialize_depth_storage(texture);
    } else {
        orbis_texture_2d_initialize_color_storage(texture, storage_source);
    }
}

// Reconstructed from eboot.elf at 0x8D6D10.
void orbis_texture_2d_update_gpu_data(OrbisTexture2D& texture) {
    orbis_texture_2d_flip_active_storage(texture);
    orbis_texture_2d_upload_active_mips(texture);
}

// Reconstructed from eboot.elf at 0x8D71E0.
const OrbisGpuRenderTarget* orbis_texture_2d_render_target(
    const OrbisTexture2D& texture) {
    const auto active_frame = orbis_active_render_frame_index();
    auto* target = texture.render_targets[active_frame];
    return target != nullptr ? target : texture.render_targets[0];
}

// Reconstructed from eboot.elf at 0x8D7240.
const OrbisGpuDepthRenderTarget* orbis_texture_2d_depth_target(
    const OrbisTexture2D& texture) {
    return texture.depth_target;
}

// Reconstructed from eboot.elf at 0x8D6E40.
void orbis_texture_2d_bind_vertex(
    const OrbisTexture2D& texture,
    OrbisRenderContext& context,
    std::uint32_t slot,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    orbis_bind_vertex_texture(
        context, slot, orbis_texture_2d_binding_view(texture, flags),
        static_cast<OrbisSamplerAddressMode>(texture.address_mode),
        texture.filter_mode, border_color);
}

// Reconstructed from eboot.elf at 0x8D6F10.
void orbis_texture_2d_bind_hull(
    const OrbisTexture2D& texture,
    OrbisRenderContext& context,
    std::uint32_t slot,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    orbis_bind_hull_texture(
        context, slot, orbis_texture_2d_binding_view(texture, flags),
        static_cast<OrbisSamplerAddressMode>(texture.address_mode),
        texture.filter_mode, flags, border_color);
}

// Reconstructed from eboot.elf at 0x8D6F80.
void orbis_texture_2d_bind_domain(
    const OrbisTexture2D& texture,
    OrbisRenderContext& context,
    std::uint32_t slot,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    orbis_bind_domain_texture(
        context, slot, orbis_texture_2d_binding_view(texture, flags),
        static_cast<OrbisSamplerAddressMode>(texture.address_mode),
        texture.filter_mode, flags, border_color);
}

// Reconstructed from eboot.elf at 0x8D6FF0.
void orbis_texture_2d_bind_geometry(
    const OrbisTexture2D& texture,
    OrbisRenderContext& context,
    std::uint32_t slot,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    orbis_bind_geometry_texture(
        context, slot, orbis_texture_2d_binding_view(texture, flags),
        static_cast<OrbisSamplerAddressMode>(texture.address_mode),
        texture.filter_mode, flags, border_color);
}

// Reconstructed from eboot.elf at 0x8D7060.
void orbis_texture_2d_bind_pixel(
    const OrbisTexture2D& texture,
    OrbisRenderContext& context,
    std::uint32_t slot,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    orbis_bind_pixel_texture(
        context, slot, orbis_texture_2d_binding_view(texture, flags),
        static_cast<OrbisSamplerAddressMode>(texture.address_mode),
        texture.filter_mode, flags, border_color);
}

// Reconstructed from eboot.elf at 0x8D70D0.
void orbis_texture_2d_bind_compute(
    const OrbisTexture2D& texture,
    OrbisRenderContext& context,
    std::uint32_t slot,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    orbis_bind_compute_texture(
        context, slot, orbis_texture_2d_binding_view(texture, flags),
        static_cast<OrbisSamplerAddressMode>(texture.address_mode),
        texture.filter_mode, flags, border_color);
}

}  // namespace rb4
