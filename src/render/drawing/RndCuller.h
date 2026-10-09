#pragma once

#include <cstddef>

#include "math/vector/Vector4.h"
#include "render/drawing/PodVector.h"
#include "render/drawing/RndDrawInstance.h"
#include "utl/containers/FixedVector.h"
#include "utl/containers/VectorAdapter.h"

class RndCameraContext;
class RndDrawInstanceCom;

// Which draw instances a camera shows: an instance is drawn when it has a
// flag of mShowFlags (or mShowFlags is zero) and none of mHideFlags
// (RndBasicCuller's visit, 0x45C1B0). The draw parameters default to
// hiding 0x21; the scene drawer adds 2 when it draws the scene's first
// camera and 4 for its second. The map names the type; field names are not
// in the reference map.
struct RndShowHideContext {
    unsigned int mShowFlags;
    unsigned int mHideFlags;
};

static_assert(sizeof(RndShowHideContext) == 8);

// Per-cull options the scene drawer passes the culler (the map's
// RndCullerParams). Field names are not in the reference map.
struct RndCullerParams {
    RndCullerParams() : mNoCulling(false), mStateFlagMask(-1) {}

    // Keeps every instance, skipping the frustum and clip-plane tests; it
    // also keeps the cull on the calling thread. The drawer sets it from
    // the scene component's flag at +280.
    bool mNoCulling;
    // -1 for every instance; otherwise a mask of RndDrawInstance::
    // mStateFlags an instance needs, and the visit of the levels after the
    // first (_VisitLodInstances<1>) is used. The depth-only draw passes 8.
    int mStateFlagMask;
    // Extra planes an instance's bounds must not lie wholly behind; the
    // drawer copies the camera context's (RndCameraContext::mDerivedCache).
    FixedVector<Vector4, 10> mClipPlanes;
};

static_assert(offsetof(RndCullerParams, mStateFlagMask) == 4);
static_assert(offsetof(RndCullerParams, mClipPlanes) == 8);
static_assert(sizeof(RndCullerParams) == 192);

// The interface the scene drawer culls through. The map has only its
// typeinfo, emitted in render/RndBasicCuller.o; every slot is pure and the
// destructor is inline, so no RndCuller vtable exists in the binary.
// RndBasicCuller is the only implementation. Slot names after slot 7 are
// not in the reference map.
class RndCuller {
public:
    virtual ~RndCuller() {}  // Slots 0-1.
    // Slot 2: makes room for the draw instance counts of each scene level
    // that registrations added.
    virtual void PrepareToGrowBy(const unsigned long* counts) = 0;
    // Slot 3: forgets every registered component.
    virtual void Clear() = 0;
    // Slots 4-5. The map's signatures start with an ObjPtr const&.
    virtual void RegisterDrawInstancesForCom(RndDrawInstanceCom& com) = 0;
    virtual void DeRegisterDrawInstancesForCom(RndDrawInstanceCom& com) = 0;
    // Slot 6: per-frame upkeep before the draw.
    virtual void Poll() = 0;
    // Slot 7: copies the camera's visible instances into the buckets. The
    // map's signature starts with RndContext&; this build passes the
    // context's scene-draw id bytes (RndContext::mSceneDrawId) and their
    // first word, which only the multithreaded jobs carry along.
    virtual void Cull(
        unsigned char* sceneDrawId,
        unsigned int sceneDrawIndex,
        const RndCameraContext& camera,
        const RndShowHideContext& showHide,
        VectorAdapter<PodVector<RndDrawInstance>>& buckets,
        const RndCullerParams& params) = 0;
    // Slot 8: the most instances each draw bucket can receive, which the
    // drawer reserves. Name not in the reference map.
    virtual void GetMaxBucketSizes(FixedVector<unsigned long, kNumDrawBuckets>& sizes) const = 0;
    // Slot 9: releases the shared instance pools, keeping nine tenths of
    // their peak sizes as the next reservation. Name not in the reference
    // map; the evidence is weak.
    virtual void TrimPools() = 0;
};
