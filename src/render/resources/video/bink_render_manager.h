#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderContext;

struct BinkVideoRenderObject {
    std::uint8_t reserved_0[136];
    bool conversion_pending;
};

struct BinkRenderManager {
    BinkVideoRenderObject** videos_begin;
    BinkVideoRenderObject** videos_end;
    BinkVideoRenderObject** videos_capacity;
    void* videos_allocator;
    BinkVideoRenderObject* conversion_slots[4];
    std::uint32_t working_buffer_count;
    std::uint32_t reserved_68;
};

static_assert(
    offsetof(BinkVideoRenderObject, conversion_pending) == 136);
static_assert(sizeof(BinkVideoRenderObject) == 137);
static_assert(offsetof(BinkRenderManager, conversion_slots) == 32);
static_assert(offsetof(BinkRenderManager, working_buffer_count) == 64);
static_assert(sizeof(BinkRenderManager) == 72);

void bink_render_manager_prepare_frame(
    BinkRenderManager& manager,
    RenderContext& context);

}  // namespace rb4
