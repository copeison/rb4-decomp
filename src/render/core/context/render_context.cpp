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

}  // namespace rb4
