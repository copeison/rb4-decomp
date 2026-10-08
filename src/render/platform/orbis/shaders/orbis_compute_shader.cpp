#include "render/platform/orbis/shaders/orbis_shader.h"

#include "render/platform/orbis/shaders/orbis_shader_adapters.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x8E3D50.
void orbis_compute_shader_destruct(OrbisShader& shader) {
    shader_destruct(shader);
}

// Reconstructed from eboot.elf at 0x8E3D80.
void orbis_compute_shader_delete(OrbisShader& shader) {
    orbis_compute_shader_destruct(shader);
    render_delete_shader(shader);
}

// Reconstructed from eboot.elf at 0x8E3DC0.
bool orbis_compute_shader_initialize(
    OrbisShader& shader,
    const OrbisShaderBinary& binary) {
    return orbis_compute_shader_load_binary(shader, binary);
}

// Reconstructed from eboot.elf at 0x8E3F40.
void orbis_compute_shader_bind(
    const OrbisShader& shader,
    OrbisRenderContext& context) {
    orbis_compute_shader_bind_backend(shader, context);
}

// Reconstructed from eboot.elf at 0x8E4030.
void orbis_compute_shader_release_backend(OrbisShader& shader) {
    orbis_compute_shader_release_allocations(shader);
}

// Reconstructed from eboot.elf at 0x8E4070.
RenderShaderStage orbis_compute_shader_stage() {
    return RenderShaderStage::kCompute;
}

}  // namespace rb4
