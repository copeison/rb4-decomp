// render/RndDrawableEntityResource.o (0x6C0B80 to 0x6C14FA).
#include "render/scene/RndDrawableEntityResource.h"

#include "utl/text/MakeString.h"
#include "utl/threading/PollMgr.h"

// The object's static, built by its static initializer (0x6C14B0). The
// three ints it first sets (0x1AB02F8) come from a shared header and are not
// modelled.
ResourceMetaData RndDrawableEntityResource::sMetaData;

// Reconstructed from eboot.elf at 0x6C0B80.
void RndDrawableEntityResource::Init() {
    sMetaData.mTypeFlags[0] = false;
    sMetaData.Init(Id(), TransEntityResource::Id(), false);
}

// Reconstructed from eboot.elf at 0x6C0CA0.
RndDrawableEntityResource::RndDrawableEntityResource()
    : mDrawPollGroup(nullptr),
      mReserved(false),
      mHeadJob("head"),
      mAfterTexRenderersJob("after tex renderers"),
      mTailJob("tail") {}

// Reconstructed from eboot.elf at 0x6C0DD0. The deleting destructor is at
// 0x6C0EF0.
RndDrawableEntityResource::~RndDrawableEntityResource() {
    delete mDrawPollGroup;
    mDrawPollGroup = nullptr;
}

// Reconstructed from eboot.elf at 0x6C0EE0. The deleting destructor is at
// 0x6C1410.
RndDrawableEntityResource::NoOpDrawJob::~NoOpDrawJob() {}

// Reconstructed from eboot.elf at 0x6C1130.
void RndDrawableEntityResource::StartTexRendererJobs(
    Entity* entity,
    PollDepBase* head,
    PollDepBase* afterTexRenderers,
    eastl::vector<PollDepBase*>& jobs) {
    _StartTexRendererJobs(entity, head, afterTexRenderers, jobs);
}

// Reconstructed from eboot.elf at 0x6C1140.
void RndDrawableEntityResource::StartDrawJobs(
    Entity* entity,
    const RndSceneDrawParams& params,
    Entity* after,
    PollDepBase* startDep,
    PollDepBase* contextDep,
    PollDepBase* endAfter,
    eastl::vector<PollDepBase*>& jobs) {
    _StartDrawJobs(entity, params, after, startDep, contextDep, endAfter, jobs);
}

// Reconstructed from eboot.elf at 0x6C12D0.
void RndDrawableEntityResource::FinishTexRendererJobs(Entity* entity, PollDepBase* afterTexRenderers) {
    _FinishTexRendererJobs(entity, afterTexRenderers);
}

// Reconstructed from eboot.elf at 0x6C12E0.
FixedVector<RndSceneDrawTarget, 2> RndDrawableEntityResource::FinishDrawJobs(Entity* entity) {
    return _FinishDrawJobs(entity);
}

// Reconstructed from eboot.elf at 0x6C1300.
void RndDrawableEntityResource::DrawTexRenderers(Entity* entity) {
    _DrawTexRenderers(entity);
}

// Reconstructed from eboot.elf at 0x6C1310.
FixedVector<RndSceneDrawTarget, 2> RndDrawableEntityResource::DrawImmediate(
    Entity* entity,
    RndSceneDrawParams& params,
    const FixedVector<RndSceneDrawTarget, 2>* previous) {
    return _Draw(entity, params, previous);
}

// Reconstructed from eboot.elf at 0x6C1330.
ResourceMetaData* RndDrawableEntityResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x6C1340.
Symbol RndDrawableEntityResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x6C13E0.
bool RndDrawableEntityResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr; metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x6C1440.
const char* RndDrawableEntityResource::NoOpDrawJob::GetPollName() const {
    return (FormatString("draw no-op job %s") << mName).Str();
}
