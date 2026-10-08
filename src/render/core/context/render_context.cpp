#include "render/core/context/render_context.h"

namespace rb4 {

namespace {

constexpr std::size_t kGpuStatScopeEndOffset = 22280;
constexpr std::size_t kGpuStatScopeSize = 16;

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

void render_context_pop_gpu_stat_scope(RenderContext& context) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&context);
    auto*& end = *reinterpret_cast<std::uint8_t**>(
        bytes + kGpuStatScopeEndOffset);
    end -= kGpuStatScopeSize;
}

void render_context_end_gpu_stat(
    RenderContext& context,
    std::uint64_t query_id) {
    context.virtual_table->end_gpu_stat(&context, query_id);
}

}  // namespace rb4
