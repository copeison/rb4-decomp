#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderConstantBuffer;
struct RenderContext;
struct RenderShaderParameterBinding;
struct RenderTexture;

// Helpers for the patterns that built-in pass draw functions inline.

// Stamps the texture with the frame epoch, raises the context's texture-slot
// limit past the slot, and binds the texture to the pixel stage. Null
// textures are skipped.
void render_shader_bind_pixel_texture(
    RenderContext& context,
    RenderTexture* texture,
    std::uint64_t slot);

// Selects the context's smallest per-draw constant buffer that holds the
// given number of 16-byte elements.
RenderConstantBuffer& render_shader_select_constant_buffer(
    RenderContext& context,
    std::uint64_t element_count);

// Address of a constant-block member inside a constant buffer's staging data.
void* render_shader_constant_member(
    RenderConstantBuffer& buffer,
    std::uint64_t member_offset);

// Uploads the first element_count elements and binds the buffer.
void render_shader_commit_constant_buffer(
    RenderConstantBuffer& buffer,
    RenderContext& context,
    std::uint64_t element_count);

// Packs a stage-local parameter value into a permutation key. The shifted
// field is sign-extended, as every recovered draw function does.
std::uint64_t render_shader_parameter_binding_apply(
    std::uint64_t key,
    const RenderShaderParameterBinding& binding,
    std::uint32_t value);

}  // namespace rb4
