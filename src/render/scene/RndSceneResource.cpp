// render/RndSceneResource.o (0x4384A0 to 0x43AEE0).
#include "render/scene/RndSceneResource.h"

#include "entity/core/Entity.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/scene/RndSceneCom.h"
#include "render/scene/RndSceneInstanceCom.h"
#include "render/textures/RndTexRendererMgr.h"
#include "utl/streams/BinStream.h"
#include "utl/threading/PollMgr.h"

namespace {

// The scene component on the entity's root object. The slots that know it
// is there search without a bound (GameObject::GetExistingCom). Name not in
// the reference map.
RndSceneCom* SceneComOf(const Entity* entity) {
    return entity->GetRoot()->GetExistingCom<RndSceneCom>();
}

// The scene drawer of the scene entity, or null for no entity. Name not in
// the reference map.
RndSceneDrawer* SceneDrawerOf(const Entity* entity) {
    return entity != nullptr ? SceneComOf(entity)->mSceneDrawer : nullptr;
}

}  // namespace

// The object's static, built by its static initializer (0x43AEB0) and
// filled by _Init (from 0x3EB592). The three ints it first sets (0x1A72FC8)
// come from a shared header and are not modelled.
ResourceMetaData RndSceneResource::sMetaData;

// Reconstructed from eboot.elf at 0x4384A0.
RndSceneResource::RndSceneResource() {}

// Reconstructed from eboot.elf at 0x4384D0. The deleting destructor is at
// 0x4384E0.
RndSceneResource::~RndSceneResource() {}

// Reconstructed from eboot.elf at 0x4389E0.
void RndSceneResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("scene"));
    metaData.mCategory = Symbol("Scenes");
    metaData.mTypeOption = 1;
}

// Reconstructed from eboot.elf at 0x438B00.
void RndSceneResource::Save(BinStream& stream, bool cached) {
    TransEntityResource::Save(stream, cached);
    const int rev = 7;
    stream.WriteEndian(&rev, sizeof(rev));
}

// Reconstructed from eboot.elf at 0x439330.
Symbol RndSceneResource::GetInstanceComId() const {
    return RndSceneInstanceCom::sClassName;
}

// Reconstructed from eboot.elf at 0x439340.
bool RndSceneResource::_IsEditorEntity() {
    return true;
}

// Reconstructed from eboot.elf at 0x439820.
void RndSceneResource::_EnterEntity(Entity* entity, unsigned int flags) {
    RndSceneCom* scene = entity->GetRoot()->GetCom<RndSceneCom>();
    RndSceneDrawer* drawer = scene->mSceneDrawer;
    drawer->EnterPrelude(entity);
    TransEntityResource::_EnterEntity(entity, flags);
    drawer->EnterCoda(entity);
    scene->mEntered = true;
}

// Reconstructed from eboot.elf at 0x4398D0.
void RndSceneResource::_ExitEntity(Entity* entity, unsigned int flags) {
    SceneComOf(entity)->mSceneDrawer->ExitPrelude(entity);
    TransEntityResource::_ExitEntity(entity, flags);
}

// Reconstructed from eboot.elf at 0x439950. The managers are looked up by
// key: 1 is the scene's, 0 the drawer's.
void RndSceneResource::_OnEntityReady(Entity* root, Entity* entity) {
    static_cast<void>(entity);
    RndSceneCom* scene = root->GetRoot()->GetCom<RndSceneCom>();
    if (scene->mEntered) {
        return;
    }
    RndSceneCom::PollGroups* groups = scene->mPollGroups;
    if (groups != nullptr) {
        groups->mPollMgrs.find(1)->second->ClearJobs();
        groups->mPollMgrs.find(0)->second->ClearJobs();
        for (RndSceneCom::PollSplitJobEntry& entry : scene->mPollSplitJobs) {
            entry.mPollMgr->ClearJobs();
            entry.mJob->ClearAllDeps();
        }
        scene->ClearPollSplitJobs();
    }
    scene->mEntered = true;
}

// Reconstructed from eboot.elf at 0x43A950.
FixedVector<RndSceneDrawTarget, 2> RndSceneResource::_Draw(
    Entity* entity,
    RndSceneDrawParams& params,
    const FixedVector<RndSceneDrawTarget, 2>* previous) {
    return SceneDrawerOf(entity)->Draw(entity, params, previous);
}

// Reconstructed from eboot.elf at 0x43A9B0. The drawer of the scene drawn
// before, if any, orders the jobs.
void RndSceneResource::_StartDrawJobs(
    Entity* entity,
    const RndSceneDrawParams& params,
    Entity* after,
    PollDepBase* startDep,
    PollDepBase* contextDep,
    PollDepBase* endAfter,
    eastl::vector<PollDepBase*>& jobs) {
    SceneDrawerOf(entity)->StartDrawJobs(
        entity, params, SceneDrawerOf(after), startDep, contextDep, endAfter, jobs);
}

// Reconstructed from eboot.elf at 0x43AA20.
FixedVector<RndSceneDrawTarget, 2> RndSceneResource::_FinishDrawJobs(Entity* entity) {
    return SceneDrawerOf(entity)->FinishDrawJobs();
}

// Reconstructed from eboot.elf at 0x43AA80.
void RndSceneResource::_DrawTexRenderers(Entity* entity) {
    RndTexRendererMgr* texRenderers = SceneComOf(entity)->mRuntimeData.mTexRendererMgr;
    if (texRenderers != nullptr) {
        texRenderers->DrawImmediate();
    }
}

// Reconstructed from eboot.elf at 0x43AAC0.
void RndSceneResource::_StartTexRendererJobs(
    Entity* entity,
    PollDepBase* head,
    PollDepBase* afterTexRenderers,
    eastl::vector<PollDepBase*>& jobs) {
    RndTexRendererMgr* texRenderers = SceneComOf(entity)->mRuntimeData.mTexRendererMgr;
    if (texRenderers != nullptr) {
        texRenderers->AddDrawJobs(head, afterTexRenderers, jobs);
    }
}

// Reconstructed from eboot.elf at 0x43AB10. The manager does not read the
// job it is passed.
void RndSceneResource::_FinishTexRendererJobs(Entity* entity, PollDepBase* afterTexRenderers) {
    static_cast<void>(afterTexRenderers);
    RndTexRendererMgr* texRenderers = SceneComOf(entity)->mRuntimeData.mTexRendererMgr;
    if (texRenderers != nullptr) {
        texRenderers->ClearDrawJobs();
    }
}

// Reconstructed from eboot.elf at 0x43AB50.
ResourceMetaData* RndSceneResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x43AB60.
Symbol RndSceneResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x43AC00.
bool RndSceneResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr; metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}
