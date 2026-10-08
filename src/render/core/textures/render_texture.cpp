#include "render/core/textures/render_texture.h"

#include "render/core/textures/render_texture_adapters.h"

namespace rb4 {

namespace {

struct RenderTextureDispatch {
    void* reserved_destruct;
    void (*release_dynamic)(RenderTexture& texture);
    std::int32_t (*descriptor_type)(const RenderTexture& texture);
    std::uint8_t reserved_24[10 * sizeof(void*)];
    void (*update_gpu_data)(RenderTexture& texture);
    void* reserved_112;
    void (*initialize_backend)(
        RenderTexture& texture,
        const RenderTexture* reusable_texture);
};

const RenderTextureDispatch& dispatch(const RenderTexture& texture) {
    return *static_cast<const RenderTextureDispatch*>(
        texture.implementation);
}

static_assert(offsetof(RenderTextureDispatch, update_gpu_data) == 104);
static_assert(offsetof(RenderTextureDispatch, descriptor_type) == 16);
static_assert(offsetof(RenderTextureDispatch, release_dynamic) == 8);
static_assert(offsetof(RenderTextureDispatch, initialize_backend) == 120);

}  // namespace

// Reconstructed from eboot.elf at 0x69B930.
void render_texture_descriptor_construct(
    RenderTextureDescriptorState& descriptor) {
    descriptor = {};
    descriptor.descriptor_type = -1;
    descriptor.data_format = -1;
    descriptor.attachment_index = -1;
}

// Reconstructed from eboot.elf at 0x69B990.
bool render_texture_descriptor_has_source_data(
    const RenderTextureDescriptorState& descriptor) {
    return descriptor.backend_initialized || (descriptor.flags & 5U) != 0;
}

// Reconstructed from eboot.elf at 0x69B6E0.
void render_texture_construct(RenderTexture& texture) {
    render_texture_set_base_dispatch(texture);
    texture.frame_stamp = -1;
    texture.descriptor_type = -1;
    for (auto& value : texture.descriptor_prefix) {
        value = 0;
    }
    texture.usage_type = RenderTextureUsage::kDefault;
    for (auto& value : texture.descriptor_suffix) {
        value = 0;
    }
    texture.address_mode = 0;
    texture.filter_mode = 0;
    texture.flags = 0;
    texture.data_format = -1;
    texture.width = 0;
    texture.height = 0;
    texture.depth = 0;
    texture.source_data = nullptr;
    texture.source_size = 0;
    texture.backend_initialized = false;
    texture.bindless_index = -1;
    texture.resource_flags = 0;
    texture.name = nullptr;
    texture.resource_index = -1;
}

// Reconstructed from eboot.elf at 0x69B770.
void render_texture_destruct(RenderTexture&) {
}

// Reconstructed from eboot.elf at 0x69B780.
void render_texture_delete(RenderTexture& texture) {
    render_texture_destruct(texture);
    render_delete_texture_storage(texture);
}

void render_texture_release_dynamic(RenderTexture& texture) {
    dispatch(texture).release_dynamic(texture);
}

// Reconstructed from eboot.elf at 0x69B7A0.
void render_texture_initialize_backend(
    RenderTexture& texture,
    const RenderTexture* reusable_texture) {
    if (render_texture_backend_initialization_disabled()) {
        return;
    }

    const auto& methods = dispatch(texture);
    methods.initialize_backend(texture, reusable_texture);
    if (texture.source_data == nullptr && (texture.flags & 5U) == 0) {
        methods.update_gpu_data(texture);
    }
}

std::int32_t render_texture_runtime_descriptor_type(
    const RenderTexture& texture) {
    return dispatch(texture).descriptor_type(texture);
}

// Reconstructed from eboot.elf at 0x50CE00.
std::int32_t render_texture_default_address_mode(
    std::uint32_t resource_kind) {
    constexpr std::uint64_t kAddressModeOneKinds = 0x1F80400E0ULL;
    if (resource_kind > 32) {
        return -1;
    }
    return (kAddressModeOneKinds & (1ULL << resource_kind)) != 0
        ? 1
        : -1;
}

// Reconstructed from eboot.elf at 0x50CE30.
std::int32_t render_texture_default_filter_mode(
    std::uint32_t resource_kind) {
    constexpr std::uint64_t kFilterModeTwoKinds = 0x178040060ULL;
    constexpr std::uint64_t kFilterModeOneKinds = 0x80000080ULL;
    if (resource_kind > 32) {
        return -1;
    }
    const auto kind_bit = 1ULL << resource_kind;
    if ((kFilterModeTwoKinds & kind_bit) != 0) {
        return 2;
    }
    return (kFilterModeOneKinds & kind_bit) != 0 ? 1 : -1;
}

}  // namespace rb4
