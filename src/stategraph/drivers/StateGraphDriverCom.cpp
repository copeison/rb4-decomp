// stategraph/StateGraphDriverCom.o (0x7013A0 to 0x703CB5). The class's
// registration and factory are emitted in stategraph/StateGraph.o.
#include "stategraph/drivers/StateGraphDriverCom.h"

#include <new>

#include "anim/drivers/AnimDriverCom.h"
#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/system/SoundManager.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/TransEntityResource.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropUtl.h"
#include "entity/resources/ResourceLoader.h"
#include "stategraph/nodes/StateNodeRoot.h"
#include "stategraph/resources/StateGraphResource.h"
#include "stategraph/runtime/StateGraphInstance.h"

namespace {

// The null object id of a shared header, which the static initializer sets
// to -1 (0x1AB2980). Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The resource at the path: the inline resource of the entity resource
// with the path, or the loaded one. The resource class id is evaluated for
// an assertion compiled out of this build. Name not in the reference map.
// Reconstructed from eboot.elf at 0x702B30.
ResourcePtr<Resource> GetInlineOrLoaded(const EntityResource& owner, ResourcePath path) {
    ResourcePtr<Resource> inlined = owner.TryGetInline<Resource>(path);
    if (inlined != nullptr) {
        return ResourcePtr<Resource>(inlined.Get());
    }
    static_cast<void>(Resource::Id());
    ResourcePtr<Resource> found;
    found = Resource::Get(path).Get();
    return ResourcePtr<Resource>(found.Get());
}

}  // namespace

// The object's statics, in the order of its static initializer (0x703B30).
CritSec StateGraphDriverCom::sGlobalSoundHandlesLock;
eastl::map<Symbol, unsigned int> StateGraphDriverCom::sGlobalSoundHandles;
Symbol StateGraphDriverCom::sId("StateGraphDriver");
Symbol StateGraphDriverCom::sClassName("StateGraphDriver");
PropRegistry StateGraphDriverCom::sPropRegistry;
ComMetaData StateGraphDriverCom::sMetaData;

// Reconstructed from eboot.elf at 0x7013A0.
StateGraphDriverCom::RuntimeData::RuntimeData()
    : mLoader(nullptr), mPollCount(0), mNextPollEven(true), mClock(nullptr) {}

// Reconstructed from eboot.elf at 0x7014F0.
StateGraphDriverCom::RuntimeData::~RuntimeData() {}

// Reconstructed from eboot.elf at 0x7015B0. The deleting destructor is at
// 0x7015E0.
StateGraphDriverCom::~StateGraphDriverCom() {}

// Reconstructed from eboot.elf at 0x701620.
void StateGraphDriverCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    SoundManager::sAddSoundHandleFunc = &StateGraphDriverCom::AddSoundHandle;
    StateGraphDriverCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }
    metadata.mDescription = "Manages StateGraph instances running within an entity.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mCategory = ComMetaData::kCategoryMiscellaneous;
    metadata.mAllowedResources.push_back(TransEntityResource::Id());
    metadata.mLightweight = true;
}

// Reconstructed from eboot.elf at 0x701970.
bool StateGraphDriverCom::AddSoundHandle(unsigned int handle, Symbol name) {
    if (name == Symbol()) {
        return false;
    }
    ScopedCritSec lock(sGlobalSoundHandlesLock);
    const auto found = sGlobalSoundHandles.find(name);
    if (found != sGlobalSoundHandles.end()) {
        AudioGenerator* const playing = theSoundManager.LockIfOwned(found->second);
        if (playing != nullptr) {
            --playing->mRefCount;
            return false;
        }
    }
    sGlobalSoundHandles[name] = handle;
    return true;
}

// Reconstructed from eboot.elf at 0x701B20. A running instance is kept
// when no object is given.
void StateGraphDriverCom::Activate(ResourcePath path, GameObjectId object) {
    if (_FindInst(path) != mRuntime.mInstances.end()) {
        if (object.mId == gNullObjectId.mId) {
            return;
        }
        Deactivate(path);
    }
    StateGraphInstance* const instance =
        new StateGraphInstance(mObject->mEntity, mRuntime.mClock, path);
    if (instance->GetResource()->GetRootNode()->mPollFrequency == StateNodeRoot::kPollEveryOtherFrame) {
        instance->mPollPhase = mRuntime.mNextPollEven ? StateGraphInstance::kPollEvenFrames
                                                      : StateGraphInstance::kPollOddFrames;
        mRuntime.mNextPollEven = !mRuntime.mNextPollEven;
    }
    StateGraphResource* const resource = instance->GetResource();
    if (resource != nullptr && !resource->Fail()) {
        mRuntime.mInstances.push_back(instance);
        instance->Activate(object);
    } else {
        delete instance;
    }
}

// Reconstructed from eboot.elf at 0x701D70.
bool StateGraphDriverCom::IsActive(ResourcePath path) const {
    return _FindInst(path) != mRuntime.mInstances.end();
}

// Reconstructed from eboot.elf at 0x701DD0.
void StateGraphDriverCom::Deactivate(ResourcePath path) {
    StateGraphInstance** const found = _FindInst(path);
    if (found != mRuntime.mInstances.end()) {
        StateGraphInstance* const instance = *found;
        instance->Deactivate();
        delete instance;
        mRuntime.mInstances.erase(found);
    }
}

// Reconstructed from eboot.elf at 0x701E60.
StateGraphInstance** StateGraphDriverCom::_FindInst(ResourcePath path) {
    StateGraphInstance** instance = mRuntime.mInstances.begin();
    for (; instance != mRuntime.mInstances.end(); ++instance) {
        if ((*instance)->GetResource()->mPath == path) {
            break;
        }
    }
    return instance;
}

// Reconstructed from eboot.elf at 0x701EB0.
void StateGraphDriverCom::DeactivateAll() {
    while (!mRuntime.mInstances.empty()) {
        StateGraphInstance* const instance = mRuntime.mInstances.back();
        instance->Deactivate();
        delete instance;
        mRuntime.mInstances.pop_back();
    }
}

// Reconstructed from eboot.elf at 0x701F10.
StateGraphInstance* const* StateGraphDriverCom::_FindInst(ResourcePath path) const {
    StateGraphInstance* const* instance = mRuntime.mInstances.begin();
    for (; instance != mRuntime.mInstances.end(); ++instance) {
        if ((*instance)->GetResource()->mPath == path) {
            break;
        }
    }
    return instance;
}

// Reconstructed from eboot.elf at 0x701F60.
bool StateGraphDriverCom::AddSoundHandle(unsigned int handle, Symbol name, SoundHandleScope scope) {
    if (name == Symbol()) {
        mRuntime.mUnnamedSoundHandles.push_back(handle);
        return true;
    }
    unsigned int existing;
    if (HasSoundHandle(name, existing, scope)) {
        AudioGenerator* const playing = theSoundManager.LockIfOwned(existing);
        if (playing != nullptr) {
            --playing->mRefCount;
            return false;
        }
    }
    if (scope == kSoundHandleLocal) {
        mRuntime.mSoundHandles[name] = handle;
        return true;
    }
    ScopedCritSec lock(sGlobalSoundHandlesLock);
    sGlobalSoundHandles[name] = handle;
    return true;
}

// Reconstructed from eboot.elf at 0x702180.
bool StateGraphDriverCom::HasSoundHandle(Symbol name, unsigned int& handle, SoundHandleScope scope) {
    if (name == Symbol()) {
        return false;
    }
    if (scope == kSoundHandleLocal || scope == kSoundHandleAny) {
        const auto found = mRuntime.mSoundHandles.find(name);
        if (found != mRuntime.mSoundHandles.end()) {
            handle = found->second;
            return true;
        }
    }
    if (scope != kSoundHandleLocal) {
        ScopedCritSec lock(sGlobalSoundHandlesLock);
        const auto found = sGlobalSoundHandles.find(name);
        if (found != sGlobalSoundHandles.end()) {
            handle = found->second;
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x7022E0.
bool StateGraphDriverCom::HasSoundHandle(Symbol name, unsigned int& handle) {
    ScopedCritSec lock(sGlobalSoundHandlesLock);
    if (name == Symbol()) {
        return false;
    }
    const auto found = sGlobalSoundHandles.find(name);
    if (found == sGlobalSoundHandles.end()) {
        return false;
    }
    handle = found->second;
    return true;
}

// Reconstructed from eboot.elf at 0x7023C0.
bool StateGraphDriverCom::RemoveSoundHandle(unsigned int handle, SoundHandleScope scope) {
    bool removed = false;
    if (scope == kSoundHandleLocal || scope == kSoundHandleAny) {
        for (auto entry = mRuntime.mSoundHandles.begin(); entry != mRuntime.mSoundHandles.end();) {
            const auto current = entry;
            ++entry;
            if (current->second == handle) {
                mRuntime.mSoundHandles.erase(current);
                removed = true;
            }
        }
    }
    if (scope == kSoundHandleGlobal || scope == kSoundHandleAny) {
        ScopedCritSec lock(sGlobalSoundHandlesLock);
        for (auto entry = sGlobalSoundHandles.begin(); entry != sGlobalSoundHandles.end();) {
            const auto current = entry;
            ++entry;
            if (current->second == handle) {
                sGlobalSoundHandles.erase(current);
                removed = true;
            }
        }
    }
    return removed;
}

// Reconstructed from eboot.elf at 0x702520.
bool StateGraphDriverCom::RemoveGlobalSoundHandle(unsigned int handle) {
    ScopedCritSec lock(sGlobalSoundHandlesLock);
    bool removed = false;
    for (auto entry = sGlobalSoundHandles.begin(); entry != sGlobalSoundHandles.end();) {
        const auto current = entry;
        ++entry;
        if (current->second == handle) {
            sGlobalSoundHandles.erase(current);
            removed = true;
        }
    }
    return removed;
}

// Reconstructed from eboot.elf at 0x7025F0.
void StateGraphDriverCom::StopGlobalSounds() {
    ScopedCritSec lock(sGlobalSoundHandlesLock);
    for (auto entry = sGlobalSoundHandles.begin(); entry != sGlobalSoundHandles.end();) {
        AudioGenerator* const playing = theSoundManager.LockIfOwned(entry->second);
        if (playing != nullptr) {
            playing->Stop();
            --playing->mRefCount;
        }
        entry = sGlobalSoundHandles.erase(entry);
    }
}

// Reconstructed from eboot.elf at 0x7026D0. An empty path or a resource
// that needs no load is set at once.
void StateGraphDriverCom::LoadAndSetResourcePathProperty(const PropRef& ref, ResourcePath path) {
    if (mRuntime.mLoader != nullptr && mRuntime.mLoadingRef == ref) {
        if (mRuntime.mLoader->mPath == path) {
            return;
        }
        ResourceLoader::ReleaseLoader(mRuntime.mLoader);
        mRuntime.mLoadingRef = PropRef();
    } else {
        ResourceLoaderJob* job = mRuntime.mLoaderJobs.begin();
        for (; job != mRuntime.mLoaderJobs.end(); ++job) {
            if (job->mRef == ref) {
                break;
            }
        }
        if (job != mRuntime.mLoaderJobs.end()) {
            if (job->mPath == path) {
                return;
            }
            mRuntime.mLoaderJobs.erase(job);
        }
    }
    Entity* const entity = mObject->mEntity;
    if (path.mPath != Symbol()) {
        if (GetInlineOrLoaded(*entity->mResource, path) == nullptr) {
            ResourceLoaderJob& job = mRuntime.mLoaderJobs.emplace_back();
            job.mRef = ref;
            job.mPath = path;
            return;
        }
    }
    DynamicSetResourcePathProperty(entity, ref, path);
}

// Reconstructed from eboot.elf at 0x702CA0.
bool StateGraphDriverCom::IsLoadingResources() const {
    return !mRuntime.mLoaderJobs.empty();
}

// Reconstructed from eboot.elf at 0x702CC0.
StateNodeInstance* StateGraphDriverCom::FindNodeInstance(const GameObject* object) {
    for (StateGraphInstance* instance : mRuntime.mInstances) {
        StateNodeInstance* const found = instance->FindNodeInstance(object);
        if (found != nullptr) {
            return found;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x702D10.
void StateGraphDriverCom::_GetComponentOrderDeps(
    eastl::vector<Symbol>& follows,
    eastl::vector<Symbol>& precedes) {
    static_cast<void>(follows);
    precedes.push_back(AnimDriverCom::sClassName);
}

// Reconstructed from eboot.elf at 0x702DD0.
bool StateGraphDriverCom::_OnResourcesLoaded() {
    mRuntime.mClock = TheTimeMgr->GetClock(mObject->mEntity);
    return true;
}

// Reconstructed from eboot.elf at 0x702E00. The list is reserved for the
// inlined state graphs before any is activated.
void StateGraphDriverCom::_Enter() {
    const EntityResource* const owner = mObject->mEntity->mResource;
    unsigned long count = 0;
    for (const EntityResource::LayerInfo& layer : owner->mLayers) {
        for (Resource* resource : layer.mInlineResources) {
            count += resource->IsA(StateGraphResource::Id());
        }
    }
    mRuntime.mInstances.reserve(count);
    for (const EntityResource::LayerInfo& layer : mObject->mEntity->mResource->mLayers) {
        for (Resource* resource : layer.mInlineResources) {
            if (resource->IsA(StateGraphResource::Id()) &&
                static_cast<StateGraphResource*>(resource)->GetRootNode()->mStartOnEnter) {
                Activate(resource->mPath, gNullObjectId);
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x7030D0.
void StateGraphDriverCom::_Poll() {
    ++mRuntime.mPollCount;
    for (StateGraphInstance** instance = mRuntime.mInstances.begin();
         instance != mRuntime.mInstances.end();) {
        StateGraphInstance* const polled = *instance;
        if (polled->ShouldPoll(mRuntime.mPollCount) && !polled->Poll()) {
            delete polled;
            mRuntime.mInstances.erase(instance);
        } else {
            ++instance;
        }
    }
    for (ResourceLoader*& loader : mRuntime.mFinishedLoaders) {
        ResourceLoader::ReleaseLoader(loader);
    }
    mRuntime.mFinishedLoaders.clear();
    for (;;) {
        if (mRuntime.mLoader == nullptr) {
            if (mRuntime.mLoaderJobs.empty()) {
                break;
            }
            ResourceLoaderJob& job = mRuntime.mLoaderJobs.front();
            mRuntime.mLoadingRef = job.mRef;
            const bool start = true;
            mRuntime.mLoader = ResourceLoader::AcquireLoader(job.mPath, start);
            mRuntime.mLoaderJobs.erase(mRuntime.mLoaderJobs.begin());
        }
        if (!mRuntime.mLoader->mDone) {
            break;
        }
        ResourcePath path = mRuntime.mLoader->mPath;
        DynamicSetResourcePathProperty(mObject->mEntity, mRuntime.mLoadingRef, path);
        mRuntime.mFinishedLoaders.push_back(mRuntime.mLoader);
        mRuntime.mLoader = nullptr;
        mRuntime.mLoadingRef = PropRef();
    }
    for (auto handle = mRuntime.mUnnamedSoundHandles.begin();
         handle != mRuntime.mUnnamedSoundHandles.end();) {
        AudioGenerator* const playing = theSoundManager.LockIfOwned(*handle);
        if (playing != nullptr) {
            --playing->mRefCount;
            ++handle;
        } else {
            handle = mRuntime.mUnnamedSoundHandles.erase(handle);
        }
    }
}

// Reconstructed from eboot.elf at 0x7034F0. The finished loaders are
// released but stay listed.
void StateGraphDriverCom::_Exit(DestroyType type) {
    static_cast<void>(type);
    for (auto& entry : mRuntime.mSoundHandles) {
        theSoundManager.StopSound(entry.second);
    }
    mRuntime.mSoundHandles.clear();
    for (unsigned int handle : mRuntime.mUnnamedSoundHandles) {
        theSoundManager.StopSound(handle);
    }
    mRuntime.mUnnamedSoundHandles.clear();
    for (ResourceLoader*& loader : mRuntime.mFinishedLoaders) {
        ResourceLoader::ReleaseLoader(loader);
    }
    if (mRuntime.mLoader != nullptr) {
        ResourceLoader::ReleaseLoader(mRuntime.mLoader);
    }
    for (StateGraphInstance* instance : mRuntime.mInstances) {
        delete instance;
    }
    mRuntime.mInstances.clear();
}

// Reconstructed from eboot.elf at 0x703660.
Symbol StateGraphDriverCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x703670.
Symbol StateGraphDriverCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x703680.
int StateGraphDriverCom::CurrentRev() const {
    return const_cast<StateGraphDriverCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x7036A0.
bool StateGraphDriverCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x7036D0.
Component* StateGraphDriverCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x7036E0. The copy goes at the buffer's
// next 8-byte boundary, built as a new driver, and its properties follow it.
char* StateGraphDriverCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateGraphDriverCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateGraphDriverCom();
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x703920.
PropRegistry& StateGraphDriverCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x703930.
ComMetaData& StateGraphDriverCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x6FED90. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateGraphDriverCom::_Create() {
    return new StateGraphDriverCom();
}

// Reconstructed from eboot.elf at 0x6F6330. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateGraphDriverCom::Init() {
    InitComponentClass<StateGraphDriverCom>();
}
