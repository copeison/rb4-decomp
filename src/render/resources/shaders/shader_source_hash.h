#pragma once

#include <cstdint>

namespace rb4 {

enum class RenderShaderConstantType : std::uint32_t;

// Generated shader source is never materialized at runtime; it is streamed
// into an FNV-1a hashing text stream (dispatch 0x1911CD8). These helpers
// reproduce that stream's text and number output.
constexpr std::uint32_t kShaderSourceHashBasis = 0x811C9DC5U;

void render_shader_source_hash_append(std::uint32_t& hash, const char* text);
void render_shader_source_hash_append_unsigned(
    std::uint32_t& hash,
    std::uint64_t value);
void render_shader_source_hash_append_signed(
    std::uint32_t& hash,
    std::int32_t value);
void render_shader_source_hash_append_byte(
    std::uint32_t& hash,
    std::uint8_t value);

const char* render_shader_constant_type_name(std::uint32_t type);
std::uint32_t render_shader_constant_base_type(std::uint32_t type);

}  // namespace rb4
