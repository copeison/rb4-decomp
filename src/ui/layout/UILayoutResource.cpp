// ui/UILayoutResource.o (0x8AAA40 to 0x8B1BAF). The layout's scene draw
// job (vtable 0x195D368, destructors 0x8B1B10 and 0x8B1B60) and the
// remaining slots are not reconstructed.
#include "ui/layout/UILayoutResource.h"

#include "anim/drivers/AnimDriverCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "os/memory/MemMgr.h"
#include "render/drawing/RndDrawUtl.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndSceneCom.h"
#include "render/scene/RndSceneInstanceCom.h"
#include "render/scene/RndSceneResource.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/targets/RndBufferCollection.h"
#include "stategraph/drivers/StateGraphDriverCom.h"
#include "ui/input/UIInputCom.h"
#include "ui/layout/UILayoutCom.h"
#include "utl/data/DataNode.h"
#include "utl/data/DataUtl.h"
#include "utl/files/FileUtl.h"
#include "utl/streams/BinStream.h"

namespace {

// The null object id of a shared header, which the static initializer sets
// to -1 (0x1AC8FA8). Name not in the reference map.
GameObjectId gNoObject = {0xFFFFFFFFU};

// Removes the element at the index, as the binary's inlined PropArray
// erase does: destroys it and moves the later elements down through the
// array's vtable. Name not in the reference map.
void EraseAt(PropArray<GameObjectId>& array, unsigned int index) {
    array.Destruct(1, array.ElementAt(index));
    if (index < array.mSize - 1) {
        array.Move(array.mSize - 1 - index, array.ElementAt(index), array.ElementAt(index + 1));
    }
    --array.mSize;
}

// Inserts the id before or after the anchor's place in the order; nothing
// happens when the anchor is not in it. An out-of-line copy is at 0x8AE200.
// Name not in the reference map.
void InsertNextTo(
    PropArray<GameObjectId>& order,
    UILayoutResource::QueuedSceneMode mode,
    GameObjectId id,
    GameObjectId anchor) {
    for (unsigned long i = 0; i < order.mSize; ++i) {
        if (order.data()[i].mId == anchor.mId) {
            order._Insert(i + (mode == UILayoutResource::kQueuedSceneAfter ? 1 : 0), &id);
            return;
        }
    }
}

// The entity a scene object instances, or null when the object has no
// scene instance or it has no entity yet. Inlined by every scene walk.
// Name not in the reference map.
Entity* SceneEntityOf(const GameObject* object) {
    const RndSceneInstanceCom* instance = object->GetCom<RndSceneInstanceCom>();
    if (instance == nullptr) {
        return nullptr;
    }
    return instance->mRuntime.mEntity;
}

// The scene entity of a showing scene object, whose components are
// searched without a bound. Inlined into the texture renderer slots. Name
// not in the reference map.
Entity* ShowingSceneEntity(const Entity* entity, GameObjectId id) {
    return entity->GetObject(id)->GetExistingCom<RndSceneInstanceCom>()->mRuntime.mEntity;
}

// Reconstructed from eboot.elf at 0x8AF870. Draws the masking scenes'
// depth with the scene's parameters, each clearing only as the first; the
// results are dropped. The binary leaves the unused layout argument unset.
// Name not in the reference map.
void DrawMaskingScenes(
    const UILayoutResource* layout,
    Entity* entity,
    const RndSceneDrawParams& params,
    ResourcePath scenePath,
    const VectorAdapter<GameObjectId>& scenes) {
    static_cast<void>(layout);
    static_cast<void>(scenePath);
    if (scenes.mSize == 0) {
        return;
    }
    RndDevice* const device = gRndDevice;
    if (device->mBeginFramePending) {
        device->_FlushPendingBeginFrame();
    }
    RndSceneDrawParams maskParams(params);
    maskParams.mDepthOnly = true;
    const Vector2i size = maskParams.mBuffers[0]->mSize;
    for (unsigned long i = 0; i < scenes.mSize; ++i) {
        const RndSceneInstanceCom* const instance =
            entity->GetObject(scenes.mData[i])->GetExistingCom<RndSceneInstanceCom>();
        Entity* const scene = instance->mRuntime.mEntity;
        RndDrawableEntityResource* const resource =
            static_cast<RndDrawableEntityResource*>(scene->mResource);
        maskParams.mProjectionRect = instance->CalcProjectionRect(size);
        maskParams.mClear = i == 0;
        resource->Draw(scene, maskParams);
    }
}

}  // namespace

// The object's static, built by its static initializer.
ResourceMetaData UILayoutResource::sMetaData;

// Reconstructed from eboot.elf at 0x8AAA40.
UILayoutResource::UILayoutResource()
    : mEntering(false), mExiting(false), mFramesToSkipDraw(0), mSerialPollFrames(0) {}

// Reconstructed from eboot.elf at 0x8AAB30.
UILayoutResource::~UILayoutResource() {
    _AddQueuedScenes();
    _RemoveQueuedScenes();
    if (mEntity != nullptr) {
        _ClearThreadedPollRange(mEntity);
    }
    for (unsigned long i = 0; i < mSceneDrawJobs.size(); ++i) {
        delete mSceneDrawJobs[i];
    }
    mSceneDrawJobs.clear();
}

// Reconstructed from eboot.elf at 0x8AAC60. The queue is taken first, in
// the temporary heap, so scenes queued while it runs wait for the next call.
// Each scene gets a new object of the main layer named for the file, with a
// scene instance; the layout component is read without a check.
void UILayoutResource::_AddQueuedScenes() {
    if (mQueuedScenes.empty()) {
        return;
    }
    eastl::vector<QueuedScene> scenes;
    unsigned int temp;
    MemPushTemp(temp, true, true);
    scenes = mQueuedScenes;
    MemPopTemp(temp);
    mQueuedScenes.clear();
    Entity* const entity = mEntity;
    _ClearThreadedPollRange(entity);
    if (scenes.empty()) {
        return;
    }
    Entity* const defaultEntity = gDataThread.mDefaultEntity;
    DataSetDefaultEntity(entity);
    for (const QueuedScene& scene : scenes) {
        GameObject* const object = entity->CreateObject(0, 0);
        char base[512] = {};
        object->SetName(Symbol(FileGetBase(scene.mPath.Str(), base)));
        RndSceneInstanceCom* const instance = static_cast<RndSceneInstanceCom*>(
            object->CreateComponent(RndSceneInstanceCom::sClassName, false));
        instance->mFile = scene.mPath;
        instance->mUseThreadedPoll = scene.mUseThreadedPoll;
        UILayoutCom* const layout = entity->GetRoot()->GetCom<UILayoutCom>();
        if (scene.mMode != kQueuedSceneFront) {
            layout->RemoveSceneObject(object->mId);
            InsertNextTo(layout->mPollOrder, scene.mMode, object->mId, scene.mAnchor);
            InsertNextTo(layout->mDrawOrder, scene.mMode, object->mId, scene.mAnchor);
        }
        object->_ReenterComponents();
        UILayoutResource::_OnEntityReady(mEntity, instance->mRuntime.mEntity);

        // The other scenes resync the scenes that mask them.
        for (unsigned int i = 0; i < layout->mPollOrder.mSize; ++i) {
            const GameObjectId id = layout->mPollOrder.data()[i];
            if (id.mId == object->mId.mId || id.mId == gNoObject.mId) {
                continue;
            }
            const GameObject* const other = entity->GetObject(id);
            if (other == nullptr) {
                continue;
            }
            RndSceneInstanceCom* const otherInstance = other->GetCom<RndSceneInstanceCom>();
            if (otherInstance != nullptr && otherInstance->mMaskedByDynamicScenes.mSize != 0) {
                otherInstance->_SyncMaskingScenes();
            }
        }
    }
    gDataThread.mThisObject = nullptr;
    DataSetDefaultEntity(defaultEntity);
}

// Reconstructed from eboot.elf at 0x8AB2D0. The queue is taken first, so
// removals queued while it runs wait for the next call. The scene objects
// are looked up the last in poll order first; the layout component is read
// without a check.
void UILayoutResource::_RemoveQueuedScenes() {
    const eastl::vector<ResourcePath> removals(mQueuedRemovals);
    mQueuedRemovals.clear();
    if (removals.empty()) {
        return;
    }
    Entity* const entity = mEntity;
    _ClearThreadedPollRange(entity);
    for (const ResourcePath& path : removals) {
        GameObject* object = nullptr;
        Entity* scene = nullptr;
        for (int index = static_cast<int>(entity->mPollOrder.mSize); index > 0; --index) {
            GameObject* const candidate =
                entity->SafeGetObject(entity->mPollOrder.data()[index - 1], false);
            if (candidate == nullptr) {
                continue;
            }
            Entity* const candidateScene = SceneEntityOf(candidate);
            if (candidateScene != nullptr && candidateScene->mResource->mPath == path) {
                object = candidate;
                scene = candidateScene;
                break;
            }
        }
        if (object == nullptr) {
            continue;
        }

        UILayoutCom* const layout = entity->GetRoot()->GetCom<UILayoutCom>();
        layout->RemoveSceneObject(object->mId);
        for (auto it = layout->mReadyScenes.begin(); it != layout->mReadyScenes.end();) {
            if (it->mEntity == scene || it->mEntity->mParentObject == object) {
                it = layout->mReadyScenes.erase(it);
            } else {
                ++it;
            }
        }
        entity->DestroyObject(object->mId);

        // The scenes that remain resync the scenes that mask them.
        for (unsigned int i = 0; i < layout->mPollOrder.mSize; ++i) {
            const GameObjectId id = layout->mPollOrder.data()[i];
            if (id.mId == gNoObject.mId) {
                continue;
            }
            const GameObject* const other = entity->GetObject(id);
            if (other == nullptr) {
                continue;
            }
            RndSceneInstanceCom* const instance = other->GetCom<RndSceneInstanceCom>();
            if (instance != nullptr && instance->mMaskedByDynamicScenes.mSize != 0) {
                instance->_SyncMaskingScenes();
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x8AB6E0. Without jobs in the scene
// groups only the layout group's job count is read, and the range stays
// linked. The layout component is read without a check.
void UILayoutResource::_ClearThreadedPollRange(Entity* entity) {
    UILayoutCom* const layout = entity->GetRoot()->GetCom<UILayoutCom>();
    layout->mPollRangeReleased = true;
    RndSceneCom::PollGroups* const groups = layout->mScenePollGroups;
    bool idle = groups == nullptr;
    if (!idle) {
        PollGroup* const sceneGroup = groups->mPollGroups.find(1)->second;
        PollGroup* const drawerGroup = groups->mPollGroups.find(0)->second;
        idle = sceneGroup->GetNumJobs() == 0 && drawerGroup->GetNumJobs() == 0;
    }
    if (idle) {
        if (layout->mPollGroup != nullptr) {
            static_cast<void>(layout->mPollGroup->GetNumJobs());
        }
        return;
    }
    layout->mScenePollGroups->mPollGroups.find(1)->second->ClearJobs();
    layout->mScenePollGroups->mPollGroups.find(0)->second->ClearJobs();
    layout->mPollGroup->ClearJobs();
    for (RndSceneInstanceCom* const scene : layout->mThreadedScenes) {
        scene->GetScenePollGroup()->TryRemovePollAfter(layout->mPollGroup);
    }
    for (auto it = layout->mThreadedScenes.begin(); it != layout->mThreadedScenes.end(); ++it) {
        PollGroup* const group = (*it)->GetScenePollGroup();
        for (auto before = layout->mThreadedScenes.begin(); before != it; ++before) {
            group->TryRemovePollAfter((*before)->GetScenePollGroup());
        }
    }
    layout->mThreadedScenes.clear();
}

// Reconstructed from eboot.elf at 0x8ABA50.
void UILayoutResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("layout"));
    metaData.mTypeFlags[0] = false;
    metaData.mTypeOption = 1;
}

// Reconstructed from eboot.elf at 0x8ABB50. The layout's revision follows
// the entity from TransEntityResource revision 13 on. A cached file older
// than revision 4 fails. Older layouts gain the root components and build
// the layout component's draw order from the scene instances' draw orders,
// with the poll order a copy of it. The layout component is read without a
// check, and an empty draw order is not guarded.
bool UILayoutResource::_LoadEntity(BinStream& stream, bool cached) {
    if (!RndDrawableEntityResource::_LoadEntity(stream, cached)) {
        return false;
    }
    int rev = 0;
    if (mRev >= 13) {
        stream.ReadEndian(&rev, sizeof(rev));
    }
    if (rev <= 3 && cached) {
        return false;
    }
    if (rev <= 0) {
        GameObject* const root = mEntity->GetRoot();
        if (root->GetCom<StateGraphDriverCom>() == nullptr) {
            root->CreateComponent(StateGraphDriverCom::sClassName, false);
        }
        if (root->GetCom<AnimDriverCom>() == nullptr) {
            root->CreateComponent(AnimDriverCom::sClassName, false);
        }
    }
    if (rev <= 2) {
        mEntity->GetRoot()->CreateComponent(UILayoutCom::sClassName, false);
    }
    if (rev > 3) {
        return true;
    }

    Entity* const entity = mEntity;
    UILayoutCom* const layout = entity->GetRoot()->GetCom<UILayoutCom>();
    PropArray<GameObjectId>& drawOrder = layout->mDrawOrder;
    int lowest = 0;
    for (GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        const RndSceneInstanceCom* const instance = object->GetCom<RndSceneInstanceCom>();
        if (instance == nullptr) {
            continue;
        }
        const int order = instance->mDrawOrder;
        while (lowest > order) {
            drawOrder._Insert(0, &gNoObject);
            --lowest;
        }
        const unsigned long index = static_cast<unsigned long>(order - lowest);
        if (index >= drawOrder.mSize) {
            drawOrder.Resize(index + 1);
        }
        drawOrder.data()[index] = object->mId;
    }
    unsigned int index = drawOrder.mSize - 1;
    do {
        if (drawOrder.data()[index].mId == gNoObject.mId) {
            EraseAt(drawOrder, index);
        }
    } while (index-- != 0);
    layout->mPollOrder._Copy(drawOrder);
    entity->mFlags |= 0x400;
    return true;
}

// Reconstructed from eboot.elf at 0x8ABF60.
bool UILayoutResource::_PostLoad(BinStream& stream, bool cached) {
    if (!RndDrawableEntityResource::_PostLoad(stream, cached)) {
        return false;
    }
    _ValidateSceneData(mEntity);
    return true;
}

// Reconstructed from eboot.elf at 0x8ABF90. The checks on the light
// managers are compiled out; the scene objects and the scene components are
// read without a check.
void UILayoutResource::_ValidateSceneData(Entity* entity) {
    const unsigned int count = entity->mPollOrder.mSize;
    for (unsigned int i = 0; i < count; ++i) {
        const Entity* const scene = SceneEntityOf(entity->GetObject(entity->mPollOrder.data()[i]));
        if (scene == nullptr) {
            continue;
        }
        RndSceneCom* const sceneCom = scene->GetRoot()->GetCom<RndSceneCom>();
        if (sceneCom->GetLightMgr() != nullptr) {
            static_cast<void>(sceneCom->GetLightMgr());
        }
    }
}

// Reconstructed from eboot.elf at 0x8AC0F0.
void UILayoutResource::Save(BinStream& stream, bool cached) {
    RndDrawableEntityResource::Save(stream, cached);
    const int rev = 4;
    stream.WriteEndian(&rev, sizeof(rev));
}

// Reconstructed from eboot.elf at 0x8AC150. The root is reread from the
// resource for each component.
Entity* UILayoutResource::CreateEntity() {
    Entity* const entity = RndDrawableEntityResource::CreateEntity();
    mEntity->GetRoot()->CreateComponent(StateGraphDriverCom::sClassName, false);
    mEntity->GetRoot()->CreateComponent(AnimDriverCom::sClassName, false);
    mEntity->GetRoot()->CreateComponent(UILayoutCom::sClassName, false);
    return entity;
}

// Reconstructed from eboot.elf at 0x8AC1E0. A run of threaded scenes
// polls after the groups the last serial scene left; the serial scene that
// ends the run polls after the whole run. An object without a scene
// instance has its InstanceCom looked up, a check that is compiled out.
// The layout component is read without a check.
void UILayoutResource::_SyncThreadedPollRange(Entity* entity) {
    UILayoutCom* const layout = entity->GetRoot()->GetCom<UILayoutCom>();
    _ClearThreadedPollRange(entity);
    eastl::list<PollGroup*> after;
    eastl::list<PollGroup*> threadedRun;
    for (unsigned long i = 0; i < entity->mPollOrder.mSize; ++i) {
        const GameObject* const object = entity->GetObject(entity->mPollOrder.data()[i]);
        RndSceneInstanceCom* const instance = object->GetCom<RndSceneInstanceCom>();
        if (instance == nullptr) {
            static_cast<void>(object->GetBaseCom<InstanceCom>());
        } else if (instance->mRuntime.mEntity != nullptr) {
            layout->mThreadedScenes.push_back(instance);
        }
    }
    // -1 before the first scene, then whether the last scene was threaded.
    int lastThreaded = -1;
    for (RndSceneInstanceCom* const instance : layout->mThreadedScenes) {
        PollGroup* const group = instance->GetScenePollGroup();
        if (instance->CanUseThreadedPoll()) {
            for (PollGroup* const before : after) {
                group->PollAfter(before);
            }
            threadedRun.push_back(instance->GetScenePollGroup());
            lastThreaded = 1;
        } else {
            if (lastThreaded != 0) {
                after.clear();
                after.swap(threadedRun);
            }
            lastThreaded = 0;
            for (PollGroup* const before : after) {
                group->PollAfter(before);
            }
            after.clear();
            after.push_back(group);
        }
    }
    layout->mPollGroup->AddJob(entity);
    for (RndSceneInstanceCom* const instance : layout->mThreadedScenes) {
        instance->GetScenePollGroup()->PollAfter(layout->mPollGroup);
    }
    layout->mScenePollGroups->mPollGroups.find(1)->second->AddPollGroup(layout->mPollGroup);
    for (RndSceneInstanceCom* const instance : layout->mThreadedScenes) {
        layout->mScenePollGroups->mPollGroups.find(0)->second->AddPollGroup(
            instance->GetSceneDrawerPollGroup());
    }
}

// Reconstructed from eboot.elf at 0x8AC810. An enter with bit 0 of the
// flags goes to the base class. Otherwise the layout enters its objects in
// the game mode itself, with the entity as the default entity: it creates
// the layout component's poll managers, keeps the scenes whose scene
// component is entered for the next sync of the poll range, and marks the
// range released. The layout component is read without a check.
void UILayoutResource::_EnterEntity(Entity* entity, unsigned int flags) {
    if ((flags & 1) != 0) {
        RndDrawableEntityResource::_EnterEntity(entity, flags);
        return;
    }
    UILayoutCom* const layout = entity->GetRoot()->GetCom<UILayoutCom>();
    static_cast<PollDepBase*>(entity)->SetThreaded(false);
    entity->_UpdatePollOrder();
    Entity* const defaultEntity = gDataThread.mDefaultEntity;
    DataSetDefaultEntity(entity);
    layout->_ObtainPollMgr();
    layout->_CreatePollGroups();
    layout->mReadyScenes.clear();
    for (unsigned long index = 0; index < entity->mPollOrder.mSize; ++index) {
        GameObject* const object = entity->GetObject(entity->mPollOrder[index]);
        object->_EnterComponents();
        Entity* const scene = SceneEntityOf(object);
        if (scene == nullptr) {
            continue;
        }
        if (scene->GetRoot()->GetExistingCom<RndSceneCom>()->mEntered) {
            RndSceneResource* const resource = static_cast<RndSceneResource*>(scene->mResource);
            layout->mReadyScenes.push_back(UILayoutCom::ReadyScene{resource, scene});
        }
    }
    layout->mPollRangeReleased = true;
    gDataThread.mThisObject = nullptr;
    DataSetDefaultEntity(defaultEntity);
    entity->mFlags = (entity->mFlags & ~0x16U) | 0x12U;
    mSerialPollFrames = 0;
}

// Reconstructed from eboot.elf at 0x8ACBC0.
void UILayoutResource::_ExitEntity(Entity* entity, unsigned int flags) {
    UILayoutCom* const layout = entity->GetRoot()->GetCom<UILayoutCom>();
    _ClearThreadedPollRange(entity);
    layout->_DestroyPollGroups();
    RndDrawableEntityResource::_ExitEntity(entity, flags);
}

// Reconstructed from eboot.elf at 0x8ACC60. A thread that is polling
// polls the entity through the base class only. While mSerialPollFrames
// counts down, the audio lock is left as it is. The layout component is
// read without a check.
void UILayoutResource::_PollEntity(Entity* entity) {
    if (gEntityThreadState.mPollMgr != nullptr) {
        TransEntityResource::_PollEntity(entity);
        return;
    }
    _AddQueuedScenes();
    _RemoveQueuedScenes();
    if ((entity->mFlags & 0x16) == 0x14) {
        TransEntityResource::_PollEntity(entity);
    } else if ((entity->mFlags & 0x100) != 0) {
        Entity* const defaultEntity = gDataThread.mDefaultEntity;
        DataSetDefaultEntity(entity);
        UILayoutCom* const layout = entity->GetRoot()->GetCom<UILayoutCom>();
        if ((entity->mFlags & 0x400) != 0) {
            entity->_UpdatePollOrder();
            layout->mPollRangeReleased = true;
            UILayoutResource::_EnterImmediately(entity);
        } else if (layout->mPollRangeReleased) {
            UILayoutResource::_EnterImmediately(entity);
        }
        PollMgr* const pollMgr = layout->mPollMgr;
        pollMgr->Select();
        static const unsigned long sAudioLock =
            DataVarIndex(Symbol("use_pollmgr_audio_lock"), DataNode(1));
        const DataNode& audioLock = DataVariable(sAudioLock);
        if (mSerialPollFrames == 0) {
            PollMgr::BlockAudioThread(audioLock.mValue.integer != 0);
        }
        for (PollGroup* const group : layout->mStartPollGroups) {
            layout->mPollMgr->StartPollGroup(group);
        }
        layout->mPollMgr->StartPollGroup(layout->mScenePollGroups->mPollGroups.find(0)->second);
        layout->mPollMgr->Sync(layout->mScenePollGroups->mPollGroups.find(1)->second);
        if (mSerialPollFrames != 0) {
            const int frames = mSerialPollFrames - 1;
            mSerialPollFrames = frames < 0 ? 0 : frames;
        } else {
            PollMgr::BlockAudioThread(false);
        }
        layout->mPollMgr->Sync(nullptr);
        PollMgr::Deselect();
        {
            eastl::list<RndSceneInstanceCom*> scenes(layout->mThreadedScenes);
            layout->mScenePollGroups->mPollGroups.find(1)->second->_PostPoll();
            for (RndSceneInstanceCom* const scene : scenes) {
                PollMgr* const scenePollMgr = scene->GetScenePollMgr();
                scenePollMgr->Select(nullptr);
                scene->GetScenePollGroup()->_PostPoll();
                PollMgr::Deselect();
            }
            for (RndSceneInstanceCom* const scene : scenes) {
                PollMgr* const scenePollMgr = scene->GetScenePollMgr();
                scenePollMgr->Select(nullptr);
                scene->GetSceneDrawerPollGroup()->_PostPoll();
                PollMgr::Deselect();
            }
            layout->mScenePollGroups->mPollGroups.find(0)->second->_PostPoll();
        }
        gDataThread.mThisObject = nullptr;
        DataSetDefaultEntity(defaultEntity);
    }
    _AssignPartialFramerateScenes(entity);
    entity->GetRoot()->GetCom<UILayoutCom>()->mStartPollGroups.clear();
}

// Reconstructed from eboot.elf at 0x8AD8F0. The layout component is read
// without a check.
void UILayoutResource::_EnterImmediately(Entity* entity) {
    UILayoutCom* const layout = entity->GetRoot()->GetCom<UILayoutCom>();
    if (!layout->mPollRangeReleased) {
        return;
    }
    _ClearThreadedPollRange(entity);
    for (UILayoutCom::ReadyScene& ready : layout->mReadyScenes) {
        ready.mResource->RndSceneResource::_EnterImmediately(ready.mEntity);
    }
    _SyncThreadedPollRange(entity);
    layout->mReadyScenes.clear();
    layout->mPollRangeReleased = false;
}

// Reconstructed from eboot.elf at 0x8ADA20. The layout component is read
// without a check.
void UILayoutResource::_OnEntityReady(Entity* root, Entity* entity) {
    UILayoutCom* const layout = root->GetRoot()->GetCom<UILayoutCom>();
    if (root == entity) {
        if (!layout->mPollRangeReleased) {
            _ClearThreadedPollRange(root);
        }
        return;
    }
    RndSceneResource* const scene = static_cast<RndSceneResource*>(entity->mResource);
    for (const UILayoutCom::ReadyScene& ready : layout->mReadyScenes) {
        if (ready.mResource == scene && ready.mEntity == entity) {
            return;
        }
    }
    scene->RndSceneResource::_OnEntityReady(entity, entity);
    layout->mReadyScenes.push_back(UILayoutCom::ReadyScene{scene, entity});
    _ClearThreadedPollRange(root);
}

// Reconstructed from eboot.elf at 0x8ADB70.
bool UILayoutResource::HandleMsg(DataArray* msg, bool stopAtHandled) {
    Entity* const entity = mEntity;
    int index = static_cast<int>(entity->mPollOrder.mSize);
    if (index <= 0) {
        return false;
    }
    if (!stopAtHandled) {
        do {
            const GameObject* object =
                entity->SafeGetObject(entity->mPollOrder.data()[--index], false);
            if (object != nullptr) {
                Entity* const scene = SceneEntityOf(object);
                if (scene != nullptr) {
                    scene->Export(msg);
                }
            }
        } while (index > 0);
        return false;
    }
    do {
        const GameObject* object =
            entity->SafeGetObject(entity->mPollOrder.data()[index - 1], false);
        if (object != nullptr) {
            Entity* const scene = SceneEntityOf(object);
            if (scene != nullptr && scene->Export(msg)) {
                return true;
            }
        }
    } while (--index > 0);
    return false;
}

// Reconstructed from eboot.elf at 0x8ADCD0.
void UILayoutResource::AddSceneToFront(const ResourcePath& path, unsigned int flags) {
    QueuedScene scene;
    scene.mPath = path;
    scene.mMode = kQueuedSceneFront;
    scene.mAnchor.mId = 0xFFFFFFFFU;
    scene.mUseThreadedPoll = (flags & kSceneQueueThreadedPoll) != 0;
    mQueuedScenes.push_back(scene);
    if ((flags & kSceneQueueNow) != 0) {
        _AddQueuedScenes();
    }
}

// Reconstructed from eboot.elf at 0x8ADE80.
void UILayoutResource::AddSceneAtDrawOrder(
    const ResourcePath& path, GameObjectId before, unsigned int flags) {
    QueuedScene scene;
    scene.mPath = path;
    scene.mMode = kQueuedSceneBefore;
    scene.mAnchor = before;
    scene.mUseThreadedPoll = (flags & kSceneQueueThreadedPoll) != 0;
    mQueuedScenes.push_back(scene);
    if ((flags & kSceneQueueNow) != 0) {
        _AddQueuedScenes();
    }
}

// Reconstructed from eboot.elf at 0x8AE040.
void UILayoutResource::AddSceneAfterObject(
    const ResourcePath& path, GameObjectId after, unsigned int flags) {
    QueuedScene scene;
    scene.mPath = path;
    scene.mMode = kQueuedSceneAfter;
    scene.mAnchor = after;
    scene.mUseThreadedPoll = (flags & kSceneQueueThreadedPoll) != 0;
    mQueuedScenes.push_back(scene);
    if ((flags & kSceneQueueNow) != 0) {
        _AddQueuedScenes();
    }
}

// Reconstructed from eboot.elf at 0x8AE270. The queued additions of the
// path are compacted out in place.
void UILayoutResource::RemoveScene(const ResourcePath& path, unsigned int flags) {
    QueuedScene* const end = mQueuedScenes.end();
    QueuedScene* kept = mQueuedScenes.begin();
    for (QueuedScene* scene = kept; scene != end; ++scene) {
        if (scene->mPath != path) {
            *kept++ = *scene;
        }
    }
    mQueuedScenes.erase(kept, end);
    mQueuedRemovals.push_back(path);
    if ((flags & kSceneQueueNow) != 0) {
        _RemoveQueuedScenes();
    }
}

// Reconstructed from eboot.elf at 0x8AE430.
bool UILayoutResource::IsSceneQueuedForRemoval(const ResourcePath& path) const {
    for (const ResourcePath& removal : mQueuedRemovals) {
        if (removal == path) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x8AE480.
GameObject* UILayoutResource::_FindSceneObject(const ResourcePath& path) const {
    const Entity* const entity = mEntity;
    for (GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        const Entity* const scene = SceneEntityOf(object);
        if (scene != nullptr && scene->mResource->mPath == path) {
            return object;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x8AE560.
RndSceneInstanceCom* UILayoutResource::GetSceneInstance(const ResourcePath& path) const {
    const Entity* const entity = mEntity;
    for (GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        const Entity* const scene = SceneEntityOf(object);
        if (scene != nullptr && scene->mResource->mPath == path) {
            return object->GetCom<RndSceneInstanceCom>();
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x8AE670. The scene object's draw node
// is found without a bound.
void UILayoutResource::SetSceneShowing(const ResourcePath& path, bool showing) {
    Entity* const entity = mEntity;
    for (GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        Entity* const scene = SceneEntityOf(object);
        if (scene != nullptr && scene->mResource->mPath == path) {
            RndDrawNodeCom* const node = object->GetExistingCom<RndDrawNodeCom>();
            if (showing) {
                node->mShowHideFlags &= ~1U;
            } else {
                node->mShowHideFlags |= 1U;
            }
            return;
        }
    }
}

// Reconstructed from eboot.elf at 0x8AE790.
void UILayoutResource::SetScenesActive(bool active) {
    Entity* const entity = mEntity;
    for (GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        Entity* const scene = SceneEntityOf(object);
        if (scene != nullptr) {
            scene->SetActive(active);
        }
    }
}

// Reconstructed from eboot.elf at 0x8AE860.
void UILayoutResource::ForEachScene(std::function<void(RndSceneInstanceCom*)> function) const {
    const Entity* const entity = mEntity;
    for (long index = static_cast<int>(entity->mPollOrder.mSize); index > 0; --index) {
        const GameObject* object =
            entity->SafeGetObject(entity->mPollOrder.data()[index - 1], false);
        if (object == nullptr) {
            continue;
        }
        RndSceneInstanceCom* const instance = object->GetCom<RndSceneInstanceCom>();
        if (instance != nullptr) {
            function(instance);
        }
    }
}

// Reconstructed from eboot.elf at 0x8AE960.
eastl::vector<RndSceneInstanceCom*> UILayoutResource::GetSceneInstances() const {
    eastl::vector<RndSceneInstanceCom*> instances;
    const Entity* const entity = mEntity;
    long index = static_cast<int>(entity->mPollOrder.mSize);
    instances.reserve(index);
    for (; index > 0; --index) {
        const GameObject* object =
            entity->SafeGetObject(entity->mPollOrder.data()[index - 1], false);
        if (object != nullptr) {
            RndSceneInstanceCom* const instance = object->GetCom<RndSceneInstanceCom>();
            if (instance != nullptr) {
                instances.push_back(instance);
            }
        }
    }
    return instances;
}

// Reconstructed from eboot.elf at 0x8AEB90.
void UILayoutResource::Exit() {
    const unsigned int count = mEntity->mPollOrder.mSize;
    for (unsigned int i = 0; i < count; ++i) {
        const Entity* const entity = mEntity;
        const GameObject* object = entity->SafeGetObject(entity->mPollOrder.data()[i], false);
        if (object == nullptr) {
            continue;
        }
        Entity* const scene = SceneEntityOf(object);
        if (scene == nullptr) {
            continue;
        }
        UIInputCom* const input = scene->GetRoot()->GetBaseCom<UIInputCom>();
        if (input != nullptr) {
            input->Exit();
        }
    }
}

// Reconstructed from eboot.elf at 0x8AECC0. The scene objects are read
// without a check.
bool UILayoutResource::IsExiting() const {
    const unsigned int count = mEntity->mPollOrder.mSize;
    for (unsigned int i = 0; i < count; ++i) {
        const Entity* const entity = mEntity;
        Entity* const scene = SceneEntityOf(entity->GetObject(entity->mPollOrder.data()[i]));
        if (scene == nullptr) {
            continue;
        }
        const UIInputCom* const input = scene->GetRoot()->GetBaseCom<UIInputCom>();
        if (input != nullptr && input->IsExiting()) {
            return true;
        }
    }
    return mExiting;
}

// Reconstructed from eboot.elf at 0x8AEE40.
bool UILayoutResource::IsEntering() const {
    const unsigned int count = mEntity->mPollOrder.mSize;
    for (unsigned int i = 0; i < count; ++i) {
        const Entity* const entity = mEntity;
        Entity* const scene = SceneEntityOf(entity->GetObject(entity->mPollOrder.data()[i]));
        if (scene == nullptr) {
            continue;
        }
        const UIInputCom* const input = scene->GetRoot()->GetBaseCom<UIInputCom>();
        if (input != nullptr && input->IsEntering()) {
            return true;
        }
    }
    return mEntering;
}

// Reconstructed from eboot.elf at 0x8AF080.
void UILayoutResource::_GetShowingScenes(
    Entity* entity, SceneIdList& scenes, bool hmdMirrorOverlays) const {
    UILayoutCom* const layout = entity->GetRoot()->GetCom<UILayoutCom>();
    const PropArray<GameObjectId>& order =
        hmdMirrorOverlays ? layout->mHmdMirrorOverlays : layout->mDrawOrder;
    const unsigned long count = order.mSize;
    for (unsigned long i = 0; i < count; ++i) {
        const GameObjectId id = order.data()[i];
        if (_IsSceneInstanceShowing(entity, id)) {
            scenes.mData[scenes.mSize++] = id;
        }
    }
}

// Reconstructed from eboot.elf at 0x8AF240. Each overlay draws over the
// last, without clearing; the results are dropped.
void UILayoutResource::DrawHmdMirrorOverlays(RndBufferCollection* buffers) {
    RndSceneDrawParams params;
    params.mBuffers.push_back(buffers);
    RndDevice* const device = gRndDevice;
    if (device->mBeginFramePending) {
        device->_FlushPendingBeginFrame();
    }
    Entity* const entity = mEntity;
    const UILayoutCom* const layout = entity->GetRoot()->GetExistingCom<UILayoutCom>();
    SceneIdList scenes;
    scenes.mCapacity = layout->mHmdMirrorOverlays.mSize;
    scenes.mData = scenes.mCapacity != 0
        ? static_cast<GameObjectId*>(__builtin_alloca(scenes.mCapacity * sizeof(GameObjectId)))
        : nullptr;
    scenes.mSize = 0;
    _GetShowingScenes(entity, scenes, true);
    FixedVector<RndSceneDrawTarget, 2> targets;
    targets.resize(params.mBuffers.size());
    for (unsigned long i = 0; i < targets.size(); ++i) {
        targets[i].mBuffersIndex = i;
    }
    for (unsigned long i = 0; i < scenes.mSize; ++i) {
        const RndSceneInstanceCom* const instance =
            entity->GetObject(scenes.mData[i])->GetCom<RndSceneInstanceCom>();
        Entity* const scene = instance->mRuntime.mEntity;
        RndDrawableEntityResource* const resource =
            static_cast<RndDrawableEntityResource*>(scene->mResource);
        SceneIdList masks;
        masks.mCapacity = instance->mRuntimeData.mMaskingScenes.mSize;
        masks.mData = masks.mCapacity != 0
            ? static_cast<GameObjectId*>(__builtin_alloca(masks.mCapacity * sizeof(GameObjectId)))
            : nullptr;
        masks.mSize = 0;
        _GetMaskingScenes(entity, instance, masks);
        DrawMaskingScenes(this, entity, params, resource->mPath, {masks.mData, masks.mSize});
        params.mProjectionRect = instance->CalcProjectionRect(params.mBuffers[0]->mSize);
        params.mOutputToBackBuffer = i == scenes.mSize - 1;
        params.mDrawSceneMask = masks.mSize != 0;
        params.mClear = false;
        targets = resource->DrawImmediate(scene, params, &targets);
    }
}

// Reconstructed from eboot.elf at 0x8AF6D0.
void UILayoutResource::_GetMaskingScenes(
    Entity* entity,
    const RndSceneInstanceCom* instance,
    SceneIdList& scenes) const {
    scenes.mSize = 0;
    if (!gRndDevice->mSettings->mSceneMaskEnabled) {
        return;
    }
    const PropArray<GameObjectId>& masking = instance->mRuntimeData.mMaskingScenes;
    for (unsigned long i = 0; i < masking.mSize; ++i) {
        const GameObjectId id = masking.data()[i];
        if (_IsSceneInstanceShowing(entity, id)) {
            scenes.mData[scenes.mSize++] = id;
        }
    }
}

// Reconstructed from eboot.elf at 0x8AFB90.
void UILayoutResource::_SetSceneDrawParams(
    RndSceneDrawParams& params,
    const RndSceneInstanceCom* instance,
    unsigned long index,
    unsigned long count,
    unsigned long numMaskingScenes) const {
    params.mProjectionRect = instance->CalcProjectionRect(params.mBuffers[0]->mSize);
    params.mClear = index == 0;
    params.mOutputToBackBuffer = count - 1 == index;
    params.mDrawSceneMask = numMaskingScenes != 0;
}

// Reconstructed from eboot.elf at 0x8AFC20.
bool UILayoutResource::CreateRequiredComponents(GameObject* object, const ComMetaData& metaData) {
    if (!RndDrawableEntityResource::CreateRequiredComponents(object, metaData)) {
        return false;
    }
    if (metaData.mId != RndSceneInstanceCom::sClassName) {
        return true;
    }
    return object->CreateComponent(RndDrawNodeCom::sClassName, false) != nullptr;
}

// Reconstructed from eboot.elf at 0x8AFC80.
void UILayoutResource::DestroyDependentComponents(GameObject* object, const ComMetaData& metaData) {
    if (metaData.mId == RndDrawNodeCom::sClassName) {
        object->DestroyComponent(RndSceneInstanceCom::sId, true);
    }
    RndDrawableEntityResource::DestroyDependentComponents(object, metaData);
}

// Reconstructed from eboot.elf at 0x8AFCE0.
void UILayoutResource::_OnComponentCreated(Component* component) {
    if (component->GetClassName() != RndSceneInstanceCom::sClassName) {
        return;
    }
    GameObject* const object = component->mObject;
    UILayoutCom* const layout = object->mEntity->GetRoot()->GetCom<UILayoutCom>();
    GameObjectId id = object->mId;
    layout->mPollOrder._Insert(layout->mPollOrder.mSize, &id);
    id = object->mId;
    layout->mDrawOrder._Insert(layout->mDrawOrder.mSize, &id);
}

// Reconstructed from eboot.elf at 0x8AFDC0. The scenes before
// RndConfig::mFirstDrawnScene and from mLastDrawnScene on are left out;
// without scenes the buffers are cleared.
FixedVector<RndSceneDrawTarget, 2> UILayoutResource::_Draw(
    Entity* entity,
    RndSceneDrawParams& params,
    const FixedVector<RndSceneDrawTarget, 2>* previous) {
    static_cast<void>(previous);
    RndSceneDrawParams drawParams(params);
    RndDevice* const device = gRndDevice;
    if (device->mBeginFramePending) {
        device->_FlushPendingBeginFrame();
    }
    RndContext* const context = device->mImmediateContext;
    const RndConfig* const settings = gRndDevice->mSettings;
    const UILayoutCom* const layout = entity->GetRoot()->GetExistingCom<UILayoutCom>();
    SceneIdList scenes;
    scenes.mCapacity = layout->mDrawOrder.mSize;
    scenes.mData = scenes.mCapacity != 0
        ? static_cast<GameObjectId*>(__builtin_alloca(scenes.mCapacity * sizeof(GameObjectId)))
        : nullptr;
    scenes.mSize = 0;
    _GetShowingScenes(entity, scenes, false);
    const unsigned long last = static_cast<unsigned long>(settings->mLastDrawnScene);
    if (last < scenes.mSize) {
        scenes.mSize = last;
    }
    const unsigned long first = static_cast<unsigned long>(settings->mFirstDrawnScene);
    if (first != 0) {
        const unsigned long skip = first < scenes.mSize ? first : scenes.mSize;
        for (unsigned long i = skip; i < scenes.mSize; ++i) {
            scenes.mData[i - skip] = scenes.mData[i];
        }
        scenes.mSize -= skip;
    }
    FixedVector<RndSceneDrawTarget, 2> targets;
    targets.resize(drawParams.mBuffers.size());
    for (unsigned long i = 0; i < targets.size(); ++i) {
        targets[i].mBuffersIndex = i;
    }
    if (scenes.mSize == 0) {
        const VectorAdapter<RndBufferCollection*> buffers = {
            drawParams.mBuffers.mData, drawParams.mBuffers.mSize};
        RndDrawUtl::ClearBuffers(*context, buffers, drawParams.mClearColor);
        return targets;
    }
    for (unsigned long i = 0; i < scenes.mSize; ++i) {
        const RndSceneInstanceCom* const instance =
            entity->GetObject(scenes.mData[i])->GetCom<RndSceneInstanceCom>();
        Entity* const scene = instance->mRuntime.mEntity;
        RndDrawableEntityResource* const resource =
            static_cast<RndDrawableEntityResource*>(scene->mResource);
        SceneIdList masks;
        masks.mCapacity = instance->mRuntimeData.mMaskingScenes.mSize;
        masks.mData = masks.mCapacity != 0
            ? static_cast<GameObjectId*>(__builtin_alloca(masks.mCapacity * sizeof(GameObjectId)))
            : nullptr;
        masks.mSize = 0;
        _GetMaskingScenes(entity, instance, masks);
        DrawMaskingScenes(this, entity, drawParams, resource->mPath, {masks.mData, masks.mSize});
        _SetSceneDrawParams(drawParams, instance, i, scenes.mSize, masks.mSize);
        targets = resource->DrawImmediate(scene, drawParams, &targets);
    }
    return targets;
}

// Reconstructed from eboot.elf at 0x8B05E0. Each scene's draw jobs run
// after its job, which waits for the previous scene's draw, and before the
// next scene's job; without scenes the clear job runs between the context
// job and the end. The scene jobs are created as needed and kept.
void UILayoutResource::_StartDrawJobs(
    Entity* entity,
    const RndSceneDrawParams& params,
    Entity* after,
    PollDepBase* startDep,
    PollDepBase* contextDep,
    PollDepBase* endAfter,
    eastl::vector<PollDepBase*>& jobs) {
    static_cast<void>(after);
    RndSceneDrawParams drawParams(params);
    const UILayoutCom* const layout = entity->GetRoot()->GetExistingCom<UILayoutCom>();
    SceneIdList scenes;
    scenes.mCapacity = layout->mDrawOrder.mSize;
    scenes.mData = scenes.mCapacity != 0
        ? static_cast<GameObjectId*>(__builtin_alloca(scenes.mCapacity * sizeof(GameObjectId)))
        : nullptr;
    scenes.mSize = 0;
    _GetShowingScenes(entity, scenes, false);
    const unsigned long count = scenes.mSize;
    if (count == 0) {
        mClearJob.mParams = drawParams;
        mClearJob.AddPollDep(contextDep);
        endAfter->AddPollDep(&mClearJob);
        return;
    }
    while (mSceneDrawJobs.size() < count) {
        mSceneDrawJobs.push_back(new SceneDrawJob(this));
    }
    mSceneDrawJobs[0]->AddPollDep(contextDep);
    Entity* previous = nullptr;
    for (unsigned long i = 0; i < count; ++i) {
        const RndSceneInstanceCom* const instance =
            entity->GetObject(scenes.mData[i])->GetExistingCom<RndSceneInstanceCom>();
        Entity* const scene = instance->mRuntime.mEntity;
        RndDrawableEntityResource* const resource =
            static_cast<RndDrawableEntityResource*>(scene->mResource);
        SceneDrawJob* const job = static_cast<SceneDrawJob*>(mSceneDrawJobs[i]);
        job->mEntity = entity;
        job->mParams = drawParams;
        job->mScenePath = resource->mPath;
        _GetMaskingScenes(entity, instance, job->mMaskingScenes);
        _SetSceneDrawParams(drawParams, instance, i, count, job->mMaskingScenes.size());
        PollDepBase* const next = i + 1 < count ? mSceneDrawJobs[i + 1] : endAfter;
        resource->StartDrawJobs(scene, drawParams, previous, startDep, job, next, jobs);
        previous = scene;
    }
}

// Reconstructed from eboot.elf at 0x8B0E90.
void UILayoutResource::_GetMaskingScenes(
    Entity* entity,
    const RndSceneInstanceCom* instance,
    eastl::vector<GameObjectId>& scenes) const {
    scenes.clear();
    if (!gRndDevice->mSettings->mSceneMaskEnabled) {
        return;
    }
    const PropArray<GameObjectId>& masking = instance->mRuntimeData.mMaskingScenes;
    for (unsigned long i = 0; i < masking.mSize; ++i) {
        const GameObjectId id = masking.data()[i];
        if (_IsSceneInstanceShowing(entity, id)) {
            scenes.push_back(id);
        }
    }
}

// Reconstructed from eboot.elf at 0x8B1100. Each scene's jobs are
// unhooked; the last scene's result is returned.
FixedVector<RndSceneDrawTarget, 2> UILayoutResource::_FinishDrawJobs(Entity* entity) {
    const UILayoutCom* const layout = entity->GetRoot()->GetExistingCom<UILayoutCom>();
    SceneIdList scenes;
    scenes.mCapacity = layout->mDrawOrder.mSize;
    scenes.mData = scenes.mCapacity != 0
        ? static_cast<GameObjectId*>(__builtin_alloca(scenes.mCapacity * sizeof(GameObjectId)))
        : nullptr;
    scenes.mSize = 0;
    _GetShowingScenes(entity, scenes, false);
    FixedVector<RndSceneDrawTarget, 2> targets;
    if (scenes.mSize == 0) {
        mClearJob.ClearAllDeps();
        return targets;
    }
    for (unsigned long i = 0; i < scenes.mSize; ++i) {
        mSceneDrawJobs[i]->ClearAllDeps();
        Entity* const scene = ShowingSceneEntity(entity, scenes.mData[i]);
        targets = static_cast<RndDrawableEntityResource*>(scene->mResource)->FinishDrawJobs(scene);
    }
    return targets;
}

// Reconstructed from eboot.elf at 0x8B1350. The layout component is
// searched without a bound.
void UILayoutResource::_DrawTexRenderers(Entity* entity) {
    const UILayoutCom* const layout = entity->GetRoot()->GetExistingCom<UILayoutCom>();
    // The list's storage is on the stack, as many ids as the draw order.
    SceneIdList scenes;
    scenes.mCapacity = layout->mDrawOrder.mSize;
    scenes.mData = scenes.mCapacity != 0
        ? static_cast<GameObjectId*>(__builtin_alloca(scenes.mCapacity * sizeof(GameObjectId)))
        : nullptr;
    scenes.mSize = 0;
    _GetShowingScenes(entity, scenes, false);
    for (unsigned long i = 0; i < scenes.mSize; ++i) {
        Entity* const scene = ShowingSceneEntity(entity, scenes.mData[i]);
        static_cast<RndDrawableEntityResource*>(scene->mResource)->DrawTexRenderers(scene);
    }
}

// Reconstructed from eboot.elf at 0x8B14A0.
void UILayoutResource::_StartTexRendererJobs(
    Entity* entity,
    PollDepBase* head,
    PollDepBase* afterTexRenderers,
    eastl::vector<PollDepBase*>& jobs) {
    const UILayoutCom* const layout = entity->GetRoot()->GetExistingCom<UILayoutCom>();
    // The list's storage is on the stack, as many ids as the draw order.
    SceneIdList scenes;
    scenes.mCapacity = layout->mDrawOrder.mSize;
    scenes.mData = scenes.mCapacity != 0
        ? static_cast<GameObjectId*>(__builtin_alloca(scenes.mCapacity * sizeof(GameObjectId)))
        : nullptr;
    scenes.mSize = 0;
    _GetShowingScenes(entity, scenes, false);
    for (unsigned long i = 0; i < scenes.mSize; ++i) {
        Entity* const scene = ShowingSceneEntity(entity, scenes.mData[i]);
        static_cast<RndDrawableEntityResource*>(scene->mResource)
            ->StartTexRendererJobs(scene, head, afterTexRenderers, jobs);
    }
}

// Reconstructed from eboot.elf at 0x8B1600.
void UILayoutResource::_FinishTexRendererJobs(Entity* entity, PollDepBase* afterTexRenderers) {
    const UILayoutCom* const layout = entity->GetRoot()->GetExistingCom<UILayoutCom>();
    // The list's storage is on the stack, as many ids as the draw order.
    SceneIdList scenes;
    scenes.mCapacity = layout->mDrawOrder.mSize;
    scenes.mData = scenes.mCapacity != 0
        ? static_cast<GameObjectId*>(__builtin_alloca(scenes.mCapacity * sizeof(GameObjectId)))
        : nullptr;
    scenes.mSize = 0;
    _GetShowingScenes(entity, scenes, false);
    for (unsigned long i = 0; i < scenes.mSize; ++i) {
        Entity* const scene = ShowingSceneEntity(entity, scenes.mData[i]);
        static_cast<RndDrawableEntityResource*>(scene->mResource)
            ->FinishTexRendererJobs(scene, afterTexRenderers);
    }
}

// Reconstructed from eboot.elf at 0x8B1750. The entity root's draw node is
// found without a bound.
bool UILayoutResource::_IsSceneInstanceShowing(Entity* entity, GameObjectId id) const {
    const GameObject* const object = entity->SafeGetObject(id, false);
    if (object == nullptr) {
        return false;
    }
    const Entity* const scene = SceneEntityOf(object);
    if (scene == nullptr || (scene->mFlags & 0x100) == 0) {
        return false;
    }
    const RndDrawNodeCom* const node = object->GetCom<RndDrawNodeCom>();
    if (node != nullptr && (node->mRuntime.mWorldShowHideFlags & 1) != 0) {
        return false;
    }
    return (scene->GetRoot()->GetExistingCom<RndDrawNodeCom>()->mRuntime.mWorldShowHideFlags & 1) == 0;
}

// Reconstructed from eboot.elf at 0x8B1840.
bool UILayoutResource::UpdateSkipDrawState() {
    if (mFramesToSkipDraw == 0) {
        return false;
    }
    --mFramesToSkipDraw;
    return true;
}

// Reconstructed from eboot.elf at 0x8B18A0.
void UILayoutResource::ClearJob::ThreadPoll(const int& thread) {
    static_cast<void>(thread);
    RndDevice* const device = gRndDevice;
    if (device->mBeginFramePending) {
        device->_FlushPendingBeginFrame();
    }
    const VectorAdapter<RndBufferCollection*> buffers = {
        mParams.mBuffers.mData, mParams.mBuffers.mSize};
    RndDrawUtl::ClearBuffers(*device->mImmediateContext, buffers, mParams.mClearColor);
}

// Reconstructed from eboot.elf at 0x8B1980.
void UILayoutResource::SceneDrawJob::ThreadPoll(const int& thread) {
    static_cast<void>(thread);
    const VectorAdapter<GameObjectId> scenes = {mMaskingScenes.begin(), mMaskingScenes.size()};
    DrawMaskingScenes(mLayout, mEntity, mParams, mScenePath, scenes);
}

// Reconstructed from eboot.elf at 0x8B1A20.
ResourceMetaData* UILayoutResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x8B1A30.
Symbol UILayoutResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x8B1AD0.
bool UILayoutResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr;
         metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x8B1B00.
bool UILayoutResource::IsTransient() const {
    return false;
}
