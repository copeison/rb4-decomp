#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "render/core/transition_aliases.h"

// GPU occlusion query, linked into an intrusive list while alive. The base
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

    DELETE_OVERLOAD

    // Field names are not in the reference map.
    const char* mName;
    unsigned char mState[3];
    int mResults[4];
    int mFrame;
    unsigned int mResult;
    unsigned int mUnknown44;
    Link mLink;
};

static_assert(sizeof(RndOcclusionQuery::Link) == 16);
static_assert(offsetof(RndOcclusionQuery, mResults) == 20);
static_assert(offsetof(RndOcclusionQuery, mFrame) == 36);
static_assert(offsetof(RndOcclusionQuery, mLink) == 48);
static_assert(sizeof(RndOcclusionQuery) == 64);
