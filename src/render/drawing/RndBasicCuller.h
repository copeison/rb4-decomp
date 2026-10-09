#pragma once

#include <cstddef>

#include "render/drawing/PodVector.h"
#include "render/drawing/RndCuller.h"
#include "render/drawing/RndDrawInstance.h"
#include "utl/containers/FixedVector.h"
#include "utl/containers/Vector.h"
#include "utl/containers/VectorAdapter.h"
#include "utl/threading/ThreadedJob.h"

class Frustum;
class RndBasicCuller;
class RndCameraContext;
class RndDrawInstanceCom;
enum RndSceneLod : unsigned int;

// The culler the scene drawer uses (render/RndBasicCuller.o,
// 0x459C20-0x45EB90). It owns the draw instances of the registered
// components, one pool per scene level, and copies the instances a camera
// sees into the drawer's buckets: on the calling thread, or split into
// RndBasicCullerJobs polled by the "Culler" poll group. The vtable at
// 0x1903740 has 10 slots. The object is 344 bytes.
class RndBasicCuller : public RndCuller {
public:
    // The draw instances of one scene level (112 bytes). Registrations take
    // a run of freed instances, then the pool's spare capacity; once the
    // pool is full they get a block of their own, since growing the pool
    // would move instances the components point at. The map's constructor
    // and destructor (LodInstances(), ~LodInstances()) are inlined into the
    // culler's here. Field names are not in the reference map.
    struct LodInstances {
        LodInstances() : mOverflowCount(0), mPeakOverflowCount(0) {}

        eastl::vector<RndDrawInstance> mInstances;
        // The deregistered instances inside mInstances, sorted by address.
        eastl::vector<RndDrawInstance*> mFreeList;
        // The blocks allocated with new[] when mInstances was full.
        eastl::vector<VectorAdapter<RndDrawInstance>> mOverflowBlocks;
        // The instances in mOverflowBlocks, and the most there have been.
        // Nothing reads the peak.
        unsigned long mOverflowCount;
        unsigned long mPeakOverflowCount;
    };

    // Creates the "Culler" poll group on first use.
    RndBasicCuller();  // 0x459C20
    // Slots 0-1: 0x459DC0, 0x45A100. Clears the culler first.
    ~RndBasicCuller() override;

    // Slot 2 at 0x45A120: reserves one and a half times the counts in each
    // level's empty pool and free list.
    void PrepareToGrowBy(const unsigned long* counts) override;
    // Slot 3 at 0x459F00: forgets every instance, frees the overflow blocks
    // and trims the job lists (TrimPools, inlined).
    void Clear() override;
    // Slots 4-5 at 0x45A490 and 0x45ABF0. The map's signatures start with
    // an ObjPtr const&.
    void RegisterDrawInstancesForCom(RndDrawInstanceCom& com) override;
    void DeRegisterDrawInstancesForCom(RndDrawInstanceCom& com) override;
    // Slot 6 at 0x45CF60: empty.
    void Poll() override;
    // Slot 7 at 0x45AE90: culls on the calling thread when the parameters
    // ask for no culling or the thread already polls for a manager, and in
    // jobs otherwise.
    void Cull(
        unsigned char* sceneDrawId,
        unsigned int sceneDrawIndex,
        const RndCameraContext& camera,
        const RndShowHideContext& showHide,
        VectorAdapter<PodVector<RndDrawInstance>>& buckets,
        const RndCullerParams& params) override;
    // Slot 8 at 0x45B940: counts the first level's pool instances per
    // bucket. Name not in the reference map.
    void GetMaxBucketSizes(FixedVector<unsigned long, kNumDrawBuckets>& sizes) const override;
    // Slot 9 at 0x45A3B0: frees the jobs' visible lists, keeping nine
    // tenths of each used list's capacity (at least its size) as the next
    // reservation. Name not in the reference map.
    void TrimPools() override;

    // Takes `count` consecutive freed instances of the level from the free
    // list and resets them, or returns null. Registration inlines it.
    RndDrawInstance* _AllocFromFreeList(RndSceneLod lod, unsigned long count);  // 0x45AA60

    // The map's signatures start with RndContext&; this build passes the
    // scene-draw id and its first word (see RndCuller::Cull), which the
    // visits ignore, and the multithreaded cull takes only the id.
    void _CullSingleThreaded(
        unsigned char* sceneDrawId,
        unsigned int sceneDrawIndex,
        const RndCameraContext& camera,
        const RndShowHideContext& showHide,
        VectorAdapter<PodVector<RndDrawInstance>>& buckets,
        const RndCullerParams& params);  // 0x45B6C0
    void _CullMultiThreaded(
        unsigned char* sceneDrawId,
        const RndCameraContext& camera,
        const RndShowHideContext& showHide,
        VectorAdapter<PodVector<RndDrawInstance>>& buckets,
        const RndCullerParams& params);  // 0x45B080

    // Appends the level's visible instances to the buckets of their state
    // flags. An instance is visible inside the frustum and outside
    // `prevFrustum`, the previous level's; with N = 1 it also needs a state
    // flag of RndCullerParams::mStateFlagMask, which these visits use as a
    // bucket mask. At 0x45C1B0 (N = 0) and 0x45C660 (N = 1).
    template <unsigned int N>
    void _VisitLodInstances(
        unsigned char* sceneDrawId,
        unsigned int sceneDrawIndex,
        const Frustum& frustum,
        const Frustum* prevFrustum,
        const RndShowHideContext& showHide,
        const LodInstances& instances,
        VectorAdapter<PodVector<RndDrawInstance>>& buckets,
        const RndCullerParams& params);

    // One pool per scene level. Name not in the reference map.
    LodInstances mLods[3];
};

static_assert(offsetof(RndBasicCuller::LodInstances, mFreeList) == 32);
static_assert(offsetof(RndBasicCuller::LodInstances, mOverflowBlocks) == 64);
static_assert(offsetof(RndBasicCuller::LodInstances, mOverflowCount) == 96);
static_assert(offsetof(RndBasicCuller::LodInstances, mPeakOverflowCount) == 104);
static_assert(sizeof(RndBasicCuller::LodInstances) == 112);
static_assert(offsetof(RndBasicCuller, mLods) == 8);
static_assert(sizeof(RndBasicCuller) == 344);

// One share of a multithreaded cull (1296 bytes). In the first pass a job
// tests a range of one level's pool, and the first job of a level also the
// overflow blocks, collecting pointers to the visible instances per bucket;
// in the second it copies them into the drawer's buckets at the offsets
// the earlier jobs leave free, and sets their sort distances. The culler
// keeps 48 in a static array. The vtable at 0x19037A0 has 12 slots. Field
// names are not in the reference map.
class RndBasicCullerJob : public ThreadedJob {
public:
    // The pass a job runs. Names not in the reference map.
    enum Mode {
        kModeCull = 0,
        kModeCopy = 1,
    };

    // Inlined into the static initializer (0x45EA70).
    RndBasicCullerJob();
    // Slots 0-1: 0x45CDB0 (a jump to 0x45E330), 0x45CF70. Frees the visible
    // lists.
    ~RndBasicCullerJob() override;

    // Slot 10 at 0x45CF90: runs the job's pass; the cull takes the bucket
    // mask template when the parameters have one. The map names it
    // RndBasicCullerJob::Poll().
    void _DoPoll() override;
    // Slot 11 at 0x412070, a shared empty body: nothing to do after the
    // poll.
    void _DoPostPoll() override {}

    // The first pass, with the culler's visit test (N as in
    // RndBasicCuller::_VisitLodInstances). At 0x45CFC0 (N = 0) and 0x45D710
    // (N = 1).
    template <unsigned int N>
    void _PollCullMode();
    // The second pass. Empties the visible lists.
    void _PollCopyMode();  // 0x45DE80

    RndBasicCuller* mCuller;
    int mMode;
    // The level and the range of its pool the job tests.
    int mLod;
    unsigned long mBegin;
    unsigned long mEnd;
    // Also tests the level's overflow blocks.
    bool mIncludeOverflow;
    const RndCameraContext* mCamera;
    RndShowHideContext mShowHide;
    RndCullerParams mParams;
    // The visible instances per bucket.
    PodVector<RndDrawInstance*> mVisible[kNumDrawBuckets];
    // What each visible list reserves before the first pass (TrimPools).
    unsigned long mReserveSizes[kNumDrawBuckets];
    // The second pass's destination and where its instances go in each
    // bucket.
    VectorAdapter<PodVector<RndDrawInstance>>* mOutBuckets;
    unsigned long mOffsets[kNumDrawBuckets];
    // The pool and overflow instances the job tested; nothing reads them.
    unsigned long mNumTested;
    unsigned long mNumOverflowTested;
};

static_assert(offsetof(RndBasicCullerJob, mCuller) == 144);
static_assert(offsetof(RndBasicCullerJob, mMode) == 152);
static_assert(offsetof(RndBasicCullerJob, mLod) == 156);
static_assert(offsetof(RndBasicCullerJob, mBegin) == 160);
static_assert(offsetof(RndBasicCullerJob, mEnd) == 168);
static_assert(offsetof(RndBasicCullerJob, mIncludeOverflow) == 176);
static_assert(offsetof(RndBasicCullerJob, mCamera) == 184);
static_assert(offsetof(RndBasicCullerJob, mShowHide) == 192);
static_assert(offsetof(RndBasicCullerJob, mParams) == 200);
static_assert(offsetof(RndBasicCullerJob, mVisible) == 392);
static_assert(offsetof(RndBasicCullerJob, mReserveSizes) == 920);
static_assert(offsetof(RndBasicCullerJob, mOutBuckets) == 1096);
static_assert(offsetof(RndBasicCullerJob, mOffsets) == 1104);
static_assert(offsetof(RndBasicCullerJob, mNumTested) == 1280);
static_assert(offsetof(RndBasicCullerJob, mNumOverflowTested) == 1288);
static_assert(sizeof(RndBasicCullerJob) == 1296);
