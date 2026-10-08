#pragma once

#include <cstdint>

namespace rb4 {

struct RenderOcclusionQueryLink {
    RenderOcclusionQueryLink* next;
    RenderOcclusionQueryLink* previous;
};

struct RenderOcclusionQuery {
    void* implementation;
    void* owner;
    std::uint8_t state_flags[3];
    std::uint8_t reserved_alignment;
    std::int32_t query_values[4];
    std::int32_t frame_index;
    std::uint32_t result;
    std::uint32_t reserved;
    RenderOcclusionQueryLink link;
};

static_assert(sizeof(RenderOcclusionQueryLink) == 16);
static_assert(sizeof(RenderOcclusionQuery) == 64);

RenderOcclusionQuery* render_create_occlusion_query(void* owner);
void render_occlusion_query_construct(
    RenderOcclusionQuery& query,
    void* owner);
void render_occlusion_query_destruct(RenderOcclusionQuery& query);
void render_occlusion_query_delete(RenderOcclusionQuery& query);

}  // namespace rb4
