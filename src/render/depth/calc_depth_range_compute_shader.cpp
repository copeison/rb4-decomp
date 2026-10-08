#include "render/resources/shaders/builtin_shader_resources.h"

#include <cstdint>

#include "core/memory/engine_memory.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/system/render_system_state.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

struct DepthRangeShaderDispatch {
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
};

static_assert(sizeof(DepthRangeShaderDispatch) == 56);

RenderPrimaryShaderResource& primary_shader(void* shader) {
    return *static_cast<RenderPrimaryShaderResource*>(shader);
}

std::int64_t* shader_fields(void* shader) {
    auto* bytes = static_cast<std::uint8_t*>(shader);
    return reinterpret_cast<std::int64_t*>(bytes + 288);
}

void depth_range_shader_destruct(void* shader) {
    render_primary_shader_destruct(primary_shader(shader));
}

void depth_range_shader_delete(void* shader) {
    depth_range_shader_destruct(shader);
    render_release(shader);
}

const void* depth_range_shader_source_identifier(void*) {
    return "RndCShaderCalcDepthRange";
}

void* depth_range_shader_backend_path(void*) {
    return const_cast<char*>(
        "../../system/data/shaders/compute/CalcDepthRange.hlsl");
}

void initialize_depth_range_support_objects(
    void* shader,
    RenderShaderConstantRegistry* constants,
    RenderShaderParameterRegistrySet*,
    RenderShaderConstantBlock* constant_block,
    RenderShaderBackendState* backend_state) {
    const auto& settings = *render_system_core_state(
        *render_system_instance()).settings;
    render_shader_constant_registry_add_definition(
        *constants,
        "HX_TILE_SIZE",
        static_cast<std::int32_t>(settings.light_tile_size));

    auto* fields = shader_fields(shader);
    fields[0] = render_shader_backend_add_texture_binding(
        *backend_state, "gLinearDepthBuffer", "", 1, 5, 12);
    fields[1] = render_shader_backend_add_output_binding(
        *backend_state, "gTiledDepthRangeBuffer", 1, 5, 12);
    fields[2] = render_shader_constant_block_add(
        *constant_block,
        RenderShaderConstantType::vector2,
        "gTileCounts");
    fields[3] = static_cast<std::int64_t>(constant_block->next_offset);
}

std::int32_t depth_range_shader_mode(void*) {
    return 0;
}

std::int32_t depth_range_shader_variant(void*) {
    return 16;
}

DepthRangeShaderDispatch kDepthRangeShaderDispatch{
    depth_range_shader_destruct,
    depth_range_shader_delete,
    depth_range_shader_source_identifier,
    depth_range_shader_backend_path,
    initialize_depth_range_support_objects,
    depth_range_shader_mode,
    depth_range_shader_variant,
};

}  // namespace

// Reconstructed from eboot.elf at 0x636DE0.
void render_calc_depth_range_compute_shader_construct(void* shader) {
    render_primary_shader_construct(primary_shader(shader));
    *static_cast<void**>(shader) = &kDepthRangeShaderDispatch;

    auto* fields = shader_fields(shader);
    fields[0] = -1;
    fields[1] = -1;
    fields[2] = -1;
    fields[3] = 0;
}

}  // namespace rb4
