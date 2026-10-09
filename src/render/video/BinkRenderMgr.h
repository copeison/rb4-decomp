#pragma once

#include <cstddef>

#include "render/meshes/RndDynamicGpuData.h"
#include "utl/containers/Vector.h"

class RndContext;
class RndTextureBase;

// A Bink video's render resources: the decoded planes, the converted output
// and Bink's color-space constants. The vtable is at 0x192AA28; its base
// is RndDynamicGpuData, whose per-frame update copies the color-space
// constants. Name and field names not in the reference map, which has no
// Bink code.
class BinkRenderVideo : public RndDynamicGpuData {
public:
    ~BinkRenderVideo() override;  // 0x5F30A0, 0x5F31F0
    void _SyncDynamicGpuDataImpl(RndContext& context) override;  // 0x5F3330

    void* mBink;                  // The Bink handle.
    void* mDecodeState;           // 216 bytes, owned.
    // The plane textures, of which the alpha plane is optional, and the
    // RGB output. Owned.
    RndTextureBase* mYPlane;
    RndTextureBase* mCRPlane;
    RndTextureBase* mCBPlane;
    RndTextureBase* mAPlane;
    RndTextureBase* mOutput;
    // Copied from the Bink handle's color-space constants (0x5F3330).
    float mYScale[4];
    float mCRScale[4];
    float mCBScale[4];
    float mOffset[4];
    bool mConversionPending;
};

static_assert(offsetof(BinkRenderVideo, mBink) == 16);
static_assert(offsetof(BinkRenderVideo, mYPlane) == 32);
static_assert(offsetof(BinkRenderVideo, mOutput) == 64);
static_assert(offsetof(BinkRenderVideo, mYScale) == 72);
static_assert(offsetof(BinkRenderVideo, mOffset) == 120);
static_assert(offsetof(BinkRenderVideo, mConversionPending) == 136);

// The Bink videos the renderer draws, with up to four frame conversions
// queued for the next frame. Name not in the reference map, which has no
// Bink code.
class BinkRenderMgr {
public:
    // The manager the device's frame start uses; RndDevice::_DoBeginFrame
    // reads sInstance inline. Name not in the reference map.
    static BinkRenderMgr& Instance() { return *sInstance; }
    // Runs the queued frame conversions.
    void PrepareFrame(RndContext& context);  // 0x5F2B40
    // Converts the video's decoded planes into its output texture. The
    // manager is not read, and the callers leave its argument register
    // unset. Name not in the reference map.
    void ConvertFrame(RndContext& context, BinkRenderVideo& video);  // 0x5F2BF0

    // Allocated and published by the initialization at 0x5F28C0. Name not
    // in the reference map.
    static BinkRenderMgr* sInstance;  // 0x1AA76F8

    // Field names are not in the reference map.
    eastl::vector<BinkRenderVideo*> mVideos;
    BinkRenderVideo* mConversions[4];
    unsigned int mWorkingBufferCount;
    // Tail padding: the initialization at 0x5F28C0 does not write it and
    // nothing reads it.
    unsigned int mPad68;
};

static_assert(offsetof(BinkRenderMgr, mConversions) == 32);
static_assert(offsetof(BinkRenderMgr, mWorkingBufferCount) == 64);
static_assert(sizeof(BinkRenderMgr) == 72);
