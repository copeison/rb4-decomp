#include "render/core/render_texture.h"

#include "render/core/render_texture_adapters.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x69B6E0.
void render_texture_construct(RenderTexture& texture) {
    render_texture_set_base_dispatch(texture);
    texture.frame_stamp = -1;
    texture.descriptor_type = -1;
    for (auto& value : texture.descriptor_prefix) {
        value = 0;
    }
    texture.usage_type = 0;
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

}  // namespace rb4
