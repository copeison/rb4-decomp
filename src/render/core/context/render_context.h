#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

class RndShaderCBuffer;

class RndTextureBase;

namespace rb4 {

struct RenderContext;

struct RenderContextSubmissionResource {
    void* reserved_0;
    RndTextureBase* texture;
    std::int64_t resource_index;
    std::uint64_t flags;
};

struct RenderGpuStatScope {
    void* statistic;
    std::uint64_t query_id;
};

struct RenderGpuStatistics {
    float elapsed_seconds;
    std::uint32_t reserved;
    std::array<std::uint64_t, 6> hardware_counters;
};

struct RenderContextVtable {
    void* reserved_0;
    void (*delete_context)(RenderContext* context);
    void (*initialize)(RenderContext* context);
    void (*shutdown)(RenderContext* context);
    void* reserved_32[17];
    void (*prepare_submission_resources)(
        RenderContext* context,
        std::size_t resource_count,
        const RenderContextSubmissionResource* resources);
    void* reserved_176[6];
    void (*unbind_shader_stage)(RenderContext* context, std::uint32_t stage);
    void* reserved_232[2];
    void (*begin_gpu_stat)(RenderContext* context, std::uint64_t query_id);
    void (*end_gpu_stat)(RenderContext* context, std::uint64_t query_id);
    RenderGpuStatistics (*resolve_gpu_stat)(
        RenderContext* context,
        std::uint64_t query_id);
};

struct RenderContext {
    RenderContextVtable* virtual_table;
    bool frame_active;
    std::uint8_t reserved_9[3];
    std::uint32_t mode;
};

static_assert(offsetof(RenderContext, frame_active) == 8);
static_assert(offsetof(RenderContext, mode) == 12);
static_assert(sizeof(RenderContext) == 16);
static_assert(sizeof(RenderContextSubmissionResource) == 32);
static_assert(sizeof(RenderGpuStatScope) == 16);
static_assert(sizeof(RenderGpuStatistics) == 56);
static_assert(
    offsetof(RenderContextVtable, prepare_submission_resources) == 168);
static_assert(offsetof(RenderContextVtable, unbind_shader_stage) == 224);
static_assert(offsetof(RenderContextVtable, begin_gpu_stat) == 248);
static_assert(offsetof(RenderContextVtable, end_gpu_stat) == 256);
static_assert(offsetof(RenderContextVtable, resolve_gpu_stat) == 264);

void render_context_delete(RenderContext& context);
void render_context_initialize(RenderContext& context);
void render_context_shutdown(RenderContext& context);
void render_context_prepare_submission_resources(
    RenderContext& context,
    const RenderContextSubmissionResource* resources,
    std::size_t resource_count);
RenderGpuStatScope* render_context_last_gpu_stat_scope(
    RenderContext& context);
void render_context_push_gpu_stat_scope(
    RenderContext& context,
    const RenderGpuStatScope& scope);
void render_context_pop_gpu_stat_scope(RenderContext& context);
void render_context_begin_gpu_stat(
    RenderContext& context,
    std::uint64_t query_id);
void render_context_end_gpu_stat(
    RenderContext& context,
    std::uint64_t query_id);
RenderGpuStatistics render_context_resolve_gpu_stat(
    RenderContext& context,
    std::uint64_t query_id);

// Fields of the common context beyond its modeled 16-byte prefix.
// +0x10: render-target slice mode; -1 selects a single slice.
std::int32_t render_context_slice_mode(const RenderContext& context);
// +0x4960: bit per shader stage with a bound program.
std::uint8_t& render_context_active_shader_stages(RenderContext& context);
// +0x4A18: active debug shading mode.
std::int32_t render_context_shading_mode(const RenderContext& context);
// +0x4968 and +0x4998: per shader stage, one past the highest input and
// output resource slot bound this draw.
std::uint64_t& render_context_input_slot_limit(
    RenderContext& context,
    std::uint32_t stage);
std::uint64_t& render_context_output_slot_limit(
    RenderContext& context,
    std::uint32_t stage);
// +0x4A80: per-draw constant buffers of 16, 32, 64, ... elements.
RndShaderCBuffer* render_context_constant_buffer(
    RenderContext& context,
    std::size_t size_class);
void render_context_unbind_shader_stage(
    RenderContext& context,
    std::uint32_t stage);

}  // namespace rb4
