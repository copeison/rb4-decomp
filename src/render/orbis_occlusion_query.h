#pragma once

namespace rb4 {

struct OrbisOcclusionQuery;

OrbisOcclusionQuery* orbis_create_occlusion_query(void* owner);
void orbis_occlusion_query_construct(
    OrbisOcclusionQuery& query,
    void* owner);

}  // namespace rb4
