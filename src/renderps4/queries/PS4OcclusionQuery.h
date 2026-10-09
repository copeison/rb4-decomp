#pragma once

#include <cstddef>

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

    void* mResults;  // Name not in the reference map.

private:
    // Stand-ins for context command code inlined into the slots; not yet
    // reconstructed. Names not in the reference map.
    static void* AllocateResults(RndContext& context, unsigned long size, unsigned long alignment);
    static void BeginQueryCommand(RndContext& context, void* results);
    static void EndQueryCommand(RndContext& context, const void* results);
    static void SetQueryEnabled(RndContext& context, bool enabled);
    static void BeginPredicationCommand(RndContext& context, const void* results);
    static void EndPredicationCommand(RndContext& context);
};

static_assert(offsetof(PS4OcclusionQuery, mResults) == 64);
static_assert(sizeof(PS4OcclusionQuery) == 72);
