#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstddef>
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

std::int64_t* construct_compute_shader(
    void* storage,
    void (*install_dispatch)(void*)) {
    render_primary_shader_construct(
        *static_cast<RenderPrimaryShaderResource*>(storage));
    render_compute_shader_install_dispatch(storage);
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

// Reconstructed from eboot.elf at 0x62E1C0.
void render_blur_classify_compute_shader_construct(void* shader) {
    auto* fields = construct_compute_shader(
        shader, render_blur_classify_compute_shader_install_dispatch);
    for (std::size_t index = 0; index < 6; ++index) {
        fields[index] = -1;
    }
}

// Reconstructed from eboot.elf at 0x636DE0.
void render_calc_depth_range_compute_shader_construct(void* shader) {
    auto* fields = construct_compute_shader(
        shader, render_calc_depth_range_compute_shader_install_dispatch);
    fields[0] = -1;
    fields[1] = -1;
    fields[2] = -1;
    fields[3] = 0;
}

// Reconstructed from eboot.elf at 0x6F2B40.
void render_dof_disc_blur_compute_shader_construct(void* shader) {
    auto* fields = construct_shader(
        shader, render_dof_disc_blur_compute_shader_install_dispatch);
    for (std::size_t index = 0; index < 10; ++index) {
        fields[index] = -1;
    }
    fields[10] = 0;
}

// Reconstructed from eboot.elf at 0x6D7130.
void render_ssao_compute_shader_construct(void* shader) {
    auto* fields = construct_shader(
        shader, render_ssao_compute_shader_install_dispatch);
    for (std::size_t index = 0; index < 5; ++index) {
        fields[index] = -1;
    }
    fields[6] = -1;
    fields[7] = -1;
}

// Reconstructed from eboot.elf at 0x450490.
void render_cmaa_edge_detect_compute_shader_construct(void* shader) {
    auto* fields = construct_shader(
        shader, render_cmaa_edge_detect_compute_shader_install_dispatch);
    for (std::size_t index = 0; index < 4; ++index) {
        fields[index] = -1;
    }
    fields[4] = 0;
}

// Reconstructed from eboot.elf at 0x4508B0.
void render_cmaa_edge_prune_compute_shader_construct(void* shader) {
    auto* fields = construct_shader(
        shader, render_cmaa_edge_prune_compute_shader_install_dispatch);
    fields[0] = -1;
    fields[1] = -1;
    fields[2] = -1;
    fields[3] = 0;
}

// Reconstructed from eboot.elf at 0x451030.
void render_cmaa_shape_fit_compute_shader_construct(void* shader) {
    auto* fields = construct_shader(
        shader, render_cmaa_shape_fit_compute_shader_install_dispatch);
    for (std::size_t index = 0; index < 4; ++index) {
        fields[index] = -1;
    }
    fields[4] = 0;
}

// Reconstructed from eboot.elf at 0x450BF0.
void render_cmaa_final_process_compute_shader_construct(void* shader) {
    auto* fields = construct_shader(
        shader, render_cmaa_final_process_compute_shader_install_dispatch);
    for (std::size_t index = 0; index < 4; ++index) {
        fields[index] = -1;
    }
    fields[4] = 0;
}

// Reconstructed from eboot.elf at 0x62E660.
void render_signed_distance_compute_shader_construct(void* shader) {
    auto* fields = construct_compute_shader(
        shader, render_signed_distance_compute_shader_install_dispatch);
    for (std::size_t index = 0; index < 7; ++index) {
        fields[index] = -1;
    }
}

// Reconstructed from eboot.elf at 0x62EAF0.
void render_signed_distance_classify_compute_shader_construct(void* shader) {
    auto* fields = construct_compute_shader(
        shader,
        render_signed_distance_classify_compute_shader_install_dispatch);
    for (std::size_t index = 0; index < 6; ++index) {
        fields[index] = -1;
    }
}

}  // namespace rb4
