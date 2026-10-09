#pragma once

#include <cstddef>
#include <gnm/gpustructs.h>

#include "render/queries/RndOcclusionQuery.h"

// PS4 occlusion query. The vtable is at 0x195F740.
class PS4OcclusionQuery : public RndOcclusionQuery {
public:
    explicit PS4OcclusionQuery(const char* name);  // 0x8E28C0
    ~PS4OcclusionQuery() override {}               // 0x8E28F0, 0x8E2900

    void _BeginQueryImpl(RndContext& context) override;        // 0x8E2920
    void _EndQueryImpl(RndContext& context) override;          // 0x8E29E0
    void _BeginPredicationImpl(RndContext& context) override;  // 0x8E2A30
    void _EndPredicationImpl(RndContext& context) override;    // 0x8E2A60

    // Results of the current query, in the active graphics command buffer.
    // Name not in the reference map.
    sce::Gnm::OcclusionQueryResults* mResults;
};

static_assert(offsetof(PS4OcclusionQuery, mResults) == 64);
static_assert(sizeof(PS4OcclusionQuery) == 72);
