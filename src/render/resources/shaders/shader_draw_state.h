#pragma once

#include <cstddef>
#include <cstdint>

class RndComputeBuffer;
class RndShaderCBuffer;

class RndTextureBase;

class RndContext;

namespace rb4 {

struct RenderShaderParameterBinding;

// Helpers for the patterns that built-in pass draw functions inline.

// Stamps the resource with the frame epoch, raises the context's input-slot
// limit for the stage past the slot, and binds it.
void render_shader_bind_texture(
    RndContext& context,
    RndTextureBase& texture,
    std::uint32_t stage,
    std::uint64_t slot,
    std::uint32_t flags);
void render_shader_bind_buffer(
    RndContext& context,
    RndComputeBuffer& buffer,
    std::uint32_t stage,
    std::uint64_t slot,
    std::uint32_t flags);

// Stamps the texture with the frame epoch, raises the context's texture-slot
// limit past the slot, and binds the texture to the pixel stage. Null
// textures are skipped.
void render_shader_bind_pixel_texture(
    RndContext& context,
    RndTextureBase* texture,
    std::uint64_t slot,
    std::uint32_t flags = 0);

// Selects the context's smallest per-draw constant buffer that holds the
// given number of 16-byte elements.
RndShaderCBuffer& render_shader_select_constant_buffer(
    RndContext& context,
    std::uint64_t element_count);

// Address of a constant-block member inside a constant buffer's staging data.
void* render_shader_constant_member(
    RndShaderCBuffer& buffer,
    std::uint64_t member_offset);

// Uploads the first element_count elements and binds the buffer.
void render_shader_commit_constant_buffer(
    RndShaderCBuffer& buffer,
    RndContext& context,
    std::uint64_t element_count);

// Packs a stage-local parameter value into a permutation key. The shifted
// field is sign-extended, as every recovered draw function does.
std::uint64_t render_shader_parameter_binding_apply(
    std::uint64_t key,
    const RenderShaderParameterBinding& binding,
    std::uint32_t value);

// Draw used by single-texture graphics shaders: binds the texture to the pixel
// stage at the slot and binds the shader with default permutation keys.
void render_shader_draw_with_pixel_texture(
    void* shader,
    RndContext& context,
    RndTextureBase& texture,
    std::uint64_t slot);

}  // namespace rb4
