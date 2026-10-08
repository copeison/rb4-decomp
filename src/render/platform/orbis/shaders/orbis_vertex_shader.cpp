#include "render/platform/orbis/shaders/orbis_shader.h"

#include "render/platform/orbis/shaders/orbis_shader_adapters.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x8E4720.
void orbis_vertex_shader_destruct(OrbisShader& shader) {
    render_shader_release(shader);
    render_shader_destruct(shader);
}

// Reconstructed from eboot.elf at 0x8E4750.
void orbis_vertex_shader_delete(OrbisShader& shader) {
    orbis_vertex_shader_destruct(shader);
    render_shader_delete(shader);
}

// Reconstructed from eboot.elf at 0x8E4790.
bool orbis_vertex_shader_initialize(
    OrbisShader& shader,
    const OrbisShaderBinary& binary) {
    return orbis_vertex_shader_load_binary(shader, binary);
}

// Reconstructed from eboot.elf at 0x8E4D80.
void orbis_vertex_shader_bind(
    const OrbisShader& shader,
    OrbisRenderContext& context) {
    orbis_vertex_shader_bind_backend(shader, context);
}

// Reconstructed from eboot.elf at 0x8E4ED0.
void orbis_vertex_shader_release_backend(OrbisShader& shader) {
    orbis_vertex_shader_release_allocations(shader);
}

// Reconstructed from eboot.elf at 0x8E4F30.
RenderShaderStage orbis_vertex_shader_stage() {
    return RenderShaderStage::kVertex;
}

}  // namespace rb4
