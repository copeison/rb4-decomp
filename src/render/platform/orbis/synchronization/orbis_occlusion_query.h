#pragma once

#include "render/core/render_occlusion_query.h"

namespace rb4 {

struct OrbisRenderContext;

struct OrbisOcclusionQuery : RenderOcclusionQuery {
    void* result_address;
};

static_assert(sizeof(OrbisOcclusionQuery) == 72);

OrbisOcclusionQuery* orbis_create_occlusion_query(void* owner);
void orbis_occlusion_query_construct(
    OrbisOcclusionQuery& query,
    void* owner);
void orbis_occlusion_query_destruct(OrbisOcclusionQuery& query);
void orbis_occlusion_query_delete(OrbisOcclusionQuery& query);
void orbis_occlusion_query_begin(
    OrbisOcclusionQuery& query,
    OrbisRenderContext& context);
void orbis_occlusion_query_end(
    OrbisOcclusionQuery& query,
    OrbisRenderContext& context);
void orbis_occlusion_query_begin_conditional_render(
    const OrbisOcclusionQuery& query,
    OrbisRenderContext& context);
void orbis_occlusion_query_end_conditional_render(
    const OrbisOcclusionQuery& query,
    OrbisRenderContext& context);

}  // namespace rb4
