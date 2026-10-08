#pragma once

#include <cstddef>

#include "orbis_occlusion_query.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void occlusion_query_construct(OrbisOcclusionQuery& query, void* owner);
void orbis_occlusion_query_clear_backend_state(OrbisOcclusionQuery& query);

}  // namespace rb4
