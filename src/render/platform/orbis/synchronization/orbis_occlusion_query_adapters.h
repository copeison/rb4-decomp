#pragma once

#include <cstddef>

#include "render/platform/orbis/synchronization/orbis_occlusion_query.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void occlusion_query_construct(OrbisOcclusionQuery& query, void* owner);
void orbis_occlusion_query_clear_backend_state(OrbisOcclusionQuery& query);
void occlusion_query_destruct(OrbisOcclusionQuery& query);
void render_delete_occlusion_query(OrbisOcclusionQuery& query);
void* orbis_occlusion_query_allocate_result(
    OrbisRenderContext& context,
    std::size_t size,
    std::size_t alignment);
void orbis_occlusion_query_set_result_address(
    OrbisOcclusionQuery& query,
    void* address);
const void* orbis_occlusion_query_result_address(
    const OrbisOcclusionQuery& query);
void orbis_command_begin_occlusion_query(
    OrbisRenderContext& context,
    void* result_address);
void orbis_command_end_occlusion_query(
    OrbisRenderContext& context,
    const void* result_address);
void orbis_command_set_occlusion_query_enabled(
    OrbisRenderContext& context,
    bool enabled);
void orbis_command_begin_conditional_render(
    OrbisRenderContext& context,
    const void* result_address);
void orbis_command_end_conditional_render(OrbisRenderContext& context);

}  // namespace rb4
