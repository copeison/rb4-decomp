#include "render/core/textures/render_texture_2d.h"

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture_mip_chain.h"

namespace rb4 {

namespace {

struct RenderTexture2DDispatch {
    void (*destruct)(RenderTexture2D& texture);
    void (*delete_texture)(RenderTexture2D& texture);
    std::int32_t (*descriptor_type)(const RenderTexture2D& texture);
    void (*reserved_bind_methods[7])();
    std::size_t (*mip_level_count)(const RenderTexture2D& texture);
    std::size_t (*source_size)(const RenderTexture2D& texture);
    void (*apply_creation_state)(
        RenderTexture2D& texture,
        const RenderTextureCreationState& state);
    void (*release_source_data)(RenderTexture2D& texture);
    RenderTexture* (*resolve_resource)(
        RenderTexture2D& texture,
        std::int64_t& resource_index);
    void (*initialize_backend)(
        RenderTexture2D& texture,
        const RenderTexture2D* reusable_texture);
    bool (*reserved_predicate)(const RenderTexture2D& texture);
};

std::int32_t descriptor_type(const RenderTexture2D& texture) {
    return texture.descriptor_type;
}

std::size_t mip_level_count(const RenderTexture2D& texture) {
    return render_texture_mip_chain_level_count(texture.mip_chain);
}

std::size_t source_size(const RenderTexture2D& texture) {
    return render_texture_mip_chain_source_size(texture.mip_chain);
}

void apply_creation_state(
    RenderTexture2D& texture,
    const RenderTextureCreationState& state) {
    texture.descriptor_state.creation_state = state;
    render_texture_apply_descriptor_state(texture, texture.descriptor_state);
}

void release_source_data(RenderTexture2D& texture) {
    render_texture_mip_chain_release_source_data(texture.mip_chain);
}

RenderTexture* resolve_resource(
    RenderTexture2D& texture,
    std::int64_t& resource_index) {
    if (texture.linked_resource == nullptr) {
        return &texture;
    }
    resource_index = texture.linked_resource_index;
    return static_cast<RenderTexture*>(texture.linked_resource);
}

bool reserved_predicate(const RenderTexture2D&) {
    return false;
}

RenderTexture2DDispatch kBaseTexture2DDispatch{
    render_texture_2d_destruct,
    render_texture_2d_delete,
    descriptor_type,
    {},
    mip_level_count,
    source_size,
    apply_creation_state,
    release_source_data,
    resolve_resource,
    nullptr,
    reserved_predicate,
};

static_assert(offsetof(RenderTexture2DDispatch, mip_level_count) == 80);
static_assert(offsetof(RenderTexture2DDispatch, release_source_data) == 104);
static_assert(offsetof(RenderTexture2DDispatch, resolve_resource) == 112);
static_assert(offsetof(RenderTexture2DDispatch, initialize_backend) == 120);
static_assert(sizeof(RenderTexture2DDispatch) == 17 * sizeof(void*));

void set_base_dispatch(RenderTexture2D& texture) {
    texture.implementation = &kBaseTexture2DDispatch;
}

}  // namespace

// Reconstructed from eboot.elf at 0x690330.
void render_texture_2d_descriptor_construct(
    RenderTexture2DDescriptor& descriptor) {
    render_texture_descriptor_construct(descriptor.texture_state);
    render_texture_mip_chain_descriptor_construct(descriptor.mip_chain);
    descriptor.texture_state.descriptor_type = 1;
}

// Reconstructed from eboot.elf at 0x6900B0.
void render_texture_2d_construct(
    RenderTexture2D& texture,
    const RenderTexture2DDescriptor& descriptor) {
    render_texture_construct(texture);
    set_base_dispatch(texture);

    texture.descriptor_state = descriptor.texture_state;
    render_texture_mip_chain_construct(
        texture.mip_chain,
        descriptor.mip_chain,
        render_texture_descriptor_has_source_data(descriptor.texture_state));

    texture.linked_resource = nullptr;
    texture.linked_resource_index = -1;
    texture.resource_index = 0;
    texture.descriptor_state.data_format =
        descriptor.mip_chain.fields.data_format;
    texture.descriptor_state.width = descriptor.mip_chain.fields.width;
    texture.descriptor_state.height = descriptor.mip_chain.fields.height;
    texture.descriptor_state.depth = descriptor.mip_chain.fields.depth;
    render_texture_apply_descriptor_state(
        texture, texture.descriptor_state);
}

// Reconstructed from eboot.elf at 0x68FE80.
RenderTexture2D* render_create_texture_2d(
    RenderTexture2DDescriptor& descriptor) {
    render_texture_resolve_descriptor_fields(
        descriptor.texture_state, 1, -1);
    auto& factory = *render_system_factory(*render_system_instance());
    auto* texture = render_factory_create_texture_2d(factory, descriptor);
    render_texture_initialize_backend(*texture, nullptr);
    return texture;
}

// Reconstructed from eboot.elf at 0x6901D0.
void render_texture_2d_destruct(RenderTexture2D& texture) {
    set_base_dispatch(texture);
    render_texture_mip_chain_destruct(texture.mip_chain);
    render_texture_destruct(texture);
}

// Reconstructed from eboot.elf at 0x690210.
void render_texture_2d_delete(RenderTexture2D& texture) {
    render_texture_2d_destruct(texture);
    MemFree(&texture);
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
