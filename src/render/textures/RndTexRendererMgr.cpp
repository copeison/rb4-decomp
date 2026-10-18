// The texture renderer manager (0x6A8150 to 0x6A8B6C). Not in the
// reference map.
#include "render/textures/RndTexRendererMgr.h"

#include "render/context/RndContext.h"
#include "render/system/RndDevice.h"
#include "utl/text/Symbol.h"

namespace {

// The -1, 8, 4 triple of a shared header that every object's static
// initializer sets (here at 0x6A8B70; see RndMeshCom.cpp). Unused here.
// Names not in the reference map.
[[maybe_unused]] unsigned int gNullObjectId = 0xFFFFFFFFU;  // 0x1AAF550
[[maybe_unused]] int gMeshGroupSize2D = 8;                // 0x1AAF554
[[maybe_unused]] int gMeshGroupSize3D = 4;                // 0x1AAF558

// The immediate context, once the device's pending frame has begun. Name
// not in the reference map.
RndContext* ImmediateContext() {
    RndDevice* device = gRndDevice;
    if (device->mBeginFramePending) {
        device->_FlushPendingBeginFrame();
    }
    return device->mImmediateContext;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6A8150.
RndTexRendererMgr::RndTexRendererMgr()
    : mCritSec(), mGroups(), mBeginJob(this), mEndJob(this), mStatBlock(-1) {}

// Reconstructed from eboot.elf at 0x6A8250.
RndTexRendererMgr::~RndTexRendererMgr() {
    for (unsigned long i = 0; i < mGroups.size(); ++i) {
        delete mGroups[i];
    }
    mGroups.clear();
}

// Reconstructed from eboot.elf at 0x6A8360. The renderer's first texture is
// fetched and not used.
RndTexRendererGroup* RndTexRendererMgr::AddRenderer(RndTexture2DRendererCom* renderer) {
    ScopedCritSec lock(mCritSec);
    static_cast<void>(renderer->GetTexture(0));
    RndTexRendererGroup* group = nullptr;
    for (RndTexRendererGroup* candidate : mGroups) {
        if (renderer->CanJoinGroup(*candidate)) {
            group = candidate;
            break;
        }
    }
    if (group == nullptr) {
        group = new RndTexRendererGroup;
        mGroups.push_back(group);
    }
    group->mRenderers.push_back(*renderer);
    return group;
}

// Reconstructed from eboot.elf at 0x6A84F0.
void RndTexRendererMgr::RemoveRenderer(
    RndTexRendererGroup* group,
    RndTexture2DRendererCom* renderer) {
    ScopedCritSec lock(mCritSec);
    group->mRenderers.remove(*renderer);
    if (group->mRenderers.empty()) {
        RndTexRendererGroup** it = mGroups.begin();
        while (it != mGroups.end() && *it != group) {
            ++it;
        }
        mGroups.erase(it);
        delete group;
    }
}

// Reconstructed from eboot.elf at 0x6A85C0.
void RndTexRendererMgr::DrawImmediate() {
    ScopedCritSec lock(mCritSec);
    RndTexRendererGroup** it = mGroups.begin();
    RndTexRendererGroup** end = mGroups.end();
    while (it != end && !(*it)->mDrawPending) {
        ++it;
    }
    if (it == end) {
        return;
    }
    RndContext* context = ImmediateContext();
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Tex Renderers");
    }
    RndScopedGpuStatBlock statBlock(*context, sStatName.Str());
    for (; it != end; ++it) {
        RndTexRendererGroup* group = *it;
        if (group->mDrawPending) {
            group->mRenderers.front().Draw(*context);
            group->mDrawPending = false;
        }
    }
}

// Reconstructed from eboot.elf at 0x6A8740. Unlike the immediate draw, it
// does not take the lock.
void RndTexRendererMgr::AddDrawJobs(
    PollDepBase* head,
    PollDepBase* afterTexRenderers,
    eastl::vector<PollDepBase*>& jobs) {
    RndTexRendererGroup** it = mGroups.begin();
    RndTexRendererGroup** end = mGroups.end();
    while (it != end && !(*it)->mDrawPending) {
        ++it;
    }
    if (it == end) {
        return;
    }
    mBeginJob.AddPollDep(head);
    for (; it != end; ++it) {
        RndTexRendererGroup* group = *it;
        if (group->mDrawPending) {
            group->mDrawPending = false;
            group->mDrawJob.AddPollDep(head);
            group->mExecJob.AddPollDep(&mBeginJob);
            mEndJob.AddPollDep(&group->mExecJob);
            jobs.push_back(&group->mDrawJob);
        }
    }
    afterTexRenderers->AddPollDep(&mEndJob);
}

// Reconstructed from eboot.elf at 0x6A8900.
void RndTexRendererMgr::ClearDrawJobs() {
    mBeginJob.ClearAllDeps();
    mEndJob.ClearAllDeps();
    for (RndTexRendererGroup* group : mGroups) {
        mEndJob.TryRemovePollDep(&group->mExecJob);
    }
}

// Reconstructed from eboot.elf at 0x6A89B0.
void RndTexRendererMgr::BeginJob::ThreadPoll(const int& thread) {
    static_cast<void>(thread);
    RndContext* context = ImmediateContext();
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Tex Renderers");
    }
    mOwner->mStatBlock =
        gRndDevice->mGpuStats.BeginStatBlock(*context, sStatName.Str());
}

// Reconstructed from eboot.elf at 0x6A8AD0.
void RndTexRendererMgr::EndJob::ThreadPoll(const int& thread) {
    static_cast<void>(thread);
    RndContext* context = ImmediateContext();
    gRndDevice->mGpuStats.EndStatBlock(*context, mOwner->mStatBlock);
}
