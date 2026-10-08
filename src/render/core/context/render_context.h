#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderContext;

struct RenderContextVtable {
    void* reserved_0;
    void (*delete_context)(RenderContext* context);
    void (*initialize)(RenderContext* context);
    void (*shutdown)(RenderContext* context);
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

void render_context_delete(RenderContext& context);
void render_context_initialize(RenderContext& context);
void render_context_shutdown(RenderContext& context);

}  // namespace rb4
