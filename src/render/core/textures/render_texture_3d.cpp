#include "render/core/textures/render_texture_3d.h"

#include <cstddef>

#include "core/memory/engine_memory.h"
#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture_mip_chain.h"

namespace rb4 {

namespace {

struct RenderTexture3DDispatch {
    void (*destruct)(RenderTexture3D& texture);
    void (*delete_texture)(RenderTexture3D& texture);
    std::int32_t (*descriptor_type)(const RenderTexture3D& texture);
    void (*reserved_bind_methods[7])();
    std::size_t (*mip_level_count)(const RenderTexture3D& texture);
    std::size_t (*source_size)(const RenderTexture3D& texture);
    void (*apply_creation_state)(
        RenderTexture3D& texture,
        const RenderTextureCreationState& state);
    void (*release_source_data)(RenderTexture3D& texture);
    RenderTexture3D* (*identity)(
        RenderTexture3D& texture,
        std::int64_t& resource_index);
    void (*initialize_backend)(
        RenderTexture3D& texture,
        const RenderTexture3D* reusable_texture);
    bool (*reserved_predicate)(const RenderTexture3D& texture);
};

std::int32_t descriptor_type(const RenderTexture3D& texture) {
    return texture.descriptor_type;
}

std::size_t mip_level_count(const RenderTexture3D& texture) {
    return render_texture_mip_chain_level_count(texture.mip_chain);
}

std::size_t source_size(const RenderTexture3D& texture) {
    return render_texture_mip_chain_source_size(texture.mip_chain);
}

void apply_creation_state(
    RenderTexture3D& texture,
    const RenderTextureCreationState& state) {
    texture.descriptor_state.creation_state = state;
    render_texture_apply_descriptor_state(texture, texture.descriptor_state);
}

void release_source_data(RenderTexture3D& texture) {
    render_texture_mip_chain_release_source_data(texture.mip_chain);
}

RenderTexture3D* identity(RenderTexture3D& texture, std::int64_t&) {
    return &texture;
}

bool reserved_predicate(const RenderTexture3D&) {
    return false;
}

RenderTexture3DDispatch kBaseTexture3DDispatch{
    render_texture_3d_destruct,
    render_texture_3d_delete,
    descriptor_type,
    {},
    mip_level_count,
    source_size,
    apply_creation_state,
    release_source_data,
    identity,
    nullptr,
    reserved_predicate,
};

static_assert(offsetof(RenderTexture3DDispatch, mip_level_count) == 80);
static_assert(offsetof(RenderTexture3DDispatch, release_source_data) == 104);
static_assert(offsetof(RenderTexture3DDispatch, initialize_backend) == 120);
static_assert(sizeof(RenderTexture3DDispatch) == 17 * sizeof(void*));

void set_base_dispatch(RenderTexture3D& texture) {
    texture.implementation = &kBaseTexture3DDispatch;
}

}  // namespace

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
    set_base_dispatch(texture);

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
    set_base_dispatch(texture);
    render_texture_mip_chain_destruct(texture.mip_chain);
    render_texture_destruct(texture);
}

// Reconstructed from eboot.elf at 0x6F5DF0.
void render_texture_3d_delete(RenderTexture3D& texture) {
    render_texture_3d_destruct(texture);
    render_release(&texture);
}

}  // namespace rb4
