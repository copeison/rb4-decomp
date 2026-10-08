// Constructs the supported Orbis shader object for a renderer stage.
// Reconstructed from eboot.elf at 0x8D8B30.
OrbisShader* orbis_create_shader(RenderShaderStage stage) {
    switch (stage) {
    case kShaderStageVertex:
        return construct_vertex_shader(render_allocate(104));
    case kShaderStageGeometry:
        return construct_geometry_shader(render_allocate(72));
    case kShaderStagePixel:
        return construct_pixel_shader(render_allocate(64));
    case kShaderStageCompute:
        return construct_compute_shader(render_allocate(64));
    default:
        return 0;
    }
}
