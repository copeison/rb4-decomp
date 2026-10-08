#include "render/core/context/render_context.h"

namespace rb4 {

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

}  // namespace rb4
