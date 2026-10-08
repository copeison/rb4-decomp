#include "render/platform/orbis/shaders/orbis_shader.h"

#include "render/platform/orbis/shaders/orbis_shader_adapters.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x8E40D0.
void orbis_geometry_shader_destruct(OrbisShader& shader) {
    render_shader_release(shader);
    render_shader_destruct(shader);
}

// Reconstructed from eboot.elf at 0x8E4100.
void orbis_geometry_shader_delete(OrbisShader& shader) {
    orbis_geometry_shader_destruct(shader);
    render_shader_delete(shader);
}

// Reconstructed from eboot.elf at 0x8E4140.
bool orbis_geometry_shader_initialize(
    OrbisShader& shader,
    const OrbisShaderBinary& binary) {
    return orbis_geometry_shader_load_binary(shader, binary);
}

// Reconstructed from eboot.elf at 0x8E4330.
void orbis_geometry_shader_bind(
    const OrbisShader& shader,
    OrbisRenderContext& context) {
    orbis_geometry_shader_bind_backend(shader, context);
}

// Reconstructed from eboot.elf at 0x8E4350.
void orbis_geometry_shader_release_backend(OrbisShader& shader) {
    orbis_geometry_shader_release_allocations(shader);
}

// Reconstructed from eboot.elf at 0x8E43A0.
RenderShaderStage orbis_geometry_shader_stage() {
    return RenderShaderStage::kGeometry;
}

}  // namespace rb4
