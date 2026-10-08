#include "render/platform/orbis/shaders/orbis_shader.h"

#include "render/platform/orbis/shaders/orbis_shader_adapters.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x8E4410.
void orbis_pixel_shader_destruct(OrbisShader& shader) {
    render_shader_release(shader);
    render_shader_destruct(shader);
}

// Reconstructed from eboot.elf at 0x8E4440.
void orbis_pixel_shader_delete(OrbisShader& shader) {
    orbis_pixel_shader_destruct(shader);
    render_shader_delete(shader);
}

// Reconstructed from eboot.elf at 0x8E4480.
bool orbis_pixel_shader_initialize(
    OrbisShader& shader,
    const OrbisShaderBinary& binary) {
    return orbis_pixel_shader_load_binary(shader, binary);
}

// Reconstructed from eboot.elf at 0x8E4600.
void orbis_pixel_shader_bind(
    const OrbisShader& shader,
    OrbisRenderContext& context) {
    orbis_pixel_shader_bind_backend(shader, context);
}

// Reconstructed from eboot.elf at 0x8E4670.
void orbis_pixel_shader_release_backend(OrbisShader& shader) {
    orbis_pixel_shader_release_allocations(shader);
}

// Reconstructed from eboot.elf at 0x8E46B0.
RenderShaderStage orbis_pixel_shader_stage() {
    return RenderShaderStage::kPixel;
}

}  // namespace rb4
