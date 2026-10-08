#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstddef>
#include <cstdint>

#include "render/resources/shaders/builtin_shader_adapters.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_parameter_registry.h"

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

std::uint8_t* construct_parameterized_shader(
    void* storage,
    void (*install_dispatch)(void*),
    std::size_t binding_count) {
    render_primary_shader_construct(
        *static_cast<RenderPrimaryShaderResource*>(storage));
    install_dispatch(storage);
    auto* bytes = static_cast<std::uint8_t*>(storage);
    for (std::size_t index = 0; index < binding_count; ++index) {
        auto* binding = reinterpret_cast<RenderShaderParameterBinding*>(
            bytes + 288 + index * sizeof(RenderShaderParameterBinding));
        *binding = {};
    }
    return bytes;
}

std::int64_t& shader_field(void* shader, std::size_t offset) {
    auto* bytes = static_cast<std::uint8_t*>(shader);
    return *reinterpret_cast<std::int64_t*>(bytes + offset);
}

}  // namespace

// Reconstructed from eboot.elf at 0x5F4980.
void render_bink_convert_shader_construct(void* shader) {
    construct_parameterized_shader(
        shader, render_bink_convert_shader_install_dispatch, 1);
    for (std::size_t offset = 312; offset <= 376; offset += 8) {
        shader_field(shader, offset) = -1;
    }
    shader_field(shader, 384) = 0;
}

// Reconstructed from eboot.elf at 0x634640.
void render_bloom_shader_construct(void* shader) {
    construct_parameterized_shader(
        shader, render_bloom_shader_install_dispatch, 2);
    shader_field(shader, 328) = -1;
    shader_field(shader, 336) = -1;
    shader_field(shader, 344) = 0;
    shader_field(shader, 352) = -1;
    shader_field(shader, 360) = -1;
    shader_field(shader, 368) = -1;
}

// Reconstructed from eboot.elf at 0x634AE0.
void render_blur_shader_construct(void* shader) {
    construct_parameterized_shader(
        shader, render_blur_shader_install_dispatch, 6);
    for (std::size_t offset = 408; offset <= 432; offset += 8) {
        shader_field(shader, offset) = -1;
    }
    shader_field(shader, 440) = 0;
    shader_field(shader, 448) = -1;
    shader_field(shader, 456) = -1;
    shader_field(shader, 472) = -1;
}

// Reconstructed from eboot.elf at 0x6367A0.
void render_output_conversion_shader_construct(void* shader) {
    construct_parameterized_shader(
        shader, render_output_conversion_shader_install_dispatch, 3);
    shader_field(shader, 352) = -1;
    shader_field(shader, 360) = 0;
    shader_field(shader, 368) = -1;
    shader_field(shader, 376) = -1;
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

// Reconstructed from eboot.elf at 0x642500.
void render_test_shader_construct(void* shader) {
    construct_parameterized_shader(
        shader, render_test_shader_install_dispatch, 2);
    shader_field(shader, 328) = -1;
    shader_field(shader, 336) = 0;
}

}  // namespace rb4
