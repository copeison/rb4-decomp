#pragma once

#include <cstddef>

#include "entity/core/TransEntityResource.h"
#include "entity/resources/ResourceMetaData.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "utl/containers/FixedVector.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"
#include "utl/threading/PollDep.h"

class Entity;
class PollGroup;

// The base of the entity resources the renderer draws, such as
// RndSceneResource (render/RndDrawableEntityResource.o, 0x6C0B80-0x6C14FA;
// the static initializer at 0x6C14B0). It brackets an entity's draw jobs
// between three marker jobs: "head", "after tex renderers" and "tail". The
// draw itself and the texture renderers are its subclass's slots 33-38. The
// map's older build splits this between RndEntityResource and
// RndDrawableEntityResource. The vtable at 0x1937A08 has 39 slots. The
// object is 784 bytes.
class RndDrawableEntityResource : public TransEntityResource {
public:
    // A job that does nothing but order the jobs around it, named for
    // reports ("draw no-op job <name>"). The vtable is at 0x1937B50. Name
    // not in the reference map.
    class NoOpDrawJob : public PollDepBase {
    public:
        // Inlined into the resource's constructor (0x6C0CA0).
        explicit NoOpDrawJob(const char* name) : mName(name) {}
        ~NoOpDrawJob() override;  // slots 0-1: 0x6C0EE0, 0x6C1410

        // Slot 3 at 0x6C1430: nothing.
        void ThreadPoll(const int& thread) override {
            static_cast<void>(thread);
        }
        const char* GetPollName() const override;  // slot 5: 0x6C1440

        Symbol mName;  // Name not in the reference map.
    };

    // The class id, created on first use and inlined into its users. The
    // local static is at 0x19FB668.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("RndDrawableEntityResource");
        }
        return id;
    }

    RndDrawableEntityResource();  // 0x6C0CA0

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x6C1330
    Symbol GetId() const override;                   // slot 1: 0x6C1340
    bool IsA(Symbol type) const override;            // slot 2: 0x6C13E0
    ~RndDrawableEntityResource() override;           // slots 10-11: 0x6C0DD0, 0x6C0EF0
    // Slot 31 at 0x43AC30 (emitted in render/RndSceneResource.o): the
    // entity's poll, post-poll, enter and destroy timers are created.
    bool _WantsPerfTimers() override {
        return true;
    }

    // Slots 33-38, which each class of drawable entity provides. Names not
    // in the reference map.
    // Slot 33: draws the entity now and returns the targets drawn into;
    // `previous` are a previous draw's, or null.
    virtual FixedVector<RndSceneDrawTarget, 2> _Draw(
        Entity* entity,
        RndSceneDrawParams& params,
        const FixedVector<RndSceneDrawTarget, 2>* previous) = 0;
    // Slot 34: hooks the entity's draw jobs in after `after`'s, between
    // the start and context jobs and the end job, and adds them to `jobs`.
    virtual void _StartDrawJobs(
        Entity* entity,
        const RndSceneDrawParams& params,
        Entity* after,
        PollDepBase* startDep,
        PollDepBase* contextDep,
        PollDepBase* endAfter,
        eastl::vector<PollDepBase*>& jobs) = 0;
    // Slot 35: unhooks them and returns the targets drawn into.
    virtual FixedVector<RndSceneDrawTarget, 2> _FinishDrawJobs(Entity* entity) = 0;
    // Slots 36-38: draw the entity's texture renderers now, hook their
    // jobs in between the head and the "after tex renderers" jobs, and
    // unhook them.
    virtual void _DrawTexRenderers(Entity* entity) = 0;
    virtual void _StartTexRendererJobs(
        Entity* entity,
        PollDepBase* head,
        PollDepBase* afterTexRenderers,
        eastl::vector<PollDepBase*>& jobs) = 0;
    virtual void _FinishTexRendererJobs(Entity* entity, PollDepBase* afterTexRenderers) = 0;

    // Draws the entity: on the calling thread through slots 36 and 33, or,
    // when the renderer draws with jobs and the parameters allow it, through
    // the jobs of slots 34, 35, 37 and 38 run under the "draw" poll group.
    // The map's RndEntityResource::Draw(EntityPtr, RndSceneDrawParams&).
    // Not reconstructed: the poll group's queueing (0x6C1150) and the
    // parameter checks it inlines are not modelled.
    FixedVector<RndSceneDrawTarget, 2> Draw(Entity* entity, RndSceneDrawParams& params);  // 0x6C0F10
    // Public forms of slots 33-38, which game code calls (0x8AF5E9 on).
    // Names not in the reference map.
    FixedVector<RndSceneDrawTarget, 2> DrawImmediate(
        Entity* entity,
        RndSceneDrawParams& params,
        const FixedVector<RndSceneDrawTarget, 2>* previous);  // 0x6C1310
    void StartDrawJobs(
        Entity* entity,
        const RndSceneDrawParams& params,
        Entity* after,
        PollDepBase* startDep,
        PollDepBase* contextDep,
        PollDepBase* endAfter,
        eastl::vector<PollDepBase*>& jobs);  // 0x6C1140
    FixedVector<RndSceneDrawTarget, 2> FinishDrawJobs(Entity* entity);  // 0x6C12E0
    void DrawTexRenderers(Entity* entity);  // 0x6C1300
    void StartTexRendererJobs(
        Entity* entity,
        PollDepBase* head,
        PollDepBase* afterTexRenderers,
        eastl::vector<PollDepBase*>& jobs);  // 0x6C1130
    void FinishTexRendererJobs(Entity* entity, PollDepBase* afterTexRenderers);  // 0x6C12D0

    // Registers the class: "RndDrawableEntityResource" under
    // "TransEntityResource". The map's Init().
    static void Init();  // 0x6C0B80

    static ResourceMetaData sMetaData;  // 0x1AB0308

    // Field names are not in the reference map.
    // The "draw" poll group the job draw runs under, created by the first.
    PollGroup* mDrawPollGroup;
    // Cleared by the constructor; no reader was found.
    bool mReserved;
    NoOpDrawJob mHeadJob;               // "head"
    NoOpDrawJob mAfterTexRenderersJob;  // "after tex renderers"
    NoOpDrawJob mTailJob;               // "tail"
    // The jobs of the current job draw.
    eastl::vector<PollDepBase*> mDrawJobs;
};

static_assert(offsetof(RndDrawableEntityResource::NoOpDrawJob, mName) == 0x90);
static_assert(sizeof(RndDrawableEntityResource::NoOpDrawJob) == 0x98);
static_assert(offsetof(RndDrawableEntityResource, mDrawPollGroup) == 0x118);
static_assert(offsetof(RndDrawableEntityResource, mReserved) == 0x120);
static_assert(offsetof(RndDrawableEntityResource, mHeadJob) == 0x128);
static_assert(offsetof(RndDrawableEntityResource, mAfterTexRenderersJob) == 0x1C0);
static_assert(offsetof(RndDrawableEntityResource, mTailJob) == 0x258);
static_assert(offsetof(RndDrawableEntityResource, mDrawJobs) == 0x2F0);
static_assert(sizeof(RndDrawableEntityResource) == 0x310);
