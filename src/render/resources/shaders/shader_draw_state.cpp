#include "render/resources/shaders/shader_draw_state.h"

#include "render/buffers/RndComputeBuffer.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/core/context/render_context.h"
#include "render/shaders/RndShaderProgram.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/system/render_system_state.h"
#include "render/core/textures/render_texture.h"
#include "render/resources/shaders/compiled_shader_objects.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

// Draws select a resource through the slot for the requested stage. The
// compute slot's extra argument is left unspecified by the callers in the
// binary; zero is passed here.
void select_resource(
    RndShaderResource& resource,
    RenderContext& context,
    RndShaderProgramType type,
    std::uint64_t slot,
    std::uint32_t flags) {
    switch (type) {
    case kShaderProgramVertex:
        resource._SelectForVSImpl(context, slot, flags);
        break;
    case kShaderProgramHull:
        resource._SelectForHSImpl(context, slot, flags);
        break;
    case kShaderProgramDomain:
        resource._SelectForDSImpl(context, slot, flags);
        break;
    case kShaderProgramGeometry:
        resource._SelectForGSImpl(context, slot, flags);
        break;
    case kShaderProgramPixel:
        resource._SelectForPSImpl(context, slot, flags);
        break;
    case kShaderProgramCompute:
        resource._SelectForCSImpl(context, slot, flags, 0);
        break;
    }
}

constexpr std::uint64_t kSmallestConstantBufferElements = 16;
constexpr std::size_t kConstantElementSize = 16;

std::int64_t current_frame_epoch() {
    return static_cast<std::int64_t>(
        render_system_core_state(*render_system_instance()).frame_epoch);
}

void raise_limit(std::uint64_t& limit, std::uint64_t slot) {
    if (limit < slot + 1) {
        limit = slot + 1;
    }
}

}  // namespace

void render_shader_bind_texture(
    RenderContext& context,
    RenderTexture& texture,
    std::uint32_t stage,
    std::uint64_t slot,
    std::uint32_t flags) {
    texture.frame_stamp = current_frame_epoch();
    raise_limit(render_context_input_slot_limit(context, stage), slot);
    // The original leaves the border-color argument unspecified (a stale
    // register); backends consult it only for border address modes.
    render_texture_bind(
        texture,
        context,
        static_cast<RndShaderProgramType>(stage),
        static_cast<std::uint32_t>(slot),
        flags,
        nullptr);
}

void render_shader_bind_buffer(
    RenderContext& context,
    RndComputeBuffer& buffer,
    std::uint32_t stage,
    std::uint64_t slot,
    std::uint32_t flags) {
    buffer.mFrameStamp = current_frame_epoch();
    raise_limit(render_context_input_slot_limit(context, stage), slot);
    select_resource(
        buffer, context, static_cast<RndShaderProgramType>(stage), slot, flags);
}

void render_shader_bind_pixel_texture(
    RenderContext& context,
    RenderTexture* texture,
    std::uint64_t slot,
    std::uint32_t flags) {
    constexpr std::uint32_t kPixelStage = 4;
    if (texture != nullptr) {
        render_shader_bind_texture(context, *texture, kPixelStage, slot, flags);
    }
}

RndShaderCBuffer& render_shader_select_constant_buffer(
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
    RndShaderCBuffer& buffer,
    std::uint64_t member_offset) {
    return static_cast<std::uint8_t*>(buffer.mData) +
        member_offset * kConstantElementSize;
}

void render_shader_commit_constant_buffer(
    RndShaderCBuffer& buffer,
    RenderContext& context,
    std::uint64_t element_count) {
    buffer.mSyncPending = true;
    buffer._SyncImpl(context, 0, element_count);
    buffer.mSyncPending = false;
    buffer._SelectImpl(context);
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

void render_shader_draw_with_pixel_texture(
    void* shader,
    RenderContext& context,
    RenderTexture& texture,
    std::uint64_t slot) {
    render_shader_bind_pixel_texture(context, &texture, slot);
    std::uint64_t keys[kRenderShaderProgramKeyCount] = {};
    render_primary_shader_bind(
        *static_cast<RenderPrimaryShaderResource*>(shader), context, keys);
}

}  // namespace rb4
