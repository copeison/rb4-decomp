#pragma once

#include <cstddef>

#include "os/threading/CritSec.h"
#include "render/context/RndDeferredContextJobs.h"
#include "render/textures/RndTexture2DRendererCom.h"
#include "utl/containers/LinkedList.h"
#include "utl/containers/Vector.h"
#include "utl/threading/PollDep.h"

class RndBufferCollection;

// The texture renderers that draw with one set of buffers (352 bytes,
// emitted with RndTexture2DRendererCom). The first renderer draws for the
// group: immediately, or on a deferred context through the group's draw
// job. Name not in the reference map.
class RndTexRendererGroup {
public:
    // Draws the group's first renderer on a deferred context (vtable
    // 0x1934E10). Name not in the reference map.
    class DrawJob : public RndDeferredContextDrawJob {
    public:
        ~DrawJob() override;  // slots 0-1: 0x694980, 0x694D80
        void ThreadPoll(const int& thread) override;  // slot 3: 0x6949D0

        RndTexRendererGroup* mGroup;
    };

    RndTexRendererGroup();   // 0x6948B0
    ~RndTexRendererGroup();  // 0x694930

    // Set when a renderer asks the group to draw this frame; the manager
    // clears it as it draws or queues the group.
    bool mDrawPending;
    // The RndBufferCollection2D the renderers draw into, owned.
    RndBufferCollection* mBuffers;
    LinkedList::List<RndTexture2DRendererCom, RndTexture2DRendererCom::GroupNode> mRenderers;
    RndExecDeferredContextJob mExecJob;
    DrawJob mDrawJob;
};

static_assert(offsetof(RndTexRendererGroup, mRenderers) == 16);
static_assert(offsetof(RndTexRendererGroup, mExecJob) == 32);
static_assert(offsetof(RndTexRendererGroup, mDrawJob) == 192);
static_assert(offsetof(RndTexRendererGroup, mDrawJob.mGroup) == 344);
static_assert(sizeof(RndTexRendererGroup) == 352);

// The texture renderers of a drawable entity (0x6A8150-0x6A8B6C; the static
// initializer at 0x6A8B70), drawn under the "Tex Renderers" GPU statistic
// block before the entity's own draw. A drawable entity's root component
// creates one on demand (RndDrawableEntityCom::ObtainTexRendererMgr). The
// renderers are kept in groups that share buffers; two jobs bracket the
// groups' threaded draws with the statistic block. The object is 360
// bytes. Names not in the reference map; the class name is inferred from
// the statistic name, and the evidence is weak.
class RndTexRendererMgr {
public:
    // Opens the statistic block before the groups' draw jobs (vtable
    // 0x1936328, emitted constructor 0x6A8970).
    class BeginJob : public PollDepBase {
    public:
        explicit BeginJob(RndTexRendererMgr* owner) : mOwner(owner) {
            SetThreaded(false);
        }
        ~BeginJob() override {}  // slots 0-1: 0x6A8350, 0x6A8B30
        void ThreadPoll(const int& thread) override;  // slot 3: 0x6A89B0

        RndTexRendererMgr* mOwner;
    };

    // Closes the statistic block after the groups' draw jobs (vtable
    // 0x1936388, emitted constructor 0x6A8A90).
    class EndJob : public PollDepBase {
    public:
        explicit EndJob(RndTexRendererMgr* owner) : mOwner(owner) {
            SetThreaded(false);
        }
        ~EndJob() override {}  // slots 0-1: 0x6A8340, 0x6A8B50
        void ThreadPoll(const int& thread) override;  // slot 3: 0x6A8AD0

        RndTexRendererMgr* mOwner;
    };

    RndTexRendererMgr();   // 0x6A8150
    // Deletes the groups.
    ~RndTexRendererMgr();  // 0x6A8250

    // Puts the renderer in the first group it can join, or in a new group,
    // and returns the group.
    RndTexRendererGroup* AddRenderer(RndTexture2DRendererCom* renderer);  // 0x6A8360
    // Takes the renderer out of its group and deletes the group when it
    // is left empty.
    void RemoveRenderer(
        RndTexRendererGroup* group,
        RndTexture2DRendererCom* renderer);  // 0x6A84F0
    // Draws the groups waiting to draw on the immediate context.
    void DrawImmediate();  // 0x6A85C0
    // Queues the draw jobs of the groups waiting to draw after `head`,
    // between the begin and end jobs, before `afterTexRenderers`, and adds
    // them to `jobs`.
    void AddDrawJobs(
        PollDepBase* head,
        PollDepBase* afterTexRenderers,
        eastl::vector<PollDepBase*>& jobs);  // 0x6A8740
    // Clears the dependencies AddDrawJobs created.
    void ClearDrawJobs();  // 0x6A8900

    CritSec mCritSec;
    eastl::vector<RndTexRendererGroup*> mGroups;
    BeginJob mBeginJob;
    EndJob mEndJob;
    // The statistic block the begin job opened, or -1.
    long mStatBlock;
};

static_assert(offsetof(RndTexRendererMgr, mGroups) == 16);
static_assert(offsetof(RndTexRendererMgr, mBeginJob) == 48);
static_assert(offsetof(RndTexRendererMgr, mEndJob) == 200);
static_assert(offsetof(RndTexRendererMgr, mStatBlock) == 352);
static_assert(sizeof(RndTexRendererMgr) == 360);
