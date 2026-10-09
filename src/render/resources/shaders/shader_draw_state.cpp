#include "render/resources/shaders/shader_draw_state.h"

#include "render/core/buffers/render_constant_buffer.h"
#include "render/core/context/render_context.h"
#include "render/core/shaders/render_shader.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/system/render_system_state.h"
#include "render/core/textures/render_texture.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

constexpr std::uint64_t kSmallestConstantBufferElements = 16;
constexpr std::size_t kConstantElementSize = 16;

}  // namespace

void render_shader_bind_pixel_texture(
    RenderContext& context,
    RenderTexture* texture,
    std::uint64_t slot) {
    if (texture == nullptr) {
        return;
    }
    texture->frame_stamp = static_cast<std::int64_t>(
        render_system_core_state(*render_system_instance()).frame_epoch);
    auto& limit = render_context_texture_slot_limit(context);
    if (limit < slot + 1) {
        limit = slot + 1;
    }
    // The original leaves the border-color argument unspecified; it is only
    // consulted for border address modes, which these passes never use.
    render_texture_bind(
        *texture,
        context,
        RenderShaderStage::kPixel,
        static_cast<std::uint32_t>(slot),
        0,
        nullptr);
}

RenderConstantBuffer& render_shader_select_constant_buffer(
    RenderContext& context,
    std::uint64_t element_count) {
    std::size_t size_class = 0;
    if (element_count > kSmallestConstantBufferElements) {
        auto capacity = kSmallestConstantBufferElements;
        do {
            capacity *= 2;
            ++size_class;
        } while (capacity < element_count);
    }
    return *render_context_constant_buffer(context, size_class);
}

void* render_shader_constant_member(
    RenderConstantBuffer& buffer,
    std::uint64_t member_offset) {
    return static_cast<std::uint8_t*>(buffer.data) +
        member_offset * kConstantElementSize;
}

void render_shader_commit_constant_buffer(
    RenderConstantBuffer& buffer,
    RenderContext& context,
    std::uint64_t element_count) {
    buffer.upload_pending = true;
    render_constant_buffer_update_range(buffer, context, 0, element_count);
    buffer.upload_pending = false;
    render_constant_buffer_bind(buffer, context);
}

std::uint64_t render_shader_parameter_binding_apply(
    std::uint64_t key,
    const RenderShaderParameterBinding& binding,
    std::uint32_t value) {
    const auto field = static_cast<std::int32_t>(
        (value - binding.first_value) << binding.bit_offset);
    return (key & ~static_cast<std::uint64_t>(binding.shifted_mask)) |
        static_cast<std::uint64_t>(static_cast<std::int64_t>(field));
}

}  // namespace rb4
