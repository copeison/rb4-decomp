// render/RndSceneResource.o (0x4384A0 to 0x43AEE0).
#include "render/scene/RndSceneResource.h"

#include "anim/drivers/AnimDriverCom.h"
#include "audio/core/components/AudioEmitterCom.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityHeaderCom.h"
#include "entity/core/InstanceCom.h"
#include "entity/core/TransCom.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropRegistry.h"
#include "render/atmosphere/RndFogCom.h"
#include "render/atmosphere/RndSkyCom.h"
#include "render/context/RndCameraCom.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/lighting/lights/RndLightCom.h"
#include "render/lighting/lights/RndLightEnvironCom.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndEntityInstanceCom.h"
#include "render/scene/RndSceneCom.h"
#include "render/scene/RndSceneInstanceCom.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTexRendererMgr.h"
#include "stategraph/drivers/StateGraphDriverCom.h"
#include "utl/streams/BinStream.h"
#include "utl/text/MakeString.h"
#include "utl/threading/PollMgr.h"
#include "utl/time/TimelinesCom.h"

// The class symbols of the instance pool component, "InstancePool".
// Names not in the reference map.
extern Symbol gInstancePoolComId;     // 0x19E35A0
extern Symbol gInstancePoolComClass;  // 0x19E35A8
// The class symbols of components this build adds after the map's:
// "AudioEmitterEditorData" and "TransformEditorDraw". Names not in the
// reference map.
extern Symbol gAudioEmitterEditorDataComId;      // 0x1A76098
extern Symbol gAudioEmitterEditorDataClassName;  // 0x1A760A0
extern Symbol gTransformEditorDrawComId;         // 0x1A86ED0
extern Symbol gTransformEditorDrawClassName;     // 0x1A86ED8
// The class symbol of the "ScenePollSplit" component (vtable 0x18FF610),
// which marks the object a new split poll job starts at. The class is not
// modelled. Name not in the reference map.
extern Symbol gScenePollSplitComId;  // 0x1A72D58

// Sets the property at the path from the value of the type. Emitted in
// entity/PropUtl.o; declared here until PropUtl.h declares it. The map's
// _SetProp(Component&, PropPath const&, PropInfo const&, void*, void
// const*, PropertyType, bool).
bool _SetProp(
    Component& component,
    const PropPath& path,
    const PropInfo& info,
    void* storage,
    const void* value,
    PropertyType type,
    bool notify);  // 0x18A440

// Resolves the automatic sharing of the entity's materials: collects which
// material objects must be unique (0x50E640) and sets each material
// component's uniqueness (RndMaterialCom::_SetUnique). Emitted beside
// NeedsUniqueMaterial (0x50F2D0); not reconstructed. Name not in the
// reference map.
void ResolveUniqueMaterials(Entity* entity);  // 0x50E4D0
// Moves the collision settings of particle systems saved before the
// collider component existed into "PSysCollider" components on the named
// collider objects, or on the system's own object. Emitted in
// RndParticleColliderCom's object; not reconstructed. Name not in the
// reference map.
void UpgradeParticleColliders(Entity* entity);  // 0x5FDF00

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

// The object's component with the class symbol, or null: the inlined
// GameObject::GetCom, for classes that are not modelled. Name not in the
// reference map.
Component* FindComById(const GameObject* object, Symbol id) {
    for (const GameObject::ComIndex& index : object->mComs) {
        if (index.mId == id) {
            return index.mCom;
        }
    }
    return nullptr;
}

// Sets the component's bool property, the inlined property write of
// CreateEntity. The failure report is compiled out, but its message is
// still formatted. Name not in the reference map.
void SetBoolProp(Component* component, Symbol name, bool value) {
    PropPath path;
    PropPath::Node node;
    node = name;
    path.PushNode(node);
    void* storage = component;
    const PropInfo* info = component->_GetPropRegistry().FindProp(path, &storage);
    if (info == nullptr ||
        !_SetProp(*component, path, *info, storage, &value, kPropertyBool, false)) {
        static_cast<void>(component->MakeErrorName());
        char buffer[512] = {};
        path.ToString(buffer, sizeof(buffer));
    }
}

// The class symbol made on first use, as _LoadEntity looks up the UI list
// classes by name. Name not in the reference map.
template <int Tag>
Symbol LazyClassSymbol(const char* name) {
    static Symbol sSymbol;
    if (sSymbol == Symbol()) {
        sSymbol = Symbol(name);
    }
    return sSymbol;
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

// Reconstructed from eboot.elf at 0x438670. The callbacks' std::function
// members are at 0x43AC70-0x43AEA0.
void RndSceneResource::_EnterImmediately(Entity* entity) {
    RndSceneCom* const scene = entity->GetRoot()->GetCom<RndSceneCom>();
    PollMgr* const pollMgr = scene->mPollMgr;
    if (scene->mPollGroups == nullptr) {
        scene->CreatePollGroups(entity, entity->mParentObject == nullptr);
        const std::function<void()> onPollStart = [scene, entity] {
            scene->StepFrameInterval();
            if (scene->mFrameInterval != 0) {
                entity->mFlags |= 0x2000;
            } else {
                scene->mIntervalStartFrame = static_cast<long>(TheRndDevice()->mFrameCount);
            }
        };
        const std::function<void()> onPollEnd = [entity] { entity->mFlags &= ~0x2000U; };
        scene->mPollGroups->mPollGroups.find(1)->second->SetCallbacks(onPollStart, onPollEnd);
    }
    PollGroup* const sceneGroup = scene->mPollGroups->mPollGroups.find(1)->second;
    PollGroup* const drawerGroup = scene->mPollGroups->mPollGroups.find(0)->second;
    sceneGroup->ClearJobs();
    drawerGroup->ClearJobs();
    pollMgr->Select(scene->mPollGroups);
    TransEntityResource::_EnterImmediately(entity);
    scene->mEntered = false;
    PollMgr::Deselect();
}

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

// Reconstructed from eboot.elf at 0x438B60. After TransEntityResource's
// load, the scene revision (written from resource revision 13 on) selects
// the upgrades of older scenes, oldest first.
bool RndSceneResource::_LoadEntity(BinStream& stream, bool cached) {
    if (!TransEntityResource::_LoadEntity(stream, cached)) {
        return false;
    }
    int rev = 0;
    if (mRev > 12) {
        stream.ReadEndian(&rev, sizeof(rev));
    }
    Entity* entity = mEntity;
    if (rev < 1) {
        // The root gets a state-graph driver and each instance its
        // renderer side.
        GameObject* root = entity->GetRoot();
        if (FindComById(root, StateGraphDriverCom::sId) == nullptr) {
            root->CreateComponent(StateGraphDriverCom::sClassName, false);
        }
        for (GameObject* object = entity->NextObject(nullptr, InstanceCom::sId);
             object != nullptr;
             object = entity->NextObject(object, InstanceCom::sId)) {
            if (FindComById(object, RndEntityInstanceCom::sId) == nullptr) {
                object->CreateComponent(RndEntityInstanceCom::sClassName, false);
            }
        }
    }
    if (rev < 2) {
        for (GameObject* object = entity->BeginObject(); object != nullptr;
             object = entity->NextObject(object, Symbol())) {
            if (FindComById(object, AudioEmitterCom::sId) != nullptr &&
                FindComById(object, gAudioEmitterEditorDataComId) == nullptr) {
                object->CreateComponent(gAudioEmitterEditorDataClassName, false);
            }
        }
    }
    if (rev < 3) {
        for (GameObject* object = entity->BeginObject(); object != nullptr;
             object = entity->NextObject(object, Symbol())) {
            if (FindComById(object, TransCom::sId) != nullptr &&
                FindComById(object, gTransformEditorDrawComId) == nullptr) {
                object->CreateComponent(gTransformEditorDrawClassName, false);
            }
        }
    }
    if (rev < 4) {
        // Gaze input lists pool their instances.
        for (GameObject* object = entity->BeginObject(); object != nullptr;
             object = entity->NextObject(object, Symbol())) {
            if (object->FindCom(LazyClassSymbol<0>("GazeInputUIListCom")) != nullptr &&
                FindComById(object, gInstancePoolComId) == nullptr) {
                object->CreateComponent(gInstancePoolComClass, false);
            }
        }
    }
    if (rev < 5) {
        UpgradeParticleColliders(entity);
    }
    if (rev < 6) {
        // The fog became part of the sky.
        for (GameObject* object = entity->BeginObject(); object != nullptr;
             object = entity->NextObject(object, Symbol())) {
            if (FindComById(object, RndFogCom::sId) != nullptr) {
                object->DestroyComponent(RndFogCom::sId, true);
                object->CreateComponent(RndSkyCom::sClassName, false);
            }
        }
    }
    if (rev < 7) {
        const Symbol uiList = LazyClassSymbol<1>("UIListCom");
        const Symbol uiListEditorData = LazyClassSymbol<2>("UIListEditorData");
        for (GameObject* object = entity->BeginObject(); object != nullptr;
             object = entity->NextObject(object, Symbol())) {
            if (object->FindCom(uiList) != nullptr &&
                object->FindCom(uiListEditorData) == nullptr) {
                object->CreateComponent(uiListEditorData, false);
            }
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x439300. A non-cached scene that loaded
// has its materials' uniqueness resolved.
bool RndSceneResource::_PostLoad(BinStream& stream, bool cached) {
    const bool loaded = TransEntityResource::_PostLoad(stream, cached);
    if (cached || !loaded) {
        return loaded;
    }
    ResolveUniqueMaterials(mEntity);
    return true;
}

// Reconstructed from eboot.elf at 0x439330.
Symbol RndSceneResource::GetInstanceComId() const {
    return RndSceneInstanceCom::sClassName;
}

// Reconstructed from eboot.elf at 0x439340.
bool RndSceneResource::_IsEditorEntity() {
    return true;
}

// Reconstructed from eboot.elf at 0x439350. The camera stands 20 units
// back along -Y.
Entity* RndSceneResource::CreateEntity() {
    Entity* entity = EntityResource::CreateEntity();
    RndSceneCom* scene = static_cast<RndSceneCom*>(
        entity->GetRoot()->CreateComponent(RndSceneCom::sClassName, false));
    entity->GetRoot()->CreateComponent(RndDrawNodeCom::sClassName, false);
    entity->GetRoot()->CreateComponent(AnimDriverCom::sClassName, false);
    entity->GetRoot()->CreateComponent(StateGraphDriverCom::sClassName, false);

    GameObject* cameraObject = entity->CreateObject(0, 0);
    RndCameraCom* camera = static_cast<RndCameraCom*>(
        cameraObject->CreateComponent(RndCameraCom::sClassName, false));
    cameraObject->SetName(Symbol("main_camera"));
    TransCom* trans = cameraObject->GetCom<TransCom>();
    trans->mLocalXfm.mPos = Vector3{0.0F, -20.0F, 0.0F};
    trans->mDirtyFlags |= 1;
    scene->SetCamera(0, camera);

    GameObject* lightMgrObject = entity->CreateObject(0, 0);
    RndLightMgrCom* lightMgr = static_cast<RndLightMgrCom*>(
        lightMgrObject->CreateComponent(RndLightMgrCom::sClassName, false));
    lightMgrObject->SetName(Symbol("light_mgr"));
    scene->SetLightMgr(lightMgr);

    GameObject* environObject = entity->CreateObject(0, 0);
    environObject->CreateComponent(RndLightEnvironCom::sClassName, false);
    environObject->SetName(Symbol("default_env"));
    lightMgr->SetDefaultEnvironId(environObject->mId);

    SetBoolProp(
        entity->GetRoot()->GetCom<EntityHeaderCom>(), Symbol("copy_on_instance"), false);
    return entity;
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

// Reconstructed from eboot.elf at 0x439950. The groups are looked up by
// key: 1 is the scene's, 0 the drawer's.
void RndSceneResource::_OnEntityReady(Entity* root, Entity* entity) {
    static_cast<void>(entity);
    RndSceneCom* scene = root->GetRoot()->GetCom<RndSceneCom>();
    if (scene->mEntered) {
        return;
    }
    RndSceneCom::PollGroups* groups = scene->mPollGroups;
    if (groups != nullptr) {
        groups->mPollGroups.find(1)->second->ClearJobs();
        groups->mPollGroups.find(0)->second->ClearJobs();
        for (RndSceneCom::PollSplitJobEntry& entry : scene->mPollSplitJobs) {
            entry.mPollGroup->ClearJobs();
            entry.mJob->ClearAllDeps();
        }
        scene->ClearPollSplitJobs();
    }
    scene->mEntered = true;
}

// Reconstructed from eboot.elf at 0x439AE0. The marked objects strictly
// inside the poll order close a window at their index, which the next one
// starts from; a last window runs to the end once one was closed. An
// object missing from its layer is read through anyway, as in the binary.
// The objects' components are given the job of the window their index
// falls in, compared with the next job's window end.
void RndSceneResource::_OnPolledImmediately(Entity* entity) {
    ThreadPollContext& context = gEntityThreadState;
    if (context.mPollMgr == nullptr || !context.mEnterImmediately) {
        return;
    }
    RndSceneCom* const scene = entity->GetRoot()->GetCom<RndSceneCom>();
    context.mPollGroups->mPollGroups.find(0)->second->AddJob(scene->mSceneDrawer);
    if (scene->mPollGroups == nullptr) {
        return;
    }
    PollGroup* const sceneGroup = scene->mPollGroups->mPollGroups.find(1)->second;
    scene->ClearPollSplitJobs();
    entity->_UpdatePollOrder();
    const unsigned int count = entity->mPollOrder.mSize;
    if (count == 0) {
        return;
    }
    unsigned int weight = 0;
    if (count > 2) {
        unsigned int start = 0;
        for (unsigned int i = 0; i != count - 2; ++i) {
            GameObject* const object = entity->GetObject(entity->mPollOrder.data()[i + 1]);
            const Component* const marker = object->FindCom(gScenePollSplitComId);
            if (marker == nullptr) {
                weight += object->mComs.mSize;
                continue;
            }
            const char* const name =
                (FormatString("SplitPoll %d") << scene->mPollSplitJobs.size()).Str();
            scene->AddPollSplitJob(name, static_cast<int>(start), static_cast<int>(i), weight / 10);
            weight = 0;
            start = i;
        }
    }
    if (!scene->mPollSplitJobs.empty()) {
        const RndSceneCom::PollSplitJob* const last = scene->mPollSplitJobs.back().mJob;
        const char* const name = (FormatString("SplitPoll %d") << scene->mPollSplitJobs.size()).Str();
        scene->AddPollSplitJob(name, last->mPollWindowEnd, static_cast<int>(count), weight / 10);
    }
    if (scene->mPollSplitJobs.empty()) {
        return;
    }
    sceneGroup->ClearJobs();
    entity->ClearAllDeps();
    unsigned int split = 0;
    for (unsigned int i = 0; i < entity->mPollOrder.mSize; ++i) {
        GameObject* const object = entity->GetObject(entity->mPollOrder.data()[i]);
        if (object == nullptr) {
            continue;
        }
        if (i == static_cast<unsigned int>(scene->mPollSplitJobs[split + 1].mJob->mPollWindowEnd)) {
            split = split + 1;
        }
        RndSceneCom::PollSplitJob* job = scene->mPollSplitJobs[split].mJob;
        if (i >= static_cast<unsigned int>(job->mPollWindowEnd)) {
            job = scene->mPollSplitJobs[split + 1].mJob;
        }
        const int coms = static_cast<int>(object->mComs.mSize);
        for (int c = 0; c < coms; ++c) {
            object->mComs.data()[c].mCom->_PostLoadResources(job, false);
        }
    }
    scene->LinkPollSplitJobs();
    sceneGroup->AddPollGroup(scene->mPollSplitJobs.front().mPollGroup);
}

// Reconstructed from eboot.elf at 0x43A130. A thread that is polling runs
// the game-mode poll through the base class only.
void RndSceneResource::_PollEntity(Entity* entity) {
    RndSceneCom* const scene = entity->GetRoot()->GetCom<RndSceneCom>();
    const ThreadPollContext& context = gEntityThreadState;
    const unsigned int mode = (entity->mFlags & 0x10) != 0 ? (entity->mFlags >> 1) & 3 : 0;
    if (context.mPollMgr != nullptr) {
        if (mode == 1) {
            _SyncTimelines(scene);
        }
        TransEntityResource::_PollEntity(entity);
        return;
    }
    if (mode == 2) {
        scene->StepFrameInterval();
        if (scene->mFrameInterval != 0) {
            return;
        }
        scene->mIntervalStartFrame = static_cast<long>(TheRndDevice()->mFrameCount);
        _SyncTimelines(scene);
        RndSceneDrawer* const drawer = scene->mSceneDrawer;
        drawer->_DoPoll();
        TransEntityResource::_PollEntity(entity);
        drawer->_DoPostPoll();
        return;
    }
    if (mode != 1) {
        return;
    }
    if (scene->mEntered) {
        RndSceneResource::_EnterImmediately(entity);
    }
    PollMgr* const pollMgr = scene->mPollMgr;
    RndSceneCom::PollGroups* const groups = scene->mPollGroups;
    pollMgr->Select();
    pollMgr->StartPollGroup(groups->mPollGroups.find(0)->second);
    pollMgr->Sync(groups->mPollGroups.find(1)->second);
    pollMgr->Sync(nullptr);
    groups->mPollGroups.find(1)->second->_PostPoll();
    groups->mPollGroups.find(0)->second->_PostPoll();
    PollMgr::Deselect();
}

// Reconstructed from eboot.elf at 0x43A580.
void RndSceneResource::_SyncTimelines(RndSceneCom* scene) {
    TimelinesCom* const timelines = scene->mObject->GetBaseCom<TimelinesCom>();
    if (timelines == nullptr) {
        return;
    }
    timelines->mClockSynced = true;
    timelines->_UpdateParentClock();
    timelines->_SyncClock();
}

// Reconstructed from eboot.elf at 0x43A600. The map's
// CreateRequiredComponents(ObjPtr&, ComMetaData const&).
bool RndSceneResource::CreateRequiredComponents(
    GameObject* object,
    const ComMetaData& metaData) {
    if (!RndDrawableEntityResource::CreateRequiredComponents(object, metaData)) {
        return false;
    }
    if (metaData.mId == InstanceCom::sClassName || metaData.mId == gInstancePoolComClass) {
        return object->CreateComponent(RndDrawNodeCom::sClassName, false) != nullptr;
    }
    return true;
}

// Reconstructed from eboot.elf at 0x43A660. The map's
// DestroyDependentComponents(ObjPtr&, ComMetaData const&).
void RndSceneResource::DestroyDependentComponents(
    GameObject* object,
    const ComMetaData& metaData) {
    if (metaData.mId == RndDrawNodeCom::sClassName) {
        object->DestroyComponent(InstanceCom::sId, true);
        object->DestroyComponent(gInstancePoolComId, true);
    }
    RndDrawableEntityResource::DestroyDependentComponents(object, metaData);
}

// Reconstructed from eboot.elf at 0x43A6D0. A new instance's object must
// hold a draw node.
void RndSceneResource::_OnComponentCreated(Component* component) {
    if (component->GetInterfaceId() != RndLightCom::sClassName &&
        component->GetInterfaceId() != RndDrawNodeCom::sClassName &&
        component->GetInterfaceId() != InstanceCom::sClassName) {
        return;
    }
    RndSceneCom* const scene = component->mObject->mEntity->GetRoot()->GetCom<RndSceneCom>();
    RndLightMgrCom* const lightMgr = scene->GetLightMgr();
    RndLightEnvironCom* const environ = lightMgr != nullptr ? lightMgr->GetDefaultEnviron() : nullptr;
    const Symbol id = component->GetInterfaceId();
    if (id == RndLightCom::sClassName) {
        if (environ != nullptr) {
            component->mObject->GetBaseCom<RndLightCom>()->AddToEnviron(environ);
        }
    } else if (id == RndDrawNodeCom::sClassName) {
        if (environ != nullptr) {
            component->mObject->GetExistingCom<RndDrawNodeCom>()->mEnvironment = environ->mObject->mId;
        }
    } else if (id == InstanceCom::sClassName) {
        component->mObject->CreateComponent(RndEntityInstanceCom::sClassName, false);
        if (environ != nullptr) {
            RndDrawNodeCom* const node = component->mObject->GetCom<RndDrawNodeCom>();
            const GameObjectId environId = environ->mObject->mId;
            node->mNestedLightEnvironments._Insert(node->mNestedLightEnvironments.mSize, &environId);
            node->_SyncNestedLightEnvirons();
        }
    }
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
