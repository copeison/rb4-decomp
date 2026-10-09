#include "render/core/textures/render_texture_array_1d.h"

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "render/system/RndFactory.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture_mip_chain.h"

namespace rb4 {

namespace {

struct RenderTextureArray1DDispatch {
    void (*destruct)(RenderTextureArray1D& texture);
    void (*delete_texture)(RenderTextureArray1D& texture);
    std::int32_t (*descriptor_type)(const RenderTextureArray1D& texture);
    void (*reserved_bind_methods[7])();
    std::size_t (*mip_level_count)(const RenderTextureArray1D& texture);
    std::size_t (*source_size)(const RenderTextureArray1D& texture);
    void (*apply_creation_state)(
        RenderTextureArray1D& texture,
        const RenderTextureCreationState& state);
    void (*release_source_data)(RenderTextureArray1D& texture);
    RenderTextureArray1D* (*identity)(
        RenderTextureArray1D& texture,
        std::int64_t& resource_index);
    void (*initialize_backend)(
        RenderTextureArray1D& texture,
        const RenderTextureArray1D* reusable_texture);
    bool (*reserved_predicate)(const RenderTextureArray1D& texture);
};

std::int32_t descriptor_type(const RenderTextureArray1D& texture) {
    return texture.descriptor_type;
}

std::size_t mip_level_count(const RenderTextureArray1D& texture) {
    return render_texture_mip_chain_level_count(*texture.mip_chains.begin);
}

std::size_t source_size(const RenderTextureArray1D& texture) {
    return render_texture_mip_chain_array_source_size(texture.mip_chains);
}

void apply_creation_state(
    RenderTextureArray1D& texture,
    const RenderTextureCreationState& state) {
    texture.descriptor_state.creation_state = state;
    render_texture_apply_descriptor_state(texture, texture.descriptor_state);
}

void release_source_data(RenderTextureArray1D& texture) {
    render_texture_mip_chain_array_release_source_data(texture.mip_chains);
}

RenderTextureArray1D* identity(
    RenderTextureArray1D& texture,
    std::int64_t&) {
    return &texture;
}

bool reserved_predicate(const RenderTextureArray1D&) {
    return false;
}

RenderTextureArray1DDispatch kBaseTextureArray1DDispatch{
    render_texture_array_1d_destruct,
    render_texture_array_1d_delete,
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

static_assert(
    offsetof(RenderTextureArray1DDispatch, mip_level_count) == 80);
static_assert(
    offsetof(RenderTextureArray1DDispatch, release_source_data) == 104);
static_assert(
    offsetof(RenderTextureArray1DDispatch, initialize_backend) == 120);
static_assert(sizeof(RenderTextureArray1DDispatch) == 17 * sizeof(void*));

void set_base_dispatch(RenderTextureArray1D& texture) {
    texture.implementation = &kBaseTextureArray1DDispatch;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6972D0.
void render_texture_array_1d_descriptor_construct(
    RenderTextureArray1DDescriptor& descriptor) {
    render_texture_descriptor_construct(descriptor.texture_state);
    descriptor.mip_chains = {};
    descriptor.texture_state.descriptor_type = 4;
}

// Reconstructed from eboot.elf at 0x696C00.
void render_texture_array_1d_construct(
    RenderTextureArray1D& texture,
    const RenderTextureArray1DDescriptor& descriptor) {
    render_texture_construct(texture);
    set_base_dispatch(texture);
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
    render_texture_mip_chain_array_validate(texture.mip_chains);

    const auto& first = texture.mip_chains.begin->fields;
    texture.descriptor_state.data_format = first.data_format;
    texture.descriptor_state.width = first.width;
    texture.descriptor_state.height = first.height;
    texture.descriptor_state.depth = first.depth;
    texture.descriptor_state.array_size = count;
    render_texture_apply_descriptor_state(
        texture, texture.descriptor_state);
}

// Reconstructed from eboot.elf at 0x696BA0.
RenderTextureArray1D* render_create_texture_array_1d(
    RenderTextureArray1DDescriptor& descriptor,
    RenderTextureArray1D* reusable_texture) {
    render_texture_resolve_descriptor_fields(
        descriptor.texture_state, 4, -1);
    auto& factory = *render_system_factory(*render_system_instance());
    auto* texture = factory.CreateTextureArray1D(descriptor);
    render_texture_initialize_backend(*texture, reusable_texture);
    return texture;
}

// Reconstructed from eboot.elf at 0x697020.
void render_texture_array_1d_destruct(RenderTextureArray1D& texture) {
    set_base_dispatch(texture);
    render_texture_mip_chain_array_destruct(texture.mip_chains);
    render_texture_destruct(texture);
}

// Reconstructed from eboot.elf at 0x6970A0.
void render_texture_array_1d_delete(RenderTextureArray1D& texture) {
    render_texture_array_1d_destruct(texture);
    MemFree(&texture);
}

}  // namespace rb4
