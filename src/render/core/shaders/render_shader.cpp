#include "render/core/shaders/render_shader.h"

#include "render/core/shaders/render_shader_adapters.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x642250.
RenderShader* render_create_shader(RenderShaderStage stage) {
    return render_system_create_shader(stage);
}

// Reconstructed from eboot.elf at 0x642270.
void render_shader_construct(RenderShader& shader) {
    render_shader_set_base_dispatch(shader);
    shader.owner = nullptr;
    shader.initialized = false;
    shader.variant_index = -1;
    shader.metadata = nullptr;
}

// Reconstructed from eboot.elf at 0x6422A0.
void render_shader_destruct(RenderShader&) {
}

// Reconstructed from eboot.elf at 0x6422B0.
void render_shader_delete(RenderShader& shader) {
    render_delete_shader_storage(shader);
}

// Reconstructed from eboot.elf at 0x6422C0.
bool render_shader_initialize(
    RenderShader& shader,
    void* owner,
    const RenderShaderBinary* binary,
    void* metadata) {
    shader.owner = owner;
    shader.metadata = metadata;
    render_shader_release(shader);

    if (binary == nullptr) {
        return true;
    }

    shader.initialized = render_shader_initialize_backend(shader, *binary);
    return shader.initialized;
}

// Reconstructed from eboot.elf at 0x642310.
void render_shader_release(RenderShader& shader) {
    if (!shader.initialized) {
        return;
    }

    render_shader_release_backend(shader);
    shader.initialized = false;
}

}  // namespace rb4
