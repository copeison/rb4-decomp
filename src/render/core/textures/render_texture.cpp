#include "render/core/textures/render_texture.h"

#include "render/core/textures/render_texture_adapters.h"

namespace rb4 {

namespace {

struct RenderTextureDispatch {
    std::uint8_t reserved_0[13 * sizeof(void*)];
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
static_assert(offsetof(RenderTextureDispatch, initialize_backend) == 120);

}  // namespace

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

}  // namespace rb4
