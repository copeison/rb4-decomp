#include "render/core/context/render_context.h"

#include <cstring>

#include "core/memory/engine_memory.h"

namespace rb4 {

namespace {

constexpr std::size_t kGpuStatScopeArrayOffset = 22272;

struct RenderGpuStatScopeArray {
    RenderGpuStatScope* begin;
    RenderGpuStatScope* end;
    RenderGpuStatScope* capacity;
    void* allocator;
};

RenderGpuStatScopeArray& gpu_stat_scopes(RenderContext& context) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&context);
    return *reinterpret_cast<RenderGpuStatScopeArray*>(
        bytes + kGpuStatScopeArrayOffset);
}

}  // namespace

void render_context_delete(RenderContext& context) {
    context.virtual_table->delete_context(&context);
}

void render_context_initialize(RenderContext& context) {
    context.virtual_table->initialize(&context);
}

void render_context_shutdown(RenderContext& context) {
    context.virtual_table->shutdown(&context);
}

void render_context_prepare_submission_resources(
    RenderContext& context,
    const RenderContextSubmissionResource* resources,
    std::size_t resource_count) {
    context.virtual_table->prepare_submission_resources(
        &context, resource_count, resources);
}

RenderGpuStatScope* render_context_last_gpu_stat_scope(
    RenderContext& context) {
    auto& scopes = gpu_stat_scopes(context);
    return scopes.begin == scopes.end ? nullptr : scopes.end - 1;
}

void render_context_push_gpu_stat_scope(
    RenderContext& context,
    const RenderGpuStatScope& scope) {
    auto& scopes = gpu_stat_scopes(context);
    if (scopes.end != scopes.capacity) {
        *scopes.end++ = scope;
        return;
    }

    const auto size = scopes.begin == nullptr
        ? std::size_t{0}
        : static_cast<std::size_t>(scopes.end - scopes.begin);
    const auto new_capacity = size == 0 ? std::size_t{1} : size * 2;
    auto* new_begin = static_cast<RenderGpuStatScope*>(
        engine_allocate_sized(new_capacity * sizeof(RenderGpuStatScope)));
    if (size != 0) {
        std::memmove(
            new_begin,
            scopes.begin,
            size * sizeof(RenderGpuStatScope));
    }
    new_begin[size] = scope;

    if (scopes.begin != nullptr) {
        const auto byte_count = static_cast<std::size_t>(
            reinterpret_cast<std::uint8_t*>(scopes.capacity) -
            reinterpret_cast<std::uint8_t*>(scopes.begin));
        engine_deallocate_sized(scopes.begin, byte_count);
    }

    scopes.begin = new_begin;
    scopes.end = new_begin + size + 1;
    scopes.capacity = new_begin + new_capacity;
}

void render_context_pop_gpu_stat_scope(RenderContext& context) {
    --gpu_stat_scopes(context).end;
}

void render_context_begin_gpu_stat(
    RenderContext& context,
    std::uint64_t query_id) {
    context.virtual_table->begin_gpu_stat(&context, query_id);
}

void render_context_end_gpu_stat(
    RenderContext& context,
    std::uint64_t query_id) {
    context.virtual_table->end_gpu_stat(&context, query_id);
}

RenderGpuStatistics render_context_resolve_gpu_stat(
    RenderContext& context,
    std::uint64_t query_id) {
    return context.virtual_table->resolve_gpu_stat(&context, query_id);
}

}  // namespace rb4
