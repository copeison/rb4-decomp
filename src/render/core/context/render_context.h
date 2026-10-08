#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderContext;
struct RenderTexture;

struct RenderContextSubmissionResource {
    void* reserved_0;
    RenderTexture* texture;
    std::int64_t resource_index;
    std::uint64_t flags;
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
static_assert(
    offsetof(RenderContextVtable, prepare_submission_resources) == 168);

void render_context_delete(RenderContext& context);
void render_context_initialize(RenderContext& context);
void render_context_shutdown(RenderContext& context);
void render_context_prepare_submission_resources(
    RenderContext& context,
    const RenderContextSubmissionResource* resources,
    std::size_t resource_count);

}  // namespace rb4
