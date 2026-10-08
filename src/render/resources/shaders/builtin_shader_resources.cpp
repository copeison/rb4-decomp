#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstdint>

#include "render/resources/shaders/builtin_shader_adapters.h"
#include "render/resources/shaders/primary_shader_resource.h"

namespace rb4 {

namespace {

std::int64_t* construct_shader(
    void* storage,
    void (*install_dispatch)(void*)) {
    render_primary_shader_construct(
        *static_cast<RenderPrimaryShaderResource*>(storage));
    install_dispatch(storage);
    auto* bytes = static_cast<std::uint8_t*>(storage);
    return reinterpret_cast<std::int64_t*>(bytes + 288);
}

}  // namespace

// Reconstructed from eboot.elf at 0x6364F0.
void render_fxaa_shader_construct(void* shader) {
    auto* fields = construct_shader(
        shader, render_fxaa_shader_install_dispatch);
    fields[0] = -1;
    fields[1] = 0;
    fields[2] = -1;
}

// Reconstructed from eboot.elf at 0x6F3330.
void render_dof_sprite_shader_construct(void* shader) {
    auto* fields = construct_shader(
        shader, render_dof_sprite_shader_install_dispatch);
    fields[0] = -1;
    fields[1] = -1;
}

// Reconstructed from eboot.elf at 0x63DC90.
void render_display_shading_mode_shader_construct(void* shader) {
    auto* fields = construct_shader(
        shader, render_display_shading_mode_shader_install_dispatch);
    fields[0] = -1;
    fields[1] = 0;
    fields[2] = -1;
}

// Reconstructed from eboot.elf at 0x6F4270.
void render_display_sphere_map_shader_construct(void* shader) {
    auto* fields = construct_shader(
        shader, render_display_sphere_map_shader_install_dispatch);
    fields[0] = -1;
}

// Reconstructed from eboot.elf at 0x63EF70.
void render_linearize_depth_shader_construct(void* shader) {
    auto* fields = construct_shader(
        shader, render_linearize_depth_shader_install_dispatch);
    fields[0] = -1;
}

// Reconstructed from eboot.elf at 0x642360.
void render_refine_scene_mask_shader_construct(void* shader) {
    auto* fields = construct_shader(
        shader, render_refine_scene_mask_shader_install_dispatch);
    fields[0] = -1;
}

// Reconstructed from eboot.elf at 0x644FC0.
void render_stencil_scene_mask_shader_construct(void* shader) {
    auto* fields = construct_shader(
        shader, render_stencil_scene_mask_shader_install_dispatch);
    fields[0] = -1;
    fields[1] = -1;
    fields[2] = 0;
}

// Reconstructed from eboot.elf at 0x6452E0.
void render_test_pattern_shader_construct(void* shader) {
    auto* fields = construct_shader(
        shader, render_test_pattern_shader_install_dispatch);
    fields[0] = -1;
    fields[1] = -1;
    fields[2] = -1;
    fields[3] = 0;
}

}  // namespace rb4
