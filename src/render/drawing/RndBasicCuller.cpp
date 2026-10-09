#include "render/drawing/RndBasicCuller.h"

#include <algorithm>
#include <cstddef>

#include "math/geometry/Frustum.h"
#include "math/geometry/Sphere.h"
#include "render/context/RndCameraContext.h"
#include "render/drawing/RndDrawInstanceCom.h"
#include "utl/containers/LinkedList.h"
#include "utl/data/DataNode.h"
#include "utl/text/Symbol.h"
#include "utl/threading/PollDep.h"
#include "utl/threading/PollMgr.h"

namespace {

// Links the jobs of a cull pass through PollDepBase's spare link (+24)
// while they wait to be queued. Name not in the reference map.
struct CullJobLink {
    static LinkedList::Node& ToNode(PollDepBase& job) {
        return job.mAuxNode;
    }
    static PollDepBase* FromNode(LinkedList::Node* node) {
        return reinterpret_cast<PollDepBase*>(
            reinterpret_cast<char*>(node) - offsetof(PollDepBase, mAuxNode));
    }
};

using CullJobList = LinkedList::List<PollDepBase, CullJobLink>;

constexpr unsigned long kNumCullJobs = 48;

// Whether the sphere lies wholly behind one of the planes. Inlined into
// every visit.
template <typename PlaneT>
bool IsBehindAny(const Sphere& sphere, const PlaneT* planes, unsigned long count);

template <>
bool IsBehindAny(const Sphere& sphere, const Plane* planes, unsigned long count) {
    for (const Plane* plane = planes; plane != planes + count; ++plane) {
        const float distance = plane->a * sphere.center.x + plane->b * sphere.center.y +
                               plane->c * sphere.center.z + plane->d;
        if (distance < -sphere.radius) {
            return true;
        }
    }
    return false;
}

template <>
bool IsBehindAny(const Sphere& sphere, const Vector4* planes, unsigned long count) {
    for (const Vector4* plane = planes; plane != planes + count; ++plane) {
        const float distance = plane->x * sphere.center.x + plane->y * sphere.center.y +
                               plane->z * sphere.center.z + plane->w;
        if (distance < -sphere.radius) {
            return true;
        }
    }
    return false;
}

// The visit test of the single-threaded visits and the jobs' first pass:
// a shown, active instance with a drawable whose flags pass the show-hide
// context, then, unless culling is off for the cull or the instance, inside
// the frustum and the clip planes and outside the previous level's frustum.
// With N = 1 the instance also needs a state flag of the parameters' mask.
// Inlined. Name not in the reference map.
template <unsigned int N>
bool IsInstanceVisible(
    const RndDrawInstance& instance,
    const RndShowHideContext& showHide,
    const Frustum& frustum,
    const Frustum* prevFrustum,
    const RndCullerParams& params) {
    if (N == 1 && (instance.mStateFlags & static_cast<unsigned int>(params.mStateFlagMask)) == 0) {
        return false;
    }
    if (!instance.mActive || !instance.mShowing || instance.mDrawable == nullptr) {
        return false;
    }
    const unsigned int flags = instance.mWorldShowHideFlags;
    if ((showHide.mShowFlags != 0 && (flags & showHide.mShowFlags) == 0) ||
        (flags & showHide.mHideFlags) != 0) {
        return false;
    }
    if (params.mNoCulling || instance.mNoCull) {
        return true;
    }
    const Sphere& bounds = instance.mBounds;
    if (IsBehindAny(bounds, frustum.mPlanes.begin(), frustum.mPlanes.size()) ||
        IsBehindAny(bounds, params.mClipPlanes.begin(), params.mClipPlanes.size())) {
        return false;
    }
    return prevFrustum == nullptr ||
           IsBehindAny(bounds, prevFrustum->mPlanes.begin(), prevFrustum->mPlanes.size());
}

// The bucket list the visits write through the adapter's const view.
PodVector<RndDrawInstance>& Bucket(
    VectorAdapter<PodVector<RndDrawInstance>>& buckets,
    unsigned long index) {
    return const_cast<PodVector<RndDrawInstance>&>(buckets.mData[index]);
}

// Sets the instances' sort distances: the squared distance to the camera
// for a cube target, else the depth along the camera's forward axis, moved
// by the radius toward the camera for sort_by 3 and away for sort_by 1.
// Inlined into _CullSingleThreaded and _PollCopyMode. Name not in the
// reference map.
void UpdateSortDistances(
    const RndCameraContext& camera,
    RndDrawInstance* begin,
    RndDrawInstance* end) {
    const Transform& xfm = camera.mPrimaryView.mWorldXfm;
    if (camera.mTargetMode == kTargetModeCube) {
        for (RndDrawInstance* instance = begin; instance != end; ++instance) {
            const float dx = xfm.v.x - instance->mBounds.center.x;
            const float dy = xfm.v.y - instance->mBounds.center.y;
            const float dz = xfm.v.z - instance->mBounds.center.z;
            instance->mDistance = dx * dx + dy * dy + dz * dz;
        }
        return;
    }
    const Vector3& forward = xfm.m.y;
    for (RndDrawInstance* instance = begin; instance != end; ++instance) {
        float distance = forward.x * instance->mBounds.center.x +
                         forward.y * instance->mBounds.center.y +
                         forward.z * instance->mBounds.center.z;
        if (instance->mSortBy == 3) {
            distance -= instance->mBounds.radius;
        } else if (instance->mSortBy == 1) {
            distance += instance->mBounds.radius;
        }
        instance->mDistance = distance;
    }
}

}  // namespace

// The object's statics, in the order of its static initializer (0x45EA70).
// Three ints the initializer stores first at 0x1A76588 and that nothing in
// this build reads. The name and the field names are not in the reference
// map and are guesses from the values only.
static struct CullerJobSettings {
    CullerJobSettings() : mThreadMask(-1), mMaxThreads(8), mJobsPerThread(4) {}

    int mThreadMask;
    int mMaxThreads;
    int mJobsPerThread;
} gCullerJobSettings;

// The poll group the jobs run in, created by the first culler. Name not in
// the reference map.
static PollGroup* gCullerPollGroup;  // 0x1A76598

// The jobs of the multithreaded cull, destroyed at exit by 0x45CB60. Name
// not in the reference map.
static RndBasicCullerJob gCullJobs[kNumCullJobs];  // 0x1A765B0

// Moves the jobs that wait for nothing from the list into the group's poll
// buckets, by where they may run and whether jobs wait for them, as the
// poll manager's own queueing does; the group's manager polls them. The
// timer name ("basicculler_firstpass", "basicculler_second_pass") is
// unused: the pass timers are compiled out. Name not in the reference map.
// Reconstructed from eboot.elf at 0x45CDC0.
static void QueueCullJobs(PollGroup* group, CullJobList& jobs, const char* timerName) {
    static_cast<void>(timerName);
    // The group's manager (PollGroup +48, made by its constructor).
    PollMgr* const pollMgr =
        *reinterpret_cast<PollMgr* const*>(reinterpret_cast<const char*>(group) + 48);
    PollDepBase::PollList buckets[PollDepBase::kNumPollBuckets];
    while (!jobs.empty()) {
        // As in the binary, a first job that still waits would spin here;
        // the cull jobs wait for nothing.
        PollDepBase& job = jobs.front();
        if (job._CurDepCount() != 0) {
            continue;
        }
        jobs.remove(job);
        // The buckets as the binary picks them: a job with afters goes in
        // the leaf bucket, the reverse of PollDepBase::PollBucket's notes.
        PollDepBase::PollBucket bucket;
        if ((job.mFlags & PollDepBase::kFlagThreaded) != 0) {
            if (job.mPollWeight > PollMgr::sMaxPollWeight) {
                bucket = PollDepBase::kPollBucketHeavy;
            } else if (job.mNumAfters != 0) {
                bucket = PollDepBase::kPollBucketThreadedLeaf;
            } else {
                bucket = PollDepBase::kPollBucketThreaded;
            }
        } else if (job.mNumAfters != 0) {
            bucket = PollDepBase::kPollBucketMainLeaf;
        } else {
            bucket = PollDepBase::kPollBucketMain;
        }
        buckets[bucket].push_back(job);
        job.mPollMgr = pollMgr;
    }
    // QueueForPoll reads only the lists at the start of the object, which a
    // poll group shares with a manager; the binary passes the group.
    reinterpret_cast<PollMgr*>(group)->QueueForPoll(buckets);
}

// Unlinks the jobs a pass left in the list.
static void ClearCullJobs(CullJobList& jobs) {
    while (!jobs.empty()) {
        jobs.remove(jobs.front());
    }
}

// Reconstructed from eboot.elf at 0x459C20.
RndBasicCuller::RndBasicCuller() {
    if (gCullerPollGroup == nullptr) {
        gCullerPollGroup = new PollGroup("Culler");
    }
}

// Reconstructed from eboot.elf at 0x459DC0.
RndBasicCuller::~RndBasicCuller() {
    RndBasicCuller::Clear();
}

// Reconstructed from eboot.elf at 0x459F00.
void RndBasicCuller::Clear() {
    for (LodInstances& lod : mLods) {
        lod.mInstances.clear();
        lod.mFreeList.clear();
        for (VectorAdapter<RndDrawInstance>& block : lod.mOverflowBlocks) {
            if (block.mData != nullptr) {
                delete[] block.mData;
            }
        }
        lod.mOverflowBlocks.clear();
        lod.mOverflowCount = 0;
    }
    RndBasicCuller::TrimPools();
}

// Reconstructed from eboot.elf at 0x45A120.
void RndBasicCuller::PrepareToGrowBy(const unsigned long* counts) {
    const unsigned long numLods = RndDrawInstanceCom::NumSceneLods();
    for (unsigned long lod = 0; lod != numLods; ++lod) {
        LodInstances& instances = mLods[lod];
        if (!instances.mInstances.empty()) {
            continue;
        }
        const unsigned long reserve =
            static_cast<unsigned long>(static_cast<float>(counts[lod]) * 1.5F);
        instances.mInstances.reserve(reserve);
        instances.mFreeList.reserve(reserve);
    }
}

// Reconstructed from eboot.elf at 0x45A3B0.
void RndBasicCuller::TrimPools() {
    for (RndBasicCullerJob& job : gCullJobs) {
        for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
            PodVector<RndDrawInstance*>& visible = job.mVisible[bucket];
            if (visible.capacity() == 0) {
                continue;
            }
            unsigned long reserve = 0;
            if (visible.size() != 0) {
                reserve = std::max(visible.size(), visible.capacity() * 9 / 10);
            }
            job.mReserveSizes[bucket] = reserve;
            visible.Free();
        }
    }
}

// Reconstructed from eboot.elf at 0x45A490.
void RndBasicCuller::RegisterDrawInstancesForCom(RndDrawInstanceCom& com) {
    VectorAdapter<RndDrawInstance> lists[kNumSceneLods] = {};
    const unsigned long numLods = RndDrawInstanceCom::NumSceneLods();
    for (unsigned long lod = 0; lod != numLods; ++lod) {
        const unsigned long count = com._GetNumDrawInstancesImpl(static_cast<RndSceneLod>(lod));
        if (count == 0) {
            continue;
        }
        LodInstances& instances = mLods[lod];
        RndDrawInstance* first = _AllocFromFreeList(static_cast<RndSceneLod>(lod), count);
        if (first == nullptr) {
            const unsigned long size = instances.mInstances.size();
            if (size + count <= instances.mInstances.capacity()) {
                // The pool's spare capacity: growing within it moves nothing.
                instances.mInstances.resize(size + count);
                first = instances.mInstances.begin() + size;
                for (unsigned long i = 0; i != count; ++i) {
                    first[i] = RndDrawInstance();
                }
            } else {
                first = new RndDrawInstance[count];
                instances.mOverflowBlocks.push_back(VectorAdapter<RndDrawInstance>{first, count});
                instances.mOverflowCount += count;
                instances.mPeakOverflowCount =
                    std::max(instances.mPeakOverflowCount, instances.mOverflowCount);
            }
        }
        lists[lod].mData = first;
        lists[lod].mSize = count;
    }
    com.InitDrawInstances(lists);
}

// Reconstructed from eboot.elf at 0x45AA60.
RndDrawInstance* RndBasicCuller::_AllocFromFreeList(RndSceneLod lod, unsigned long count) {
    eastl::vector<RndDrawInstance*>& freeList = mLods[lod].mFreeList;
    RndDrawInstance** run = freeList.begin();
    while (run != freeList.end()) {
        RndDrawInstance* const first = *run;
        unsigned long length = 1;
        RndDrawInstance** next = run + 1;
        while (length < count && next != freeList.end() && *next == first + length) {
            ++length;
            ++next;
        }
        if (length == count) {
            freeList.erase(run, next);
            for (unsigned long i = 0; i != count; ++i) {
                first[i] = RndDrawInstance();
            }
            return first;
        }
        run = next;
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x45ABF0.
void RndBasicCuller::DeRegisterDrawInstancesForCom(RndDrawInstanceCom& com) {
    for (unsigned long lod = 0; lod != kNumSceneLods; ++lod) {
        const VectorAdapter<RndDrawInstance>& list = com.mRuntime.mDrawInstances[lod];
        const unsigned long count = list.mSize;
        if (count == 0) {
            continue;
        }
        RndDrawInstance* const first = const_cast<RndDrawInstance*>(list.mData);
        LodInstances& instances = mLods[lod];
        if (first >= instances.mInstances.begin() && first < instances.mInstances.end()) {
            const unsigned long index =
                static_cast<unsigned long>(first - instances.mInstances.begin());
            const unsigned long size = instances.mInstances.size();
            if (index + count == size) {
                instances.mInstances.resize(size - count);
                continue;
            }
            // Freed in the middle: hide the instances and keep them for the
            // next registration that fits.
            RndDrawInstance** position = std::lower_bound(
                instances.mFreeList.begin(), instances.mFreeList.end(), first);
            for (unsigned long i = 0; i != count; ++i) {
                first[i].mActive = false;
                position = instances.mFreeList.insert(position, &first[i]) + 1;
            }
            continue;
        }
        VectorAdapter<RndDrawInstance>* block = instances.mOverflowBlocks.begin();
        while (block != instances.mOverflowBlocks.end() && block->mData != first) {
            ++block;
        }
        if (block == instances.mOverflowBlocks.end()) {
            // The report of an unknown component is compiled out.
            com.MakeErrorName();
            continue;
        }
        delete[] block->mData;
        instances.mOverflowBlocks.erase(block);
        // As the binary does, this reads the block moved into the slot (or
        // the stale entry when the erased block was the last).
        instances.mOverflowCount -= block->mSize;
    }
}

// Reconstructed from eboot.elf at 0x45AE90.
void RndBasicCuller::Cull(
    unsigned char* sceneDrawId,
    unsigned int sceneDrawIndex,
    const RndCameraContext& camera,
    const RndShowHideContext& showHide,
    VectorAdapter<PodVector<RndDrawInstance>>& buckets,
    const RndCullerParams& params) {
    // The value is read and ignored; whatever "mt_draw" decided is gone.
    static const unsigned long sMtDraw = DataVarIndex(Symbol("mt_draw"), DataNode(0));
    DataVariable(sMtDraw);
    if (params.mNoCulling || gEntityThreadState.mPollMgr != nullptr) {
        _CullSingleThreaded(sceneDrawId, sceneDrawIndex, camera, showHide, buckets, params);
    } else {
        _CullMultiThreaded(sceneDrawId, camera, showHide, buckets, params);
    }
}

// Reconstructed from eboot.elf at 0x45B080.
void RndBasicCuller::_CullMultiThreaded(
    unsigned char* sceneDrawId,
    const RndCameraContext& camera,
    const RndShowHideContext& showHide,
    VectorAdapter<PodVector<RndDrawInstance>>& buckets,
    const RndCullerParams& params) {
    static_cast<void>(sceneDrawId);
    gCullerPollGroup->Select();
    CullJobList jobs;
    unsigned long numJobs = 0;

    // First pass: split each culled level's pool into at most 16 ranges of
    // at least 100 instances; the first job also takes the overflow blocks.
    // Nothing bounds the job count by the 48 jobs.
    for (unsigned int lod = 0; lod != kNumSceneLods; ++lod) {
        if ((camera.mLodMask & (1U << lod)) == 0) {
            continue;
        }
        if (camera.mLodSettingsSet ? !camera.mLodData[lod].mActive : lod != 0) {
            continue;
        }
        LodInstances& instances = mLods[lod];
        const unsigned long count = instances.mInstances.size();
        bool includeOverflow = !instances.mOverflowBlocks.empty();
        if (count == 0 && !includeOverflow) {
            continue;
        }
        unsigned long chunk = (count >> 4) + ((count & 15) != 0);
        if (chunk <= 100) {
            chunk = 100;
        }
        for (unsigned long begin = 0; includeOverflow || begin < count;) {
            const unsigned long end = std::min(begin + chunk, count);
            RndBasicCullerJob& job = gCullJobs[numJobs++];
            job.mMode = RndBasicCullerJob::kModeCull;
            job.mCuller = this;
            job.mCamera = &camera;
            job.mShowHide = showHide;
            job.mParams = params;
            job.mLod = static_cast<int>(lod);
            job.mBegin = begin;
            job.mEnd = end;
            job.mIncludeOverflow = includeOverflow;
            job.mNumTested = 0;
            job.mNumOverflowTested = 0;
            unsigned int weight = static_cast<unsigned int>((end - begin) / 10);
            job.SetPollWeight(weight != 0 ? weight : 1, false);
            jobs.push_back(job);
            includeOverflow = false;
            begin = end;
        }
    }
    QueueCullJobs(gCullerPollGroup, jobs, "basicculler_firstpass");
    gCullerPollGroup->PollJobs(nullptr);
    ClearCullJobs(jobs);

    // Second pass: each job copies its instances after those of the jobs
    // before it.
    unsigned long offsets[kNumDrawBuckets] = {};
    for (unsigned long i = 0; i != numJobs; ++i) {
        RndBasicCullerJob& job = gCullJobs[i];
        job.mMode = RndBasicCullerJob::kModeCopy;
        job.mOutBuckets = &buckets;
        unsigned long total = 0;
        for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
            job.mOffsets[bucket] = offsets[bucket];
            const unsigned long size = job.mVisible[bucket].size();
            offsets[bucket] += size;
            total += size;
        }
        unsigned int weight = static_cast<unsigned int>(total / 10);
        job.SetPollWeight(weight != 0 ? weight : 1, false);
        jobs.push_back(job);
    }
    for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
        PodVector<RndDrawInstance>& out = Bucket(buckets, bucket);
        out.reserve(offsets[bucket]);
        out.mSize = offsets[bucket];
    }
    QueueCullJobs(gCullerPollGroup, jobs, "basicculler_second_pass");
    gCullerPollGroup->PollJobs(nullptr);
    ClearCullJobs(jobs);
    PollGroup::Deselect();
}

// Reconstructed from eboot.elf at 0x45B6C0.
void RndBasicCuller::_CullSingleThreaded(
    unsigned char* sceneDrawId,
    unsigned int sceneDrawIndex,
    const RndCameraContext& camera,
    const RndShowHideContext& showHide,
    VectorAdapter<PodVector<RndDrawInstance>>& buckets,
    const RndCullerParams& params) {
    // Without LOD settings only the first level is culled, with the
    // camera's frustum; each later level is culled outside the previous
    // level's frustum.
    for (unsigned int lod = 0; lod != kNumSceneLods; ++lod) {
        if ((camera.mLodMask & (1U << lod)) == 0) {
            continue;
        }
        const Frustum* frustum;
        const Frustum* prevFrustum = nullptr;
        if (!camera.mLodSettingsSet) {
            if (lod != 0) {
                continue;
            }
            frustum = &camera.mFrustum;
        } else {
            if (!camera.mLodData[lod].mActive) {
                continue;
            }
            frustum = &camera.mLodData[lod].mFrustum;
            if (lod != 0) {
                prevFrustum = &camera.mLodData[lod - 1].mFrustum;
            }
        }
        if (params.mStateFlagMask == -1) {
            _VisitLodInstances<0>(
                sceneDrawId, sceneDrawIndex, *frustum, prevFrustum, showHide, mLods[lod],
                buckets, params);
        } else {
            _VisitLodInstances<1>(
                sceneDrawId, sceneDrawIndex, *frustum, prevFrustum, showHide, mLods[lod],
                buckets, params);
        }
    }
    for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
        PodVector<RndDrawInstance>& list = Bucket(buckets, bucket);
        UpdateSortDistances(camera, list.begin(), list.end());
    }
}

// Reconstructed from eboot.elf at 0x45B940.
void RndBasicCuller::GetMaxBucketSizes(FixedVector<unsigned long, kNumDrawBuckets>& sizes) const {
    sizes.clear();
    sizes.resize(kNumDrawBuckets);
    for (const RndDrawInstance& instance : mLods[0].mInstances) {
        for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
            if ((instance.mStateFlags & (1U << bucket)) != 0) {
                ++sizes[bucket];
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x45CF60.
void RndBasicCuller::Poll() {}

// Reconstructed from eboot.elf at 0x45C1B0 for N = 0 and 0x45C660 for
// N = 1.
template <unsigned int N>
void RndBasicCuller::_VisitLodInstances(
    unsigned char* sceneDrawId,
    unsigned int sceneDrawIndex,
    const Frustum& frustum,
    const Frustum* prevFrustum,
    const RndShowHideContext& showHide,
    const LodInstances& instances,
    VectorAdapter<PodVector<RndDrawInstance>>& buckets,
    const RndCullerParams& params) {
    static_cast<void>(sceneDrawId);
    static_cast<void>(sceneDrawIndex);
    auto visit = [&](const RndDrawInstance& instance) {
        if (!IsInstanceVisible<N>(instance, showHide, frustum, prevFrustum, params)) {
            return;
        }
        for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
            if ((instance.mStateFlags & (1U << bucket)) != 0) {
                Bucket(buckets, bucket).push_back(instance);
            }
        }
    };
    for (const RndDrawInstance& instance : instances.mInstances) {
        visit(instance);
    }
    for (const VectorAdapter<RndDrawInstance>& block : instances.mOverflowBlocks) {
        for (unsigned long i = 0; i < block.mSize; ++i) {
            visit(block.mData[i]);
        }
    }
}

template void RndBasicCuller::_VisitLodInstances<0>(
    unsigned char*,
    unsigned int,
    const Frustum&,
    const Frustum*,
    const RndShowHideContext&,
    const LodInstances&,
    VectorAdapter<PodVector<RndDrawInstance>>&,
    const RndCullerParams&);
template void RndBasicCuller::_VisitLodInstances<1>(
    unsigned char*,
    unsigned int,
    const Frustum&,
    const Frustum*,
    const RndShowHideContext&,
    const LodInstances&,
    VectorAdapter<PodVector<RndDrawInstance>>&,
    const RndCullerParams&);

RndBasicCullerJob::RndBasicCullerJob()
    : mCuller(nullptr),
      mMode(-1),
      mLod(-1),
      mBegin(~0UL),
      mEnd(~0UL),
      mIncludeOverflow(false),
      mCamera(nullptr),
      mOutBuckets(nullptr),
      mNumTested(0),
      mNumOverflowTested(0) {
    // The draw parameters' default: show everything but flags 0x21.
    mShowHide.mShowFlags = 0;
    mShowHide.mHideFlags = 0x21;
    for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
        mReserveSizes[bucket] = 0;
        mOffsets[bucket] = 0;
    }
}

// Reconstructed from eboot.elf at 0x45E330.
RndBasicCullerJob::~RndBasicCullerJob() {
    for (unsigned long bucket = kNumDrawBuckets; bucket-- != 0;) {
        mVisible[bucket].Free();
    }
}

// Reconstructed from eboot.elf at 0x45CF90.
void RndBasicCullerJob::_DoPoll() {
    if (mMode != kModeCull) {
        _PollCopyMode();
    } else if (mParams.mStateFlagMask != -1) {
        _PollCullMode<1>();
    } else {
        _PollCullMode<0>();
    }
}

// Reconstructed from eboot.elf at 0x45CFC0 for N = 0 and 0x45D710 for
// N = 1.
template <unsigned int N>
void RndBasicCullerJob::_PollCullMode() {
    for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
        mVisible[bucket].reserve(mReserveSizes[bucket]);
    }
    const RndCameraContext& camera = *mCamera;
    const Frustum* frustum = camera.mLodSettingsSet ? &camera.mLodData[mLod].mFrustum
                                                    : &camera.mFrustum;
    const Frustum* prevFrustum = nullptr;
    if (mLod > 0) {
        prevFrustum = camera.mLodSettingsSet ? &camera.mLodData[mLod - 1].mFrustum
                                             : &camera.mFrustum;
    }
    auto visit = [&](RndDrawInstance& instance) {
        if (!IsInstanceVisible<N>(instance, mShowHide, *frustum, prevFrustum, mParams)) {
            return;
        }
        for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
            if ((instance.mStateFlags & (1U << bucket)) != 0) {
                mVisible[bucket].push_back(&instance);
            }
        }
    };
    RndBasicCuller::LodInstances& instances = mCuller->mLods[mLod];
    RndDrawInstance* const pool = instances.mInstances.begin();
    for (unsigned long i = mBegin; i != mEnd; ++i) {
        visit(pool[i]);
    }
    mNumTested += mEnd - mBegin;
    if (!mIncludeOverflow) {
        return;
    }
    for (VectorAdapter<RndDrawInstance>& block : instances.mOverflowBlocks) {
        RndDrawInstance* const data = const_cast<RndDrawInstance*>(block.mData);
        for (unsigned long i = 0; i < block.mSize; ++i) {
            visit(data[i]);
        }
        mNumOverflowTested += block.mSize;
    }
}

template void RndBasicCullerJob::_PollCullMode<0>();
template void RndBasicCullerJob::_PollCullMode<1>();

// Reconstructed from eboot.elf at 0x45DE80.
void RndBasicCullerJob::_PollCopyMode() {
    for (unsigned long bucket = 0; bucket != kNumDrawBuckets; ++bucket) {
        PodVector<RndDrawInstance*>& visible = mVisible[bucket];
        const unsigned long count = visible.size();
        if (count == 0) {
            continue;
        }
        RndDrawInstance* const out = Bucket(*mOutBuckets, bucket).begin() + mOffsets[bucket];
        for (unsigned long i = 0; i != count; ++i) {
            out[i] = *visible[i];
        }
        UpdateSortDistances(*mCamera, out, out + count);
        visible.clear();
    }
}
