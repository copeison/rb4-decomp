#include "render/core/textures/render_texture_array_2d.h"

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture_mip_chain.h"

namespace rb4 {

namespace {

struct RenderTextureArray2DDispatch {
    void (*destruct)(RenderTextureArray2D& texture);
    void (*delete_texture)(RenderTextureArray2D& texture);
    std::int32_t (*descriptor_type)(const RenderTextureArray2D& texture);
    void (*reserved_bind_methods[7])();
    std::size_t (*mip_level_count)(const RenderTextureArray2D& texture);
    std::size_t (*source_size)(const RenderTextureArray2D& texture);
    void (*apply_creation_state)(
        RenderTextureArray2D& texture,
        const RenderTextureCreationState& state);
    void (*release_source_data)(RenderTextureArray2D& texture);
    RenderTextureArray2D* (*identity)(
        RenderTextureArray2D& texture,
        std::int64_t& resource_index);
    void (*initialize_backend)(
        RenderTextureArray2D& texture,
        const RenderTextureArray2D* reusable_texture);
    bool (*reserved_predicate)(const RenderTextureArray2D& texture);
};

std::int32_t descriptor_type(const RenderTextureArray2D& texture) {
    return texture.descriptor_type;
}

std::size_t mip_level_count(const RenderTextureArray2D& texture) {
    return render_texture_mip_chain_level_count(*texture.mip_chains.begin);
}

std::size_t source_size(const RenderTextureArray2D& texture) {
    return render_texture_mip_chain_array_source_size(texture.mip_chains);
}

void apply_creation_state(
    RenderTextureArray2D& texture,
    const RenderTextureCreationState& state) {
    texture.descriptor_state.creation_state = state;
    render_texture_apply_descriptor_state(texture, texture.descriptor_state);
}

void release_source_data(RenderTextureArray2D& texture) {
    render_texture_mip_chain_array_release_source_data(texture.mip_chains);
}

RenderTextureArray2D* identity(
    RenderTextureArray2D& texture,
    std::int64_t&) {
    return &texture;
}

bool reserved_predicate(const RenderTextureArray2D&) {
    return false;
}

RenderTextureArray2DDispatch kBaseTextureArray2DDispatch{
    render_texture_array_2d_destruct,
    render_texture_array_2d_delete,
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
    offsetof(RenderTextureArray2DDispatch, mip_level_count) == 80);
static_assert(
    offsetof(RenderTextureArray2DDispatch, release_source_data) == 104);
static_assert(
    offsetof(RenderTextureArray2DDispatch, initialize_backend) == 120);
static_assert(sizeof(RenderTextureArray2DDispatch) == 17 * sizeof(void*));

void set_base_dispatch(RenderTextureArray2D& texture) {
    texture.implementation = &kBaseTextureArray2DDispatch;
}

}  // namespace

// Reconstructed from eboot.elf at 0x698910.
bool render_texture_array_2d_validate(
    const RenderTextureArray2D& texture) {
    const auto& mip_chains = texture.mip_chains;
    if (mip_chains.begin == mip_chains.end || mip_chains.begin == nullptr) {
        return false;
    }
    const auto count = static_cast<std::size_t>(
        mip_chains.end - mip_chains.begin);
    if (count > 2048) {
        return false;
    }

    const auto& first = *mip_chains.begin;
    if (first.fields.depth != 1) {
        return false;
    }
    const auto expected_levels =
        render_texture_mip_chain_level_count(first);
    for (auto* mip_chain = mip_chains.begin + 1;
         mip_chain != mip_chains.end;
         ++mip_chain) {
        if (mip_chain->fields.width != first.fields.width ||
            mip_chain->fields.height != first.fields.height ||
            mip_chain->fields.depth != first.fields.depth ||
            mip_chain->fields.data_format != first.fields.data_format ||
            render_texture_mip_chain_level_count(*mip_chain) !=
                expected_levels) {
            return false;
        }
    }
    return true;
}

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
    render_texture_array_2d_validate(texture);

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
        descriptor.texture_state, 5, -1);
    auto& factory = *render_system_factory(*render_system_instance());
    auto* texture = render_factory_create_texture_array_2d(
        factory, descriptor);
    render_texture_initialize_backend(*texture, nullptr);
    return texture;
}

// Reconstructed from eboot.elf at 0x6984C0.
void render_texture_array_2d_destruct(RenderTextureArray2D& texture) {
    set_base_dispatch(texture);
    render_texture_mip_chain_array_destruct(texture.mip_chains);
    render_texture_destruct(texture);
}

// Reconstructed from eboot.elf at 0x698540.
void render_texture_array_2d_delete(RenderTextureArray2D& texture) {
    render_texture_array_2d_destruct(texture);
    MemFree(&texture);
}

}  // namespace rb4
