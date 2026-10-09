#include "entity/core/EntityResource.h"

#include <kernel.h>

#include "entity/core/ComMetaData.h"
#include "entity/core/EditorCom.h"
#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/core/InstanceCom.h"
#include "entity/progress/LoadProgress.h"
#include "os/files/File.h"
#include "os/profiling/PerfMgr.h"
#include "utl/files/FileUtl.h"
#include "utl/streams/BinStream.h"
#include "utl/text/MakeString.h"

// The profiled path at 0x19E3000 and the class's metadata at 0x19E3008.
ResourcePath EntityResource::sProfileLoadPath;
ResourceMetaData EntityResource::sMetaData;

// The folder at 0x19B02E0.
const char* gEntityLoadDir = ".";

namespace {

// The task the entity loads report to the load-progress listeners. Name
// not in the reference map.
constexpr const char kEntityLoadTask[] = "EntityLoad";

// The revision Save writes and the newest _LoadEntity reads. Name not in
// the reference map.
constexpr int kEntityResourceSaveRev = 18;

// The id of no object, at 0x19E2FF8; EntityResource.o's initializer
// (0x103726) sets it. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFu};

// The resource whose load the calling thread reports: a TLSValue whose
// descriptor and offset are at 0x19B02E8. Name not in the reference map.
thread_local const EntityResource* gProfilingLoad;

// Releases each layer's inlined resources from the last, after clearing
// their inlined flag. Inlined into the destructor and _LoadEntity; the
// map's ClearAllInlineResources() presumably. Name not in the reference
// map.
void ReleaseInlineResources(eastl::vector<EntityResource::LayerInfo>& layers) {
    const unsigned long numLayers = layers.size();
    for (unsigned long layer = 0; layer < numLayers; ++layer) {
        eastl::vector<Resource*>& resources = layers[layer].mInlineResources;
        for (unsigned long index = resources.size(); index-- != 0;) {
            Resource* const resource = resources[index];
            resources.erase(resources.begin() + index);
            resource->mInlined = false;
            resource->ReleaseRef();
        }
    }
}

}  // namespace

// The class symbol of the component that instances an entity resource,
// whose registration at 0x118E60 describes it as "Used to load an entity
// resource". Its class name is not recovered. Name not in the reference
// map.
extern Symbol gEntityInstanceComId;  // 0x19E3280

// Reconstructed from eboot.elf at 0xFD5D0.
void EntityResource::SetProfileLoadPath(ResourcePath path) {
    static_cast<void>(scePthreadSelf());
    sProfileLoadPath = path;
}

// Reconstructed from eboot.elf at 0xFD610. The binary forwards the
// arguments to Entity::_LoadRoot.
void EntityResource::_LoadRoot(
    BinStream& stream,
    Entity* entity,
    eastl::vector<ResourcePath>* paths,
    eastl::vector<unsigned char>* rootData) {
    Entity::_LoadRoot(stream, entity, paths, rootData);
}

// Reconstructed from eboot.elf at 0xFD630.
Entity* EntityResource::_NewEntity() {
    mEntity = new Entity(1);
    mEntity->mResource = this;
    mEntity->mFlags = (mEntity->mFlags & ~0x800u) | (static_cast<unsigned int>(_IsEditorEntity()) << 11);
    return mEntity;
}

// Reconstructed from eboot.elf at 0xFD6A0. The root may not be deleted or
// have its properties changed in the editor.
Entity* EntityResource::CreateEntity() {
    _NewEntity();
    GameObject* const root = mEntity->CreateObject(0, 0);
    root->SetName(Symbol("root"));
    root->CreateComponent(InstanceCom::sClassName, false);
    for (const GameObject::ComIndex& entry : root->mComs) {
        if (entry.mId == EditorCom::sId) {
            EditorCom* const editor = static_cast<EditorCom*>(entry.mCom);
            if (editor != nullptr) {
                editor->RevokeEditorCapability(root, EditorCom::kCanDelete);
                editor->RevokeEditorCapability(root, EditorCom::kCanChangeProperties);
            }
            break;
        }
    }
    return mEntity;
}

// Reconstructed from eboot.elf at 0xFD7E0.
void EntityResource::InitObject(GameObject* object) {
    object->CreateComponent(EditorCom::sClassName, false);
}

// Reconstructed from eboot.elf at 0xFD800.
void EntityResource::DestroyEntity() {
    if (mEntity != nullptr) {
        mEntity->_Destroy();
        mEntity = nullptr;
    }
}

// Reconstructed from eboot.elf at 0xFD830.
Entity* EntityResource::SwapEntity(Entity* entity) {
    Entity* const old = mEntity;
    mEntity = entity;
    if (entity != nullptr) {
        entity->mResource = this;
    }
    return old;
}

// Reconstructed from eboot.elf at 0xFD850.
void EntityResource::EnterEntity(Entity* entity, unsigned int flags) {
    if (entity != nullptr) {
        _EnterEntity(entity, flags);
    }
}

// Reconstructed from eboot.elf at 0xFD870.
void EntityResource::_EnterEntity(Entity* entity, unsigned int flags) {
    entity->Enter(flags);
}

// Reconstructed from eboot.elf at 0xFD880.
void EntityResource::ExitEntity(Entity* entity, unsigned int flags) {
    if (entity != nullptr) {
        _ExitEntity(entity, flags);
    }
}

// Reconstructed from eboot.elf at 0xFD8A0.
void EntityResource::_ExitEntity(Entity* entity, unsigned int flags) {
    entity->Exit(flags);
}

// Reconstructed from eboot.elf at 0xFD8B0.
void EntityResource::PollEntity(Entity* entity) {
    if (entity != nullptr) {
        _PollEntity(entity);
    }
}

// Reconstructed from eboot.elf at 0xFD8D0.
void EntityResource::_PollEntity(Entity* entity) {
    entity->_Poll();
}

// Reconstructed from eboot.elf at 0xFD8E0. Only an entered entity (flag
// bits 1, 2 and 4 equal to 0x12) is reported, to the resource of its
// topmost ancestor with the ancestor's child on the way. The resource
// itself is not used.
void EntityResource::ReadyEntity(Entity* entity) {
    if (entity == nullptr || (entity->mFlags & 0x16) != 0x12) {
        return;
    }
    Entity* child = entity;
    Entity* root = entity;
    for (Entity* parent = entity; parent != nullptr; parent = parent->GetParent()) {
        child = root;
        root = parent;
    }
    root->mResource->_OnEntityReady(root, child);
}

// Reconstructed from eboot.elf at 0xFD940.
void EntityResource::_OnEntityReady(Entity* root, Entity* entity) {
    static_cast<void>(root);
    static_cast<void>(entity);
}

// Reconstructed from eboot.elf at 0xFD950.
void EntityResource::_EnterImmediately(Entity* entity) {
    entity->_EnterImmediately();
}

// Reconstructed from eboot.elf at 0xFD960.
void EntityResource::_PollImmediately(Entity* entity) {
    entity->_PollImmediately();
}

// Reconstructed from eboot.elf at 0xFD970.
void EntityResource::DestroyLayerEntity(Entity* entity) {
    if (entity != nullptr) {
        _DestroyLayerEntity(entity);
    }
}

// Reconstructed from eboot.elf at 0xFD990.
bool EntityResource::LoadResources() {
    return mEntity->_LoadResources(false);
}

// Reconstructed from eboot.elf at 0xFD9A0. The perf timers start unset.
EntityResource::EntityResource()
    : mEntity(nullptr),
      mInlineOwner(nullptr),
      mPollTimer(-1),
      mPostPollTimer(-1),
      mEnterExitTimer(-1),
      mDestroyTimer(-1),
      mHasStaleIds(false) {
    mLayers.resize(1);
}

// Reconstructed from eboot.elf at 0xFDB10. Each layer's inlined resources
// are released from the last, after their inlined flag is cleared.
EntityResource::~EntityResource() {
    ReleaseInlineResources(mLayers);
    DestroyEntity();
    mMissingComponents.clear();
}

// Reconstructed from eboot.elf at 0xFDF10.
void EntityResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("baseentity"));
    metaData.mTypeFlags[0] = false;
    metaData.mCategory = Symbol("Base Entities");
    metaData.mTypeOption = 1;
}

// Reconstructed from eboot.elf at 0xFE380. A source file has its stale ids
// fixed around the resource load, with the objects' entered state reset
// first. The binary inlines _EndLoadProgress.
bool EntityResource::Load(BinStream& stream, bool cached) {
    const bool loaded = _LoadEntity(stream, cached);
    const bool profiling = IsProfilingLoad();
    bool result = false;
    if (loaded) {
        if (!cached) {
            _FixupStaleIds(false);
            mEntity->ResetEntered();
        }
        if (mEntity->_LoadResources(false)) {
            if (!cached) {
                _FixupStaleIds(true);
            }
            result = _PostLoad(stream, cached);
        }
    }
    if (profiling) {
        _EndLoadProgress();
    }
    return result;
}

// Reconstructed from eboot.elf at 0xFE530.
bool EntityResource::IsProfilingLoad() const {
    if (!HasLoadProgressListeners()) {
        return false;
    }
    return gProfilingLoad == this;
}

// Reconstructed from eboot.elf at 0xFEC20. Revisions 12 to 18 start with
// the layer files and, from 16, the root data; older streams are read
// from the entity on, unless cached. Source files before revision 17 get
// the root's EditorCom (before 13) and InstanceCom when they lack them;
// the root's id is compared with the null id first. A cached file fails
// when a layer file is missing or not older than the resource.
bool EntityResource::_LoadEntity(BinStream& stream, bool cached) {
    const ResourcePath path = mPath;
    if (mPollTimer == -1) {
        const bool wantsTimers = _WantsPerfTimers();
        if (path.mPath != Symbol() && wantsTimers) {
            const char* const name = FileGetName(path.Str());
            FormatString poll("poll: %s");
            poll << name;
            mPollTimer = static_cast<long>(thePerfMgr.GetTimerIndex(Symbol(poll.Str())));
            FormatString postPoll("post_poll: %s");
            postPoll << name;
            mPostPollTimer = static_cast<long>(thePerfMgr.GetTimerIndex(Symbol(postPoll.Str())));
            FormatString enterExit("enter_exit: %s");
            enterExit << name;
            mEnterExitTimer = static_cast<long>(thePerfMgr.GetTimerIndex(Symbol(enterExit.Str())));
            FormatString destroy("destroy: %s");
            destroy << name;
            mDestroyTimer = static_cast<long>(thePerfMgr.GetTimerIndex(Symbol(destroy.Str())));
        }
    }
    if (mEntity != nullptr) {
        ReleaseInlineResources(mLayers);
        DestroyEntity();
        mMissingComponents.clear();
    }

    char dir[512] = {};
    gEntityLoadDir = FileGetPath(stream.Name(), dir);
    int rev = 0;
    stream.ReadEndian(&rev, sizeof(rev));
    mHasStaleIds = rev < kEntityResourceSaveRev;
    if (rev < 12 || rev > kEntityResourceSaveRev) {
        if (cached) {
            return false;
        }
        stream.Seek(-4, kSeekCur);
    } else {
        unsigned int numLayers;
        stream.ReadEndian(&numLayers, sizeof(numLayers));
        mLayers.resize(numLayers);
        for (unsigned int layer = 0; layer < numLayers; ++layer) {
            stream >> mLayers[layer].mPath.mPath;
        }
    }
    if (rev >= 16) {
        unsigned int size;
        stream.ReadEndian(&size, sizeof(size));
        mRootData.resize(size);
        if (size != 0) {
            stream.Read(mRootData.data(), size);
        }
    }

    PushFixupEntity fixup;
    if (!HasLoadProgressListeners() || path.mPath == Symbol()) {
        _ReadLoadStepCount(stream, rev, cached, false);
    } else {
        const ResourcePath profiled = sProfileLoadPath;
        const unsigned int steps = _ReadLoadStepCount(stream, rev, cached, profiled == path);
        if (profiled == path) {
            _BeginLoadProgress(cached, steps);
        }
    }
    if (!cached) {
        mEntity = Entity::_LoadUncached(stream, this);
        if (rev <= 11) {
            _LoadInlineResources(stream, 0, false);
        }
        GameObject* root = nullptr;
        if (!gResourcePrecacheMode && rev <= 16 && gFileArchiveMode == 0 && gNullObjectId.mId != 0) {
            root = mEntity->GetRoot();
        }
        if (root != nullptr) {
            if (rev < 13 && root->GetCom<EditorCom>() == nullptr) {
                InitObject(root);
            }
            if (root->GetCom<InstanceCom>() == nullptr) {
                root->CreateComponent(InstanceCom::sClassName, false);
            }
        }
        gEntityLoadDir = ".";
        return mEntity != nullptr;
    }

    mEntity = Entity::_LoadCached(stream, this);
    gEntityLoadDir = ".";
    if (mEntity == nullptr) {
        return false;
    }
    for (unsigned long layer = 1; layer < mLayers.size(); ++layer) {
        const ResourcePath& layerPath = mLayers[layer].mPath;
        if (layerPath.mPath == Symbol() || gFileArchiveMode != 0) {
            continue;
        }
        const FileStat stamp = FileTimestamp(layerPath.Str());
        const bool missing = stamp.mSeconds == 0 && stamp.mFraction == 0;
        const bool stale = mFileTime.mSeconds == stamp.mSeconds ? mFileTime.mFraction <= stamp.mFraction
                                                                : mFileTime.mSeconds < stamp.mSeconds;
        if (missing || stale) {
            DestroyEntity();
            return false;
        }
    }
    for (unsigned long layer = 0; layer < mLayers.size(); ++layer) {
        if (layer == 0 || mLayers[layer].mPath.mPath != Symbol()) {
            if (!_LoadInlineResources(stream, layer, true)) {
                return false;
            }
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0xFF810.
void EntityResource::_BeginLoadProgress(bool cached, unsigned int steps) {
    gProfilingLoad = this;
    FormatString text("Loading %s (%s)");
    text << mPath.mPath << (cached ? "cached" : "uncached");
    LoadProgressBegin(kEntityLoadTask, text.Str(), steps, 0);
}

// Reconstructed from eboot.elf at 0xFFE80.
bool EntityResource::LayerExists(unsigned long layer) const {
    if (layer == 0) {
        return true;
    }
    return layer < mLayers.size() && mLayers[layer].mPath.mPath != Symbol();
}

// Reconstructed from eboot.elf at 0x1006D0.
void EntityResource::_EndLoadProgress() {
    gProfilingLoad = nullptr;
    LoadProgressEnd(kEntityLoadTask);
}

// Reconstructed from eboot.elf at 0x100760. A source file holds the main
// layer; a cached file holds every layer, counted for the load steps, with
// each layer's inlined resources after the entity, and the entity's
// resources are loaded afterwards unless they are. The root data is
// written byte by byte, as an empty copy while precaching a cached file.
void EntityResource::Save(BinStream& stream, bool cached) {
    char dir[512] = {};
    gEntityLoadDir = FileGetPath(stream.Name(), dir);
    int rev = kEntityResourceSaveRev;
    stream.WriteEndian(&rev, sizeof(rev));
    unsigned int numLayers = static_cast<unsigned int>(mLayers.size());
    stream.WriteEndian(&numLayers, sizeof(numLayers));
    for (unsigned long layer = 0; layer < mLayers.size(); ++layer) {
        stream << mLayers[layer].mPath.mPath;
    }
    const eastl::vector<unsigned char> empty;
    const eastl::vector<unsigned char>& rootData =
        gResourcePrecacheMode && cached ? empty : mRootData;
    unsigned int size = static_cast<unsigned int>(rootData.size());
    stream.WriteEndian(&size, sizeof(size));
    for (const unsigned char& value : rootData) {
        unsigned char byte = value;
        stream.Write(&byte, 1);
    }
    if (!cached) {
        // The main layer, the constant 0 at 0x124EEA0.
        unsigned int steps = mEntity->mLayers[0].GetLoadStepCount(0, mEntity);
        stream.WriteEndian(&steps, sizeof(steps));
        mEntity->_SaveUncached(stream);
        gEntityLoadDir = ".";
        return;
    }
    unsigned int steps = 0;
    for (unsigned long layer = 0; layer < mLayers.size(); ++layer) {
        if (layer == 0 || mLayers[layer].mPath.mPath != Symbol()) {
            steps += mEntity->mLayers[layer].GetLoadStepCount(layer, mEntity);
        }
    }
    stream.WriteEndian(&steps, sizeof(steps));
    mEntity->_SaveCached(stream);
    for (unsigned long layer = 0; layer < mLayers.size(); ++layer) {
        if (layer == 0 || mLayers[layer].mPath.mPath != Symbol()) {
            _SaveInlineResources(stream, layer, true);
        }
    }
    gEntityLoadDir = ".";
    if ((mEntity->mFlags & 0x20) == 0) {
        mEntity->_LoadResources(false);
    }
}

// Reconstructed from eboot.elf at 0x101A20. Only the classes the object
// lacks, or holds as null, are created.
bool EntityResource::CreateRequiredComponents(GameObject* object, const ComMetaData& metaData) {
    for (const Symbol& required : metaData.mRequiredComponents) {
        bool found = false;
        for (const GameObject::ComIndex& entry : object->mComs) {
            if (entry.mId == required) {
                found = entry.mCom != nullptr;
                break;
            }
        }
        if (!found && object->CreateComponent(required, false) == nullptr) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x101AC0. The empty symbol matches a
// component registered without a class.
void EntityResource::DestroyDependentComponents(GameObject* object, const ComMetaData& metaData) {
    for (const Symbol& dependent : metaData.mDependentComponents) {
        for (const GameObject::ComIndex& entry : object->mComs) {
            const bool matches = dependent == Symbol()
                ? entry.mId == dependent
                : entry.mId == dependent || entry.mBaseId == dependent;
            if (matches) {
                if (entry.mCom != nullptr) {
                    object->DestroyComponent(dependent, true);
                }
                break;
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x101B80.
bool EntityResource::IsAllowedComponent(const ComMetaData& metaData) const {
    for (const Symbol& allowed : metaData.mAllowedResources) {
        if (IsA(allowed)) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x101BD0.
Symbol EntityResource::GetInstanceComId() const {
    return gEntityInstanceComId;
}

// Reconstructed from eboot.elf at 0x101BE0.
void EntityResource::AddMissingComponent(Symbol className) {
    for (const Symbol& missing : mMissingComponents) {
        if (missing == className) {
            return;
        }
    }
    mMissingComponents.push_back(className);
}

// Reconstructed from eboot.elf at 0x101CF0. The binary returns the empty
// symbol's string past the last layer.
ResourcePath EntityResource::GetLayerPath(unsigned long layer) const {
    if (layer < mLayers.size()) {
        return mLayers[layer].mPath;
    }
    return ResourcePath();
}

// Reconstructed from eboot.elf at 0x102620.
ResourceMetaData* EntityResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x102630.
Symbol EntityResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x1026D0.
bool EntityResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr;
         metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x102700.
bool EntityResource::Fail() const {
    return mEntity == nullptr;
}
