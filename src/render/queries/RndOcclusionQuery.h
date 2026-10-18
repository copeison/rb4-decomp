#pragma once

#include <cstddef>

#include "math/geometry/Sphere.h"
#include "os/memory/MemMgr.h"

class RndContext;
class RndShaderCBuffer;

// GPU occlusion query of a world sphere, linked into its scene drawer's
// RndOcclusionQueryMgr while alive (render/RndOcclusionQuery.o). The base
// vtable is at 0x192AFD8.
class RndOcclusionQuery {
public:
    // Intrusive list node. Name not in the reference map.
    struct Link {
        Link* mNext;
        Link* mPrev;
    };

    // Reconstructed from eboot.elf at 0x5F7D30.
    static RndOcclusionQuery* New(const char* name);

    explicit RndOcclusionQuery(const char* name);  // 0x5F7D50
    virtual ~RndOcclusionQuery();                  // 0x5F7DA0, 0x5F7DD0

    virtual void _BeginQueryImpl(RndContext& context) = 0;        // slot 2
    virtual void _EndQueryImpl(RndContext& context) = 0;          // slot 3
    virtual void _BeginPredicationImpl(RndContext& context) = 0;  // slot 4
    virtual void _EndPredicationImpl(RndContext& context) = 0;    // slot 5

    // Writes the query's index and expected coverage into the context's
    // occlusion constant buffer and selects it. RndLightFlareCom draws its
    // subflares with it (0x47D1C9).
    void SetShaderConstants(RndContext& context);  // 0x5F7E00
    // Writes the constants of a draw without a query (index 0, coverage 1)
    // into the buffer and marks it for upload. RndDevice::
    // _InitBuiltinCBuffers (0x3DDE2C) fills its default occlusion buffer
    // with it. The map's signature takes a RndContext&.
    static void SetNoOcclusionQueryShaderConstants(
        RndShaderCBuffer& buffer);  // 0x5F7E70

    DELETE_OVERLOAD

    // Field names are not in the reference map.
    const char* mName;
    // Whether the query is drawn; the owner sets it each poll.
    bool mEnabled;
    // Per view (RndOcclusionQueryMgr::GetViewIndex): whether the last
    // Submit found the sphere outside the camera's frustum.
    bool mCulled[2];
    // The world sphere the query draws, set by the owner.
    Sphere mSphere;
    // The index RndOcclusionQueryMgr::RegisterQuery gave the query, or ~0;
    // it selects the query's slot in the coverage buffer.
    unsigned int mIndex;
    // The pixels the sphere's octagonal silhouette covers on screen, set by
    // Submit; the coverage is read against it.
    unsigned int mPixelCount;
    // Alignment padding before mLink; never read or written. Name not in
    // the reference map.
    unsigned int mPad;
    Link mLink;
};

static_assert(sizeof(RndOcclusionQuery::Link) == 16);
static_assert(offsetof(RndOcclusionQuery, mEnabled) == 16);
static_assert(offsetof(RndOcclusionQuery, mCulled) == 17);
static_assert(offsetof(RndOcclusionQuery, mSphere) == 20);
static_assert(offsetof(RndOcclusionQuery, mIndex) == 36);
static_assert(offsetof(RndOcclusionQuery, mPixelCount) == 40);
static_assert(offsetof(RndOcclusionQuery, mLink) == 48);
static_assert(sizeof(RndOcclusionQuery) == 64);
