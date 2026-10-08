#pragma once

#include <cstddef>

#include "render/platform/orbis/synchronization/orbis_occlusion_query.h"

namespace rb4 {

void orbis_occlusion_query_install_vtable(OrbisOcclusionQuery& query);
void* orbis_occlusion_query_allocate_result(
    OrbisRenderContext& context,
    std::size_t size,
    std::size_t alignment);
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
