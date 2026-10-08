#pragma once

#include "render/core/synchronization/render_occlusion_query.h"

namespace rb4 {

void render_occlusion_query_set_base_dispatch(RenderOcclusionQuery& query);
void render_delete_occlusion_query_storage(RenderOcclusionQuery& query);

}  // namespace rb4
