#include "render/core/textures/render_texture_1d.h"

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "render/system/RndFactory.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture_mip_chain.h"

namespace rb4 {

namespace {

struct RenderTexture1DDispatch {
    void (*destruct)(RenderTexture1D& texture);
    void (*delete_texture)(RenderTexture1D& texture);
    std::int32_t (*descriptor_type)(const RenderTexture1D& texture);
    void (*reserved_bind_methods[7])();
    std::size_t (*mip_level_count)(const RenderTexture1D& texture);
    std::size_t (*source_size)(const RenderTexture1D& texture);
    void (*apply_creation_state)(
        RenderTexture1D& texture,
        const RenderTextureCreationState& state);
    void (*release_source_data)(RenderTexture1D& texture);
    RenderTexture1D* (*identity)(
        RenderTexture1D& texture,
        std::int64_t& resource_index);
    void (*initialize_backend)(
        RenderTexture1D& texture,
        const RenderTexture1D* reusable_texture);
    bool (*reserved_predicate)(const RenderTexture1D& texture);
};

std::int32_t descriptor_type(const RenderTexture1D& texture) {
    return texture.descriptor_type;
}

std::size_t mip_level_count(const RenderTexture1D& texture) {
    return render_texture_mip_chain_level_count(texture.mip_chain);
}

std::size_t source_size(const RenderTexture1D& texture) {
    return render_texture_mip_chain_source_size(texture.mip_chain);
}

void apply_creation_state(
    RenderTexture1D& texture,
    const RenderTextureCreationState& state) {
    texture.descriptor_state.creation_state = state;
    render_texture_apply_descriptor_state(texture, texture.descriptor_state);
}

void release_source_data(RenderTexture1D& texture) {
    render_texture_mip_chain_release_source_data(texture.mip_chain);
}

RenderTexture1D* identity(RenderTexture1D& texture, std::int64_t&) {
    return &texture;
}

bool reserved_predicate(const RenderTexture1D&) {
    return false;
}

RenderTexture1DDispatch kBaseTexture1DDispatch{
    render_texture_1d_destruct,
    render_texture_1d_delete,
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

static_assert(offsetof(RenderTexture1DDispatch, mip_level_count) == 80);
static_assert(offsetof(RenderTexture1DDispatch, release_source_data) == 104);
static_assert(offsetof(RenderTexture1DDispatch, initialize_backend) == 120);
static_assert(sizeof(RenderTexture1DDispatch) == 17 * sizeof(void*));

void set_base_dispatch(RenderTexture1D& texture) {
    texture.implementation = &kBaseTexture1DDispatch;
}

}  // namespace

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

// Reconstructed from eboot.elf at 0x6F5810.
RenderTexture1D* render_create_texture_1d(
    RenderTexture1DDescriptor& descriptor,
    RenderTexture1D* reusable_texture) {
    render_texture_resolve_descriptor_fields(
        descriptor.texture_state, 0, -1);
    auto& factory = *render_system_factory(*render_system_instance());
    auto* texture = factory.CreateTexture1D(descriptor);
    render_texture_initialize_backend(*texture, reusable_texture);
    return texture;
}

// Reconstructed from eboot.elf at 0x6F5970.
void render_texture_1d_destruct(RenderTexture1D& texture) {
    set_base_dispatch(texture);
    render_texture_mip_chain_destruct(texture.mip_chain);
    render_texture_destruct(texture);
}

// Reconstructed from eboot.elf at 0x6F59B0.
void render_texture_1d_delete(RenderTexture1D& texture) {
    render_texture_1d_destruct(texture);
    MemFree(&texture);
}

}  // namespace rb4
