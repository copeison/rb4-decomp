#pragma once

#include <cstddef>

#include "utl/containers/Vector.h"

class RndContext;

// A Bink video's render resources. Only the pending-conversion flag is
// recovered. Name not in the reference map, which has no Bink code.
class BinkRenderVideo {
public:
    // Not reconstructed yet. Converts the decoded frame's planes to RGB.
    void ConvertFrame(RndContext& context);

    // Field names are not in the reference map.
    unsigned char mUnknown0[136];
    bool mConversionPending;
};

static_assert(offsetof(BinkRenderVideo, mConversionPending) == 136);
static_assert(sizeof(BinkRenderVideo) == 137);

// The Bink videos the renderer draws, with up to four frame conversions
// queued for the next frame. Name not in the reference map, which has no
// Bink code.
class BinkRenderMgr {
public:
    // Not reconstructed yet. The manager the device's frame start uses.
    static BinkRenderMgr& Instance();
    // Runs the queued frame conversions.
    void PrepareFrame(RndContext& context);  // 0x5F2B40

    // Field names are not in the reference map.
    eastl::vector<BinkRenderVideo*> mVideos;
    BinkRenderVideo* mConversions[4];
    unsigned int mWorkingBufferCount;
    unsigned int mUnknown68;
};

static_assert(offsetof(BinkRenderMgr, mConversions) == 32);
static_assert(offsetof(BinkRenderMgr, mWorkingBufferCount) == 64);
static_assert(sizeof(BinkRenderMgr) == 72);
