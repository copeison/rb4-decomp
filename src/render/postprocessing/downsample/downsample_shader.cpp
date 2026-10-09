#include "render/postprocessing/downsample/downsample_shader.h"

#include <cstddef>
#include <cstdint>
#include <immintrin.h>

#include "os/memory/MemMgr.h"
#include "utl/text/Symbol.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"
#include "render/resources/shaders/builtin_shader_resources.h"
#include "render/resources/shaders/primary_shader_dispatch.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_draw_state.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

struct DownsampleShaderDispatch {
    void (*destruct)(void* shader);
    void (*delete_resource)(void* shader);
    const void* (*source_identifier)(void* shader);
    void* (*backend_path)(void* shader);
    void (*initialize_support_objects)(
        void* shader,
        RenderShaderConstantRegistry* constants,
        RenderShaderParameterRegistrySet* parameters,
        RenderShaderConstantBlock* constant_block,
        RenderShaderBackendState* backend_state);
    std::int32_t (*mode)(void* shader);
    std::int32_t (*variant)(void* shader);
    bool (*validate_permutation)(
        void* shader,
        std::uint32_t stage,
        std::uint64_t key);
    void (*bind_fallback)(void* shader, void* context);
    bool (*supports_render_target_slices)(void* shader);
    bool (*uses_geometry_program)(void* shader);
};

static_assert(sizeof(DownsampleShaderDispatch) == 88);

RenderPrimaryShaderResource& primary_shader(void* shader) {
    return *static_cast<RenderPrimaryShaderResource*>(shader);
}

std::uint8_t* shader_bytes(void* shader) {
    return static_cast<std::uint8_t*>(shader);
}

RenderShaderParameterBinding& parameter_binding(
    void* shader,
    std::size_t index) {
    return *reinterpret_cast<RenderShaderParameterBinding*>(
        shader_bytes(shader) + 288 + index * sizeof(RenderShaderParameterBinding));
}

std::int64_t& shader_field(void* shader, std::size_t offset) {
    return *reinterpret_cast<std::int64_t*>(shader_bytes(shader) + offset);
}

void downsample_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void downsample_shader_delete(void* shader) {
    downsample_shader_destruct(shader);
    MemFree(shader);
}

const void* downsample_shader_source_identifier(void*) {
    return "RndShaderDownsample";
}

void* downsample_shader_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/Downsample.hlsl");
}

void initialize_downsample_shader_support_objects(
    void* shader,
    RenderShaderConstantRegistry* constants,
    RenderShaderParameterRegistrySet* parameters,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    render_shader_constant_registry_add_definition(
        *constants, "HX_DOWNSAMPLE_COLOR_2X", 0);
    render_shader_constant_registry_add_definition(
        *constants, "HX_DOWNSAMPLE_COLOR_4X", 1);
    render_shader_constant_registry_add_definition(
        *constants, "HX_DOWNSAMPLE_BLOOM_2X", 2);
    render_shader_constant_registry_add_definition(
        *constants, "HX_DOWNSAMPLE_BLOOM_4X", 3);

    auto* pixel_parameters = &parameters->registries[4];
    const Symbol color_space("HX_BT709_TO_BT2020");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 0),
        pixel_parameters,
        color_space.Str());
    const Symbol downsample_type("HX_DOWNSAMPLE_TYPE");
    render_shader_parameter_registry_add(
        &parameter_binding(shader, 1),
        pixel_parameters,
        downsample_type.Str(),
        0,
        4);
    const Symbol value_based_bloom("HX_BLOOM_VALUE_BASED");
    render_shader_parameter_registry_add_ternary(
        &parameter_binding(shader, 2),
        pixel_parameters,
        value_based_bloom.Str());

    shader_field(shader, 352) = render_shader_constant_block_add(
        *constant_block, RenderShaderConstantType::vector4, "gTexelOffset");
    shader_field(shader, 360) = static_cast<std::int64_t>(
        constant_block->next_offset);
    shader_field(shader, 368) =
        render_shader_backend_add_graphics_texture_binding(
            *backend_state, "gTexture", "gTexSampler", 1, 12);
}

std::int32_t downsample_shader_mode(void*) {
    return 0;
}

std::int32_t downsample_shader_variant(void*) {
    return 13;
}

DownsampleShaderDispatch kDownsampleShaderDispatch{
    downsample_shader_destruct,
    downsample_shader_delete,
    downsample_shader_source_identifier,
    downsample_shader_backend_path,
    initialize_downsample_shader_support_objects,
    downsample_shader_mode,
    downsample_shader_variant,
    render_primary_shader_validate_permutation,
    render_primary_shader_bind_fallback,
    render_primary_shader_returns_true,
    render_primary_shader_uses_geometry_program,
};

}  // namespace

// Reconstructed from eboot.elf at 0x635FE0.
void render_downsample_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kDownsampleShaderDispatch;
    for (std::size_t index = 0; index < 3; ++index) {
        parameter_binding(shader, index) = {};
    }
    shader_field(shader, 352) = -1;
    shader_field(shader, 360) = 0;
    shader_field(shader, 368) = -1;
}

// Reconstructed from eboot.elf at 0x636080. The texel offset is half a texel
// of the source, (+x, +y, -x, -y), from a refined hardware reciprocal of its
// dimensions; it is written and the source bound only when a source exists.
// The constant buffer is uploaded only while its upload flag is set. HDR10
// output (mode 1) selects the BT.709-to-BT.2020 permutation.
void render_downsample_shader_draw(
    void* shader,
    RndContext& context,
    const RenderDownsampleDrawParameters& parameters) {
    constexpr std::size_t kPixelKey = 3;
    constexpr std::uint32_t kTextureFlags = 2;
    constexpr std::uint32_t kHdr10Output = 1;

    const auto extent = static_cast<std::uint64_t>(shader_field(shader, 360));
    auto& buffer = render_shader_select_constant_buffer(context, extent);
    if (auto* source = parameters.source) {
        const auto width = static_cast<std::int32_t>(source->mBaseDesc.mWidth);
        const auto height = static_cast<std::int32_t>(source->mBaseDesc.mHeight);
        const auto size = _mm_cvtepi32_ps(
            _mm_setr_epi32(width, height, width, height));
        const auto estimate = _mm_rcp_ps(size);
        const auto refined = _mm_add_ps(
            estimate,
            _mm_mul_ps(
                estimate,
                _mm_sub_ps(_mm_set1_ps(1.0F), _mm_mul_ps(size, estimate))));
        const auto offset =
            _mm_mul_ps(refined, _mm_setr_ps(0.5F, 0.5F, -0.5F, -0.5F));
        _mm_storeu_ps(
            static_cast<float*>(render_shader_constant_member(
                buffer, shader_field(shader, 352))),
            offset);
        buffer.mSyncPending = true;
        render_shader_bind_pixel_texture(
            context, source, shader_field(shader, 368), kTextureFlags);
    }
    if (buffer.mSyncPending) {
        buffer._SyncImpl(context, 0, extent);
        buffer.mSyncPending = false;
    }
    buffer._SelectImpl(context);

    const auto hdr_mode = TheRndDevice()->mHdrOutputMode;
    std::uint64_t keys[kRenderShaderProgramKeyCount] = {};
    keys[kPixelKey] = render_shader_parameter_binding_apply(
        0, parameter_binding(shader, 0), hdr_mode == kHdr10Output ? 1U : 0U);
    keys[kPixelKey] = render_shader_parameter_binding_apply(
        keys[kPixelKey],
        parameter_binding(shader, 1),
        static_cast<std::uint32_t>(parameters.downsample_type));
    keys[kPixelKey] = render_shader_parameter_binding_apply(
        keys[kPixelKey],
        parameter_binding(shader, 2),
        parameters.value_based_bloom ? 1U : 0U);
    render_primary_shader_bind(primary_shader(shader), context, keys);
}

}  // namespace rb4
