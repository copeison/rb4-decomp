#pragma once

#include <cstddef>

#include "render/context/RndDeferredContextJobs.h"
#include "render/drawing/PodVector.h"
#include "render/drawing/RndDrawInstance.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "utl/containers/FixedVector.h"
#include "utl/containers/Vector.h"
#include "utl/threading/PollDep.h"

class RndDynamicGpuData;
class RndSceneDrawer;
class RndSceneDrawJobs;
struct RndSceneInternalContext;

// The jobs of the scene drawer's job graph (RndSceneDrawJobs). Each holds
// its drawer and works on the drawer's threaded context
// (RndSceneDrawer::mThreadedContext). The jobs that draw on the
// immediate context poll on the main thread; the bucket, dynamic GPU data
// and shadow map jobs draw into a deferred context on a worker, and their
// RndExecDeferredContextJob executes it. They are emitted in
// render/RndSceneDrawer.o with their vtables at 0x1900878-0x1900E78 (ten
// slots each). The map predates the job graph: names not in the reference
// map; they follow the jobs' report names.

// "draw start": the graph's first job, which waits for the caller's start
// dependency. The vtable is at 0x1900878.
class RndSceneDrawStartJob : public PollDepBase {
public:
    explicit RndSceneDrawStartJob(RndSceneDrawer* drawer)  // 0x432440
        : mDrawer(drawer) {}
    // Slots 0-1: 0x4343E0, 0x4343F0.
    ~RndSceneDrawStartJob() override {}
    // Slot 3 at 0x432470: empty.
    void ThreadPoll(const int& thread) override {
        static_cast<void>(thread);
    }
    // Slot 5 at 0x434410.
    const char* GetPollName() const override {
        return "draw start";
    }

    RndSceneDrawer* mDrawer;
};

static_assert(sizeof(RndSceneDrawStartJob) == 152);

// "draw init immed context": begins the immediate context's frame and
// uploads the scene's shader graph globals and property hysteresis. The
// vtable is at 0x19008D8.
class RndSceneDrawInitContextJob : public PollDepBase {
public:
    explicit RndSceneDrawInitContextJob(RndSceneDrawer* drawer)  // 0x432480
        : mDrawer(drawer), mPollState(-1) {
        SetThreaded(false);
    }
    // Slots 0-1: 0x434420, 0x434430.
    ~RndSceneDrawInitContextJob() override {}
    void ThreadPoll(const int& thread) override;  // slot 3 at 0x4324C0
    // Slot 5 at 0x434450.
    const char* GetPollName() const override {
        return "draw init immed context";
    }

    RndSceneDrawer* mDrawer;
    // Set to -1 by the constructor and by every poll and never read. The
    // name is weak.
    int mPollState;
};

static_assert(offsetof(RndSceneDrawInitContextJob, mPollState) == 152);
static_assert(sizeof(RndSceneDrawInitContextJob) == 160);

// The graph's last job, which the caller's end job waits for. It keeps
// the default report name. The vtable is at 0x1900938.
class RndSceneDrawEndJob : public PollDepBase {
public:
    RndSceneDrawEndJob(RndSceneDrawer* drawer, RndSceneDrawJobs* jobs)  // 0x432560
        : mDrawer(drawer), mJobs(jobs) {
        SetThreaded(false);
    }
    // Slots 0-1: 0x434460, 0x434470.
    ~RndSceneDrawEndJob() override {}
    void ThreadPoll(const int& thread) override;  // slot 3 at 0x4325B0

    RndSceneDrawer* mDrawer;
    RndSceneDrawJobs* mJobs;
};

static_assert(sizeof(RndSceneDrawEndJob) == 160);

// "draw cull": culls one collection's camera and submits its occlusion
// queries. The vtable is at 0x1900998.
class RndSceneDrawCullJob : public PollDepBase {
public:
    RndSceneDrawCullJob(RndSceneDrawer* drawer, unsigned long index)  // 0x4325D0
        : mDrawer(drawer), mIndex(index), mSceneDrawId(nullptr) {
        SetPollWeight(0xFFFF, false);
    }
    // Slots 0-1: 0x434490, 0x4344A0.
    ~RndSceneDrawCullJob() override {}
    void ThreadPoll(const int& thread) override;  // slot 3 at 0x432630
    // Slot 5 at 0x4344C0.
    const char* GetPollName() const override {
        return "draw cull";
    }

    RndSceneDrawer* mDrawer;
    // The collection's index in the parameters.
    unsigned long mIndex;
    // The immediate context's RndContext::mSceneDrawId, set by Start.
    unsigned char* mSceneDrawId;
};

static_assert(offsetof(RndSceneDrawCullJob, mSceneDrawId) == 160);
static_assert(sizeof(RndSceneDrawCullJob) == 168);

// "draw cull analysis": analyses one collection's buckets and works out
// the draw target every job of the collection draws with. The vtable is
// at 0x19009F8.
class RndSceneDrawCullAnalysisJob : public PollDepBase {
public:
    RndSceneDrawCullAnalysisJob(
        RndSceneDrawer* drawer,
        RndSceneDrawJobs* jobs,
        unsigned long index)  // 0x432740
        : mDrawer(drawer), mJobs(jobs), mIndex(index) {
        SetPollWeight(0xFFFF, false);
    }
    // Slots 0-1: 0x4344D0, 0x4344E0.
    ~RndSceneDrawCullAnalysisJob() override {}
    void ThreadPoll(const int& thread) override;  // slot 3 at 0x4327E0
    // Slot 5 at 0x434500.
    const char* GetPollName() const override {
        return "draw cull analysis";
    }

    RndSceneDrawer* mDrawer;
    RndSceneDrawJobs* mJobs;
    unsigned long mIndex;
    // The collection's target as the graph's last job leaves it: Finish
    // returns it, and the next graph's analysis starts from it.
    RndSceneDrawTarget mTarget;
};

static_assert(offsetof(RndSceneDrawCullAnalysisJob, mTarget) == 168);
static_assert(sizeof(RndSceneDrawCullAnalysisJob) == 216);

// "draw post-cull": the scene texture capture, the depth clear and the
// scene mask that come before the opaque buckets. The vtable is at
// 0x1900A58.
class RndSceneDrawPostCullJob : public PollDepBase {
public:
    explicit RndSceneDrawPostCullJob(RndSceneDrawer* drawer)  // 0x432870
        : mDrawer(drawer) {
        SetThreaded(false);
    }
    // Slots 0-1: 0x434510, 0x434520.
    ~RndSceneDrawPostCullJob() override {}
    void ThreadPoll(const int& thread) override;  // slot 3 at 0x4328F0
    // Slot 5 at 0x434540.
    const char* GetPollName() const override {
        return "draw post-cull";
    }

    RndSceneDrawer* mDrawer;
    RndSceneDrawTarget mTarget;
};

static_assert(offsetof(RndSceneDrawPostCullJob, mTarget) == 152);
static_assert(sizeof(RndSceneDrawPostCullJob) == 200);

// "draw bucket": draws one bucket of a collection into a deferred context.
// The vtable is at 0x1900AB8.
class RndSceneDrawBucketJob : public RndDeferredContextDrawJob {
public:
    RndSceneDrawBucketJob(
        RndExecDeferredContextJob* execJob,
        RndSceneDrawer* drawer,
        int bucket)  // 0x432B90
        : RndDeferredContextDrawJob(execJob), mDrawer(drawer), mBucket(bucket) {
        SetPollWeight(0xFFFF, false);
    }
    // Slots 0-1: 0x434550, 0x434560.
    ~RndSceneDrawBucketJob() override {}
    void ThreadPoll(const int& thread) override;  // slot 3 at 0x432C20
    // Slot 5 at 0x434580.
    const char* GetPollName() const override {
        return "draw bucket";
    }

    RndSceneDrawer* mDrawer;
    int mBucket;
    RndSceneDrawTarget mTarget;
};

static_assert(offsetof(RndSceneDrawBucketJob, mBucket) == 160);
static_assert(offsetof(RndSceneDrawBucketJob, mTarget) == 168);
static_assert(sizeof(RndSceneDrawBucketJob) == 216);

// "draw light culling": culls the lights once every collection's cull
// analysis is done and hands the shadow lights to the shadow map jobs.
// The vtable is at 0x1900B18.
class RndSceneDrawLightCullingJob : public PollDepBase {
public:
    RndSceneDrawLightCullingJob(RndSceneDrawer* drawer, RndSceneDrawJobs* jobs)  // 0x432EE0
        : mDrawer(drawer), mJobs(jobs), mSceneDrawId(nullptr) {
        SetPollWeight(0xFFFF, false);
    }
    // Slots 0-1: 0x434590, 0x4345A0.
    ~RndSceneDrawLightCullingJob() override {}
    void ThreadPoll(const int& thread) override;  // slot 3 at 0x432F40
    // Slot 5 at 0x4345C0.
    const char* GetPollName() const override {
        return "draw light culling";
    }

    RndSceneDrawer* mDrawer;
    RndSceneDrawJobs* mJobs;
    // The immediate context's RndContext::mSceneDrawId, set by Start.
    unsigned char* mSceneDrawId;
};

static_assert(offsetof(RndSceneDrawLightCullingJob, mSceneDrawId) == 160);
static_assert(sizeof(RndSceneDrawLightCullingJob) == 168);

// "draw tiled light culling": generates every collection's linear depth
// and culls the lights into tiles on the immediate context. The vtable is
// at 0x1900B78.
class RndSceneDrawTiledLightCullingJob : public PollDepBase {
public:
    explicit RndSceneDrawTiledLightCullingJob(RndSceneDrawer* drawer)  // 0x4330A0
        : mDrawer(drawer) {
        SetThreaded(false);
    }
    // Slots 0-1: 0x4345D0, 0x4345E0.
    ~RndSceneDrawTiledLightCullingJob() override {}
    void ThreadPoll(const int& thread) override;  // slot 3 at 0x4330E0
    // Slot 5 at 0x434600.
    const char* GetPollName() const override {
        return "draw tiled light culling";
    }

    RndSceneDrawer* mDrawer;
};

static_assert(sizeof(RndSceneDrawTiledLightCullingJob) == 152);

// "draw post-light-cull": the stereo tiled light culling, the sky texture,
// the start of the volumetric scattering and the ambient occlusion
// (perf timer "draw_post_light_cull_job: <index>"). The vtable is at
// 0x1900BD8.
class RndSceneDrawPostLightCullJob : public PollDepBase {
public:
    RndSceneDrawPostLightCullJob(RndSceneDrawer* drawer, unsigned long index);  // 0x433360
    // Slots 0-1: 0x434610, 0x434620.
    ~RndSceneDrawPostLightCullJob() override {}
    void ThreadPoll(const int& thread) override;  // slot 3 at 0x433470
    // Slot 5 at 0x434640.
    const char* GetPollName() const override {
        return "draw post-light-cull";
    }

    RndSceneDrawer* mDrawer;
    RndSceneDrawTarget mTarget;
    // The job's perf timer; nothing in this build reads it.
    unsigned long mTimerIndex;
};

static_assert(offsetof(RndSceneDrawPostLightCullJob, mTarget) == 152);
static_assert(offsetof(RndSceneDrawPostLightCullJob, mTimerIndex) == 200);
static_assert(sizeof(RndSceneDrawPostLightCullJob) == 208);

// "draw deferred light": the deferred lighting, the fog or the volumetric
// scattering, and the scene texture capture before the forward buckets
// (perf timer "draw_deferred_light_job: <index>"). The vtable is at
// 0x1900C38.
class RndSceneDrawDeferredLightJob : public PollDepBase {
public:
    RndSceneDrawDeferredLightJob(RndSceneDrawer* drawer, unsigned long index);  // 0x4336B0
    // Slots 0-1: 0x434650, 0x434660.
    ~RndSceneDrawDeferredLightJob() override {}
    void ThreadPoll(const int& thread) override;  // slot 3 at 0x4337C0
    // Slot 5 at 0x434680.
    const char* GetPollName() const override {
        return "draw deferred light";
    }

    RndSceneDrawer* mDrawer;
    RndSceneDrawTarget mTarget;
    // The job's perf timer; nothing in this build reads it.
    unsigned long mTimerIndex;
};

static_assert(offsetof(RndSceneDrawDeferredLightJob, mTarget) == 152);
static_assert(offsetof(RndSceneDrawDeferredLightJob, mTimerIndex) == 200);
static_assert(sizeof(RndSceneDrawDeferredLightJob) == 208);

// "draw submit occ queries": draws a collection's occlusion queries. The
// vtable is at 0x1900C98.
class RndSceneDrawSubmitOccQueriesJob : public PollDepBase {
public:
    RndSceneDrawSubmitOccQueriesJob(RndSceneDrawer* drawer, unsigned long index)  // 0x4339C0
        : mDrawer(drawer), mIndex(index) {
        SetThreaded(false);
    }
    // Slots 0-1: 0x434690, 0x4346A0.
    ~RndSceneDrawSubmitOccQueriesJob() override {}
    void ThreadPoll(const int& thread) override;  // slot 3 at 0x433A10
    // Slot 5 at 0x4346C0.
    const char* GetPollName() const override {
        return "draw submit occ queries";
    }

    RndSceneDrawer* mDrawer;
    unsigned long mIndex;
};

static_assert(sizeof(RndSceneDrawSubmitOccQueriesJob) == 160);

// "draw capture scenetex": captures the scene texture for the materials
// that read it at its level. The vtable is at 0x1900CF8.
class RndSceneDrawCaptureSceneTexJob : public PollDepBase {
public:
    RndSceneDrawCaptureSceneTexJob(RndSceneDrawer* drawer, int sceneTexUsage)  // 0x433AB0
        : mDrawer(drawer), mSceneTexUsage(sceneTexUsage) {
        SetThreaded(false);
    }
    // Slots 0-1: 0x4346D0, 0x4346E0.
    ~RndSceneDrawCaptureSceneTexJob() override {}
    void ThreadPoll(const int& thread) override;  // slot 3 at 0x433B40
    // Slot 5 at 0x434700.
    const char* GetPollName() const override {
        return "draw capture scenetex";
    }

    RndSceneDrawer* mDrawer;
    RndSceneDrawTarget mTarget;
    // The CameraData::mSceneTexUsage it captures for; the graph passes 2.
    int mSceneTexUsage;
};

static_assert(offsetof(RndSceneDrawCaptureSceneTexJob, mSceneTexUsage) == 200);
static_assert(sizeof(RndSceneDrawCaptureSceneTexJob) == 208);

// "draw postproc": one of a collection's two post-processing chains. The
// vtable is at 0x1900D58.
class RndSceneDrawPostProcJob : public PollDepBase {
public:
    // The collection index is not kept.
    RndSceneDrawPostProcJob(
        RndSceneDrawer* drawer,
        unsigned long collection,
        unsigned long index)  // 0x433BE0
        : mDrawer(drawer), mIndex(index) {
        static_cast<void>(collection);
        SetThreaded(false);
    }
    // Slots 0-1: 0x434710, 0x434720.
    ~RndSceneDrawPostProcJob() override {}
    void ThreadPoll(const int& thread) override;  // slot 3 at 0x433C70
    // Slot 5 at 0x434740.
    const char* GetPollName() const override {
        return "draw postproc";
    }

    RndSceneDrawer* mDrawer;
    RndSceneDrawTarget mTarget;
    unsigned long mIndex;
};

static_assert(offsetof(RndSceneDrawPostProcJob, mIndex) == 200);
static_assert(sizeof(RndSceneDrawPostProcJob) == 208);

// "draw sync dynamic gpu data": refreshes its share of the drawer's dynamic
// GPU data queue (RndDynamicGpuDataMgr::SplitQueue) into a deferred
// context. The vtable is at 0x1900DB8.
class RndSceneDrawSyncJob : public RndDeferredContextDrawJob {
public:
    RndSceneDrawSyncJob(RndExecDeferredContextJob* execJob, RndSceneDrawer* drawer)  // 0x433D10
        : RndDeferredContextDrawJob(execJob), mDrawer(drawer) {
        SetPollWeight(0xFFFF, false);
    }
    // Slots 0-1: 0x434750, 0x4347A0.
    ~RndSceneDrawSyncJob() override {}
    void ThreadPoll(const int& thread) override;  // slot 3 at 0x433D80
    // Slot 5 at 0x4347F0.
    const char* GetPollName() const override {
        return "draw sync dynamic gpu data";
    }

    RndSceneDrawer* mDrawer;
    eastl::vector<RndDynamicGpuData*> mQueue;
};

static_assert(offsetof(RndSceneDrawSyncJob, mQueue) == 160);
static_assert(sizeof(RndSceneDrawSyncJob) == 192);

// "draw shadowmap (exec <context>)": draws the shadow maps of a range of
// the culled shadow lights into a deferred context, culling into its own
// bucket set. The light culling job hands out the ranges. The vtable is
// at 0x1900E18.
class RndSceneDrawShadowMapJob : public RndDeferredContextDrawJob {
public:
    RndSceneDrawShadowMapJob(RndExecDeferredContextJob* execJob, RndSceneDrawer* drawer);  // 0x433E50
    // Slots 0-1: 0x434800, 0x4348B0.
    ~RndSceneDrawShadowMapJob() override;
    void ThreadPoll(const int& thread) override;  // slot 3 at 0x434000
    const char* GetPollName() const override;     // slot 5 at 0x434130

    RndSceneDrawer* mDrawer;
    // The shadow lights [mFirstLight, mLastLight) to draw, or -1 for none.
    unsigned long mFirstLight;
    unsigned long mLastLight;
    FixedVector<PodVector<RndDrawInstance>, kNumDrawBuckets> mDrawInstances;
    FixedVector<PodVector<RndDrawInstance*>, kNumDrawBuckets> mSortableInstances;
};

static_assert(offsetof(RndSceneDrawShadowMapJob, mFirstLight) == 160);
static_assert(offsetof(RndSceneDrawShadowMapJob, mDrawInstances) == 176);
static_assert(offsetof(RndSceneDrawShadowMapJob, mSortableInstances) == 728);
static_assert(sizeof(RndSceneDrawShadowMapJob) == 1280);

// The last job of a collection: the antialiasing, the debug displays, the
// overlays, the output conversion and the buffer inspection (perf timer
// "draw_end_collection_job: <index>"). The vtable is at 0x1900E78.
class RndSceneDrawEndCollectionJob : public PollDepBase {
public:
    RndSceneDrawEndCollectionJob(RndSceneDrawer* drawer, unsigned long index);  // 0x4341B0
    // Slots 0-1: 0x434970, 0x434980.
    ~RndSceneDrawEndCollectionJob() override {}
    void ThreadPoll(const int& thread) override;  // slot 3 at 0x4342C0
    // Slot 5 at 0x4349A0.
    const char* GetPollName() const override {
        return "draw end";
    }

    RndSceneDrawer* mDrawer;
    RndSceneDrawTarget mTarget;
    // The job's perf timer; nothing in this build reads it.
    unsigned long mTimerIndex;
};

static_assert(offsetof(RndSceneDrawEndCollectionJob, mTarget) == 152);
static_assert(offsetof(RndSceneDrawEndCollectionJob, mTimerIndex) == 200);
static_assert(sizeof(RndSceneDrawEndCollectionJob) == 208);

// The scene drawer's job graph for the multithreaded draw (1168 bytes):
// the passes of _DrawFullFramerate split into jobs that run on the
// immediate context and on deferred contexts. Built once by the drawer
// (RndSceneDrawer::mDrawJobs) and hooked into the caller's graph each
// frame by Start. It is emitted in render/RndSceneDrawer.o
// (0x430610-0x4343DF, the jobs' members to 0x4349DF). Name and field
// names not in the reference map.
class RndSceneDrawJobs {
public:
    // A draw job and the job that executes its deferred context. Name not
    // in the reference map.
    template <typename T>
    struct DeferredJob {
        T* mJob;
        RndExecDeferredContextJob* mExecJob;
    };

    // The jobs of one buffer collection (480 bytes). Name not in the
    // reference map.
    struct CollectionJobs {
        RndSceneDrawCullJob* mCullJob = nullptr;
        RndSceneDrawCullAnalysisJob* mCullAnalysisJob = nullptr;
        RndSceneDrawPostCullJob* mPostCullJob = nullptr;
        RndSceneDrawPostLightCullJob* mPostLightCullJob = nullptr;
        RndSceneDrawDeferredLightJob* mDeferredLightJob = nullptr;
        RndSceneDrawSubmitOccQueriesJob* mSubmitOccQueriesJob = nullptr;
        RndSceneDrawCaptureSceneTexJob* mCaptureSceneTexJob = nullptr;
        // Before the transparent buckets 19 and 20.
        FixedVector<RndSceneDrawPostProcJob*, 2> mPostProcJobs;
        RndSceneDrawEndCollectionJob* mEndCollectionJob = nullptr;
        // A job per bucket; buckets 2, 3 and 21 have none.
        FixedVector<DeferredJob<RndSceneDrawBucketJob>, kNumDrawBuckets> mBucketJobs;
    };

    // Builds the graph for the drawer; the drawer passes 1.
    RndSceneDrawJobs(RndSceneDrawer* drawer, unsigned long numCollections);  // 0x430610
    ~RndSceneDrawJobs();  // 0x4314B0

    // Hooks the graph after `after`'s graph, after `startDep` and
    // `contextDep`, before `endAfter`, and appends its threaded jobs to
    // `jobs`. The context is not read: the jobs use the drawer's.
    void Start(
        RndSceneInternalContext& context,
        RndSceneDrawJobs* after,
        PollDepBase* startDep,
        PollDepBase* contextDep,
        PollDepBase* endAfter,
        eastl::vector<PollDepBase*>& jobs);  // 0x4317E0
    // Unhooks the graph and returns the draw targets of its collections.
    FixedVector<RndSceneDrawTarget, 2> Finish();  // 0x431970
    // The target a collection's jobs draw with: the previous graph's, or a
    // fresh one, handed down the collection's jobs in bucket order with the
    // changes each stage makes (the scene texture, the deferred lighting,
    // the post-processing and the debug display). Called by the cull
    // analysis job.
    RndSceneDrawTarget _PropagateDrawTarget(
        const RndSceneInternalContext& internal,
        unsigned long index);  // 0x431AB0

    RndSceneDrawStartJob* mStartJob;
    RndSceneDrawInitContextJob* mInitContextJob;
    RndSceneDrawLightCullingJob* mLightCullingJob;
    RndSceneDrawTiledLightCullingJob* mTiledLightCullingJob;
    RndSceneDrawEndJob* mEndJob;
    // A dynamic GPU data job per deferred context, and their execute jobs.
    eastl::vector<RndSceneDrawSyncJob*> mSyncJobs;
    eastl::vector<RndExecDeferredContextJob*> mSyncExecJobs;
    FixedVector<CollectionJobs, 2> mCollections;
    // RndConfig::mMaxShadowContribBuffers shadow map jobs.
    eastl::vector<DeferredJob<RndSceneDrawShadowMapJob>> mShadowMapJobs;
    RndSceneDrawer* mDrawer;
    // The graph this one follows this frame, from Start.
    RndSceneDrawJobs* mAfter;
    // The jobs that draw on deferred contexts, for the caller.
    eastl::vector<PollDepBase*> mJobs;
};

static_assert(offsetof(RndSceneDrawJobs::CollectionJobs, mPostProcJobs) == 56);
static_assert(offsetof(RndSceneDrawJobs::CollectionJobs, mEndCollectionJob) == 96);
static_assert(offsetof(RndSceneDrawJobs::CollectionJobs, mBucketJobs) == 104);
static_assert(sizeof(RndSceneDrawJobs::CollectionJobs) == 480);
static_assert(offsetof(RndSceneDrawJobs, mSyncJobs) == 40);
static_assert(offsetof(RndSceneDrawJobs, mSyncExecJobs) == 72);
static_assert(offsetof(RndSceneDrawJobs, mCollections) == 104);
static_assert(offsetof(RndSceneDrawJobs, mShadowMapJobs) == 1088);
static_assert(offsetof(RndSceneDrawJobs, mDrawer) == 1120);
static_assert(offsetof(RndSceneDrawJobs, mAfter) == 1128);
static_assert(offsetof(RndSceneDrawJobs, mJobs) == 1136);
static_assert(sizeof(RndSceneDrawJobs) == 1168);
