#include "entity/core/Entity.h"

#include <kernel.h>

#include "entity/core/Component.h"
#include "entity/core/EntityConstants.h"
#include "entity/core/EntityResource.h"
#include "entity/core/InstanceCom.h"
#include "os/files/File.h"
#include "os/memory/MemMgr.h"
#include "utl/data/DataUtl.h"
#include "utl/streams/BinStream.h"
#include "utl/text/MakeString.h"
#include "utl/threading/PollMgr.h"

namespace {

// The id of no object, at 0x19E2E38, which Entity.o's initializer (0xFD0B0)
// sets. Each object that uses it has its own copy. Name not in the
// reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFu};
// The id of the root object, at 0x19E2E3C, set by the same initializer.
// Name not in the reference map.
GameObjectId gRootObjectId = {0};

// The type the objects of the entities the thread is exiting or destroying
// exit and are destroyed with: the map's TLSValue<DestroyType>, which the
// initializer starts at kDestroyUnset; its descriptor and offset are at
// 0x19B02B8. Name not in the reference map.
thread_local DestroyType gDestroyType = kDestroyUnset;

// Sets the thread's destroy type for the entity's objects and returns the
// previous one: the entity's instance type, or the enclosing entities'
// when that is lower, and kDestroyRoot for a top-level entity. Inlined
// into Exit and _Destroy. Name not in the reference map.
DestroyType PushDestroyType(const Entity& entity) {
    const DestroyType previous = gDestroyType;
    if (entity.mParentObject != nullptr) {
        DestroyType type = (entity.mFlags & 0x800) != 0 ? kDestroyEditorInstance : kDestroyInstance;
        if (previous != kDestroyUnset && type > previous) {
            type = previous;
        }
        gDestroyType = type;
    } else if (previous == kDestroyUnset) {
        gDestroyType = kDestroyRoot;
    }
    return previous;
}

}  // namespace

// Reconstructed from eboot.elf at 0xEBB70. The flags are a 17-bit field;
// the constructor keeps the bits above it.
Entity::Entity(unsigned long numLayers)
    : mResource(nullptr),
      mPollWindowStart(0),
      mPollWindowEnd(0),
      mFlags(0x1C0),
      mParentObject(nullptr),
      mTag() {
    mLayers.Resize(numLayers);
}

// Reconstructed from eboot.elf at 0xEBCC0.
Entity::~Entity() {}

// Reconstructed from eboot.elf at 0xEE880. Components that a load adds to
// an object restart its pass with the object's poll order rebuilt. A
// failure ends the load; it is reported unless the entity is built in
// place and the archive mode is off.
bool Entity::_LoadResources(bool quiet) {
    static_cast<void>(mResource->IsProfilingLoad());
    const int numLayers = static_cast<int>(mLayers.mSize);
    if (numLayers > 0) {
        GameObject* const root = mLayers[0].mObjects[0];
        for (int layer = 0; layer < numLayers; ++layer) {
            const PropArray<GameObject*>& objects = mLayers[layer].mObjects;
            const int numObjects = static_cast<int>(objects.mSize);
            for (int index = 0; index < numObjects; ++index) {
                GameObject* const object = objects[index];
                if (object == nullptr) {
                    continue;
                }
                object->mComsAdded = false;
                unsigned int count = object->mComs.mSize;
                while (count != 0) {
                    bool restart = false;
                    for (unsigned int com = 0; com < count; ++com) {
                        Component* const component =
                            object->mComs[object->mPollOrder[com]].mCom;
                        mFlags = (mFlags & ~0x10000u) | (static_cast<unsigned int>(object == root) << 16);
                        const bool loaded = component->LoadResources(quiet);
                        const unsigned int flags = mFlags;
                        mFlags = flags & ~0x10000u;
                        if (!loaded) {
                            if ((flags & 1) == 0 || gFileArchiveMode != 0) {
                                component->MakeErrorName();
                            }
                            return false;
                        }
                        if (object->mComsAdded) {
                            restart = true;
                            break;
                        }
                    }
                    if (!restart) {
                        break;
                    }
                    object->_UpdatePollOrder();
                    object->mComsAdded = false;
                    count = object->mComs.mSize;
                }
                object->_SortComponents();
            }
        }
    }
    if (!quiet) {
        mFlags |= 0x400;
    }
    _UpdatePollOrder();
    if (mParentObject == nullptr || (mParentObject->mEntity->mFlags & 0x20) != 0) {
        for (int attempt = 0; mPollOrder.mSize != 0;) {
            bool ready = true;
            for (const GameObjectId& id : mPollOrder) {
                ready = GetObject(id)->AreResourcesReady() && ready;
            }
            if (ready) {
                break;
            }
            if (++attempt == 10) {
                MakeErrorName();
                break;
            }
        }
    }
    mFlags |= 0x20;
    return true;
}

// Reconstructed from eboot.elf at 0xEF180.
GameObject* Entity::BeginObject() const {
    return NextObject(nullptr, Symbol());
}

// Reconstructed from eboot.elf at 0xEF1A0. The search continues in the
// object's layer and then in the later layers. An object whose first
// matching component is null is skipped.
GameObject* Entity::NextObject(const GameObject* object, Symbol com) const {
    unsigned long layer = 0;
    int index = 0;
    if (object != nullptr) {
        layer = object->mId.Layer();
        index = static_cast<int>(object->mId.Index()) + 1;
    }
    for (; layer < mLayers.size(); ++layer, index = 0) {
        const PropArray<GameObject*>& objects = mLayers[layer].mObjects;
        for (; index < static_cast<int>(objects.size()); ++index) {
            GameObject* const candidate = objects[index];
            if (candidate == nullptr) {
                continue;
            }
            if (com == Symbol()) {
                return candidate;
            }
            for (const GameObject::ComIndex& entry : candidate->mComs) {
                if (entry.mId == com || entry.mBaseId == com) {
                    if (entry.mCom != nullptr) {
                        return candidate;
                    }
                    break;
                }
            }
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0xEFA00. The header is read only from
// revisions 12 to kEntityResourceRev; older streams start at the entity's
// own revision. Revisions 8 to 11 keep the layer files there, and revision 8
// on repeats the entity revision. Without root data the bytes are skipped.
void Entity::_LoadRoot(
    BinStream& stream,
    Entity* entity,
    eastl::vector<ResourcePath>* paths,
    eastl::vector<unsigned char>* rootData) {
    int rev;
    stream.ReadEndian(&rev, sizeof(rev));
    if (rev >= 12 && rev <= kEntityResourceRev) {
        unsigned int numPaths;
        stream.ReadEndian(&numPaths, sizeof(numPaths));
        paths->resize(numPaths);
        for (unsigned int index = 0; index < numPaths; ++index) {
            stream >> (*paths)[index].mPath;
        }
    } else {
        stream.Seek(stream.Tell() - 4, kSeekBegin);
    }
    if (rev >= 16) {
        unsigned int size;
        if (rootData != nullptr) {
            stream.ReadEndian(&size, sizeof(size));
            rootData->resize(size);
            if (size != 0) {
                stream.Read(rootData->data(), size);
            }
        } else {
            stream.ReadEndian(&size, sizeof(size));
            stream.Seek(stream.Tell() + size, kSeekBegin);
        }
    }
    if (rev >= 15) {
        int objectCount;
        stream.ReadEndian(&objectCount, sizeof(objectCount));
    }

    int entityRev;
    stream.ReadEndian(&entityRev, sizeof(entityRev));
    const int savedRev = gEntityRev;
    gEntityRev = entityRev;
    if (entityRev < 2) {
        static_cast<void>(stream.Name());
        gEntityRev = savedRev;
        return;
    }
    if (entityRev <= 4) {
        Symbol name;
        stream >> name;
    }
    unsigned int numLayers;
    if (gEntityRev > 8) {
        stream.ReadEndian(&numLayers, sizeof(numLayers));
    } else {
        int value = -1;
        stream.ReadEndian(&value, sizeof(value));
        if (gEntityRev >= 7) {
            stream.ReadEndian(&value, sizeof(value));
            numLayers = static_cast<unsigned int>(value);
        } else {
            numLayers = 1;
        }
    }
    entity->mLayers.Resize(numLayers);
    Layer* const layers = entity->mLayers.data();
    if (gEntityRev >= 8) {
        if (gEntityRev <= 11) {
            paths->resize(numLayers);
            for (unsigned int index = 0; index < numLayers; ++index) {
                stream >> (*paths)[index].mPath;
            }
        }
        stream.ReadEndian(&gEntityRev, sizeof(gEntityRev));
        int value;
        if (gEntityRev >= 21) {
            stream.ReadEndian(&value, sizeof(value));
        }
        stream.ReadEndian(&value, sizeof(value));
    }
    stream.ReadEndian(&layers[0].mNextSerial, sizeof(layers[0].mNextSerial));
    int hasRoot = 0;
    stream.ReadEndian(&hasRoot, sizeof(hasRoot));
    if (hasRoot != 0) {
        GameObjectId id;
        id.mId = 0xFFFFFFFFu;
        stream.ReadEndian(&id.mId, sizeof(id.mId));
        layers[0].mObjects.Resize(1);
        layers[0].mObjects[0] = new GameObject(entity, id);
        layers[0].mObjects[0]->Load(stream);
    }
    gEntityRev = savedRev;
}

// Reconstructed from eboot.elf at 0xF0780.
Entity* Entity::GetParent() const {
    return mParentObject != nullptr ? mParentObject->mEntity : nullptr;
}

// Reconstructed from eboot.elf at 0xF0A10.
GameObject* Entity::CreateObject(unsigned long layer, unsigned long numComs) {
    mFlags |= 0x400;
    const GameObjectId id = _CreateAndInsertNewGameObject(layer, nullptr);
    GameObject* const object = mLayers[layer].mObjects[id.Index()];
    if (numComs != 0) {
        object->mComs.Reserve(static_cast<unsigned int>(numComs));
    }
    mResource->InitObject(object);
    return object;
}

// Reconstructed from eboot.elf at 0xF0AD0. The id takes the layer's next
// serial and the first free index, appending an empty slot when the layer
// is full; a new object is allocated when none is given.
GameObjectId Entity::_CreateAndInsertNewGameObject(unsigned long layer, GameObject* object) {
    Layer& entry = mLayers[layer];
    const unsigned int serial = entry.mNextSerial;
    entry.mNextSerial = static_cast<unsigned short>(serial + 1);
    GameObjectId id;
    id.mId = (serial << 16) | ((layer & 0xF) << 12);
    unsigned int count = entry.mObjects.mSize;
    unsigned int index = 0;
    while (index < count && entry.mObjects[index] != nullptr) {
        index = (index + 1) & 0xFFF;
        id.mId = index | (id.mId & 0xFFFFF000u);
    }
    if (object == nullptr) {
        object = new GameObject(this, id);
        count = entry.mObjects.mSize;
    }
    if (index == count) {
        GameObject* const empty = nullptr;
        entry.mObjects._Insert(index, &empty);
    }
    entry.mObjects[index] = object;
    return id;
}

// Reconstructed from eboot.elf at 0xF0DD0. An inlined entity resource also
// names the resource it is inlined in.
const char* Entity::MakeErrorName() const {
    Resource* const resource = mResource;
    if (resource == nullptr) {
        return "Unowned entity";
    }
    const EntityResource* owner = nullptr;
    if (resource->mInlined && resource->IsA(EntityResource::Id())) {
        owner = static_cast<const EntityResource*>(resource)->mInlineOwner;
    }
    const Symbol id = resource->GetId();
    if (owner != nullptr) {
        FormatString format("%s (%s : %s)");
        format << id << resource->mPath.mPath << owner->mPath.mPath;
        return format.Str();
    }
    FormatString format("%s (%s)");
    format << id << resource->mPath.mPath;
    return format.Str();
}

// Reconstructed from eboot.elf at 0xF0FF0. The failure report that `fail`
// asks for is compiled out; only the entity's description is made.
GameObject* Entity::TryGetObject(Symbol name, bool fail) const {
    for (int layer = 0; layer < static_cast<int>(mLayers.size()); ++layer) {
        const PropArray<GameObject*>& objects = mLayers[layer].mObjects;
        for (int index = 0; index < static_cast<int>(objects.size()); ++index) {
            GameObject* const object = objects[index];
            if (object != nullptr && object->mName == name) {
                return object;
            }
        }
    }
    if (fail) {
        static_cast<void>(MakeErrorName());
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0xF11C0. An id is stale when its serial
// is past the layer's or below its index, or names another object.
GameObject* Entity::SafeGetObject(GameObjectId id, bool fail) const {
    const unsigned int serial = id.mId >> 16;
    if (id.mId != gNullObjectId.mId && id.Layer() < mLayers.mSize) {
        const Layer& layer = mLayers[id.Layer()];
        if (serial < layer.mNextSerial && id.Index() <= serial && id.Index() < layer.mObjects.mSize) {
            GameObject* const object = layer.mObjects[id.Index()];
            if (object != nullptr) {
                if (object->mId.mId == id.mId) {
                    return object;
                }
                if (fail) {
                    MakeErrorName();
                }
                return nullptr;
            }
        }
    }
    if (fail) {
        MakeErrorName();
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0xF25B0.
bool Entity::LayerExists(unsigned long layer) const {
    return mResource->LayerExists(layer);
}

// Reconstructed from eboot.elf at 0xF3050.
ResourcePath Entity::GetLayerPath(unsigned long layer) const {
    return mResource->GetLayerPath(layer);
}

// Reconstructed from eboot.elf at 0xF4D10. The default entity is this one
// while the objects enter. In the game mode the entity's poll weight is a
// tenth of its component count.
void Entity::Enter(unsigned int flags) {
    mFlags = (mFlags & ~0xEu) + 2 * (flags & 1) + 10;
    _UpdatePollOrder();
    Entity* const defaultEntity = gDataThread.mDefaultEntity;
    DataSetDefaultEntity(this);
    if ((flags & 2) == 0) {
        const unsigned int mode = (mFlags >> 1) & 3;
        if (mode == 1) {
            for (unsigned long index = 0; index < mPollOrder.mSize; ++index) {
                GetObject(mPollOrder[index])->_EnterComponents();
            }
        } else if (mode == 2) {
            for (unsigned long index = 0; index < mPollOrder.mSize; ++index) {
                GetObject(mPollOrder[index])->_EditEnterComponents();
            }
        }
    }
    gDataThread.mThisObject = nullptr;
    DataSetDefaultEntity(defaultEntity);
    const unsigned int mode = mFlags & 6;
    mFlags = (mFlags & ~0x8000u) | 0x10;
    if (mode == 2) {
        bool drivesParent = false;
        if (mParentObject != nullptr) {
            drivesParent = GetRoot()->GetExistingCom<InstanceCom>()->mDrivesParent;
        }
        mFlags |= static_cast<unsigned int>(drivesParent) << 15;
        SetPollWeight(static_cast<unsigned int>(GetNumComs(false) / 10), false);
    }
    mFlags &= ~8u;
}

// Reconstructed from eboot.elf at 0xF4FF0. Each component is marked
// entered before its _Enter.
bool GameObject::_EnterComponents() {
    mEntered = true;
    gDataThread.mThisObject = this;
    const int count = static_cast<int>(mComs.mSize);
    for (int index = 0; index < count; ++index) {
        Component* const component = mComs[index].mCom;
        component->mEntered = true;
        component->_Enter();
    }
    _SortComponents();
    return _UpdateAllComsFlagged();
}

// Reconstructed from eboot.elf at 0xF50D0. The objects exit in reverse poll
// order with the thread's destroy type; the binary assumes the poll order
// is not empty. Inactive objects that are still entered exit after them.
void Entity::Exit(unsigned int flags) {
    static_cast<void>(scePthreadSelf());
    const DestroyType previous = PushDestroyType(*this);
    _UpdatePollOrder();
    if ((mFlags & 6) == 2) {
        _RemovePostPoll();
        if ((mFlags & 0x1000) != 0) {
            _Reset();
            mFlags &= ~0x1000u;
        }
    }
    if ((flags & 1) == 0) {
        for (unsigned int index = mPollOrder.mSize - 1;; --index) {
            GameObject* const object = GetObject(mPollOrder[index]);
            const unsigned int mode = (mFlags >> 1) & 3;
            if (mode == 2) {
                object->_EditExitComponents(gDestroyType);
            } else if (mode == 1) {
                object->_ExitComponents(gDestroyType);
            }
            if (index == 0) {
                break;
            }
        }
        for (unsigned long layer = 0; layer < mLayers.mSize; ++layer) {
            const PropArray<GameObject*>& objects = mLayers[layer].mObjects;
            const unsigned long count = objects.mSize;
            for (unsigned long index = 0; index != count; ++index) {
                GameObject* const object = objects[index];
                if (object == nullptr || object->mActive || !object->mEntered) {
                    continue;
                }
                const unsigned int mode = (mFlags >> 1) & 3;
                if (mode == 2) {
                    object->_EditExitComponents(gDestroyType);
                } else if (mode == 1) {
                    object->_ExitComponents(gDestroyType);
                }
            }
        }
    }
    mFlags &= ~0x16u;
    gDestroyType = previous;
}

// Reconstructed from eboot.elf at 0xF57C0. An entity that drives its parent
// polls only while every such ancestor may.
bool Entity::IsPollEnabled() const {
    const Entity* entity = this;
    while ((entity->mFlags & 0x2140) == 0x140) {
        if ((entity->mFlags & 0x8000) == 0) {
            return true;
        }
        entity = entity->mParentObject->mEntity;
    }
    return false;
}

// Reconstructed from eboot.elf at 0xF5960.
void Entity::ThreadPoll(const int& thread) {
    static_cast<void>(thread);
    mResource->PollEntity(this);
}

// Reconstructed from eboot.elf at 0xF5FC0. The default entity is this one
// while the objects poll.
void Entity::_Poll() {
    if ((mFlags & 0x100) == 0) {
        return;
    }
    Entity* const defaultEntity = gDataThread.mDefaultEntity;
    DataSetDefaultEntity(this);
    const unsigned int flags = mFlags;
    _UpdatePollOrder();
    const unsigned int mode = (mFlags >> 1) & 3;
    if (mode == 2) {
        const int count = static_cast<int>(mPollOrder.mSize);
        for (int index = 0; index < count; ++index) {
            GameObject* const object = GetObject(mPollOrder[index]);
            gDataThread.mThisObject = object;
            const int numComs = static_cast<int>(object->mComs.mSize);
            for (int com = 0; com < numComs; ++com) {
                object->mComs[com].mCom->_EditPoll();
            }
        }
    } else if (mode == 1) {
        for (unsigned int index = mPollWindowStart; index < mPollWindowEnd; ++index) {
            GetObject(mPollOrder[index])->_PollComponents();
        }
    }
    if ((flags & 0x4000) != 0) {
        mFlags &= ~0x4000u;
    }
    gDataThread.mThisObject = nullptr;
    DataSetDefaultEntity(defaultEntity);
}

// Reconstructed from eboot.elf at 0xF6290.
void GameObject::_PollComponents() {
    gDataThread.mThisObject = this;
    const int count = static_cast<int>(mComs.mSize);
    for (int index = 0; index < count; ++index) {
        mComs[index].mCom->_Poll();
    }
    if (mEarlyFreeDep != nullptr) {
        if ((mEntity->mFlags & 0x16) == 0x12) {
            mEntity->EarlyFreeToPoll(mEarlyFreeDep);
        }
        mEarlyFreeDep = nullptr;
    }
}

// Reconstructed from eboot.elf at 0xF65C0. Every object but the root is
// destroyed first, then the root, each with the thread's destroy type.
// The root's id is compared with the null id before the root is taken. An
// entity built in place is destructed, and freed unless flag 0x200 keeps
// its storage; the binary reads that flag after the destructor ran.
void Entity::_Destroy() {
    if ((mFlags & 6) != 0) {
        Exit(0);
    }
    const DestroyType previous = PushDestroyType(*this);
    RemoveAllSinks();
    GameObject* const root = gNullObjectId.mId != gRootObjectId.mId ? GetRoot() : nullptr;
    for (unsigned long layer = 0; layer < mLayers.mSize; ++layer) {
        const PropArray<GameObject*>& objects = mLayers[layer].mObjects;
        const unsigned long count = objects.mSize;
        const DestroyType type = gDestroyType;
        for (unsigned long index = 0; index != count; ++index) {
            GameObject* const object = objects[index];
            if (object == nullptr || object->mId.mId == gRootObjectId.mId) {
                continue;
            }
            const GameObjectId id = object->mId;
            mFlags |= 0x400;
            object->_PreDestroy(type);
            object->_Destroy();
            mLayers[id.Layer()].mObjects[id.Index()] = nullptr;
        }
    }
    if (root != nullptr) {
        const GameObjectId id = root->mId;
        mFlags |= 0x400;
        root->_PreDestroy(gDestroyType);
        root->_Destroy();
        mLayers[id.Layer()].mObjects[id.Index()] = nullptr;
    }
    if (gEntityThreadState.mPollMgr != nullptr) {
        gEntityThreadState.mPollMgr->DequeueForLoadResourcesIfNeeded(this);
    }
    if ((mFlags & 1) != 0) {
        const bool keepStorage = (mFlags & 0x200) != 0;
        this->~Entity();
        if (!keepStorage) {
            MemFree(this);
        }
    } else {
        delete this;
    }
    gDestroyType = previous;
}

// Reconstructed from eboot.elf at 0xF7550.
unsigned int Entity::Layer::GetLoadStepCount(unsigned long layer, const Entity* entity) const {
    return mObjects.mSize + static_cast<unsigned int>(entity->mResource->mLayers[layer].mInlineResources.size());
}

// Reconstructed from eboot.elf at 0xF7910.
void* Entity::GetSinkObject() {
    return this;
}

// Reconstructed from eboot.elf at 0xF7920.
void Entity::PostPoll() {
    if ((mFlags & 0x100) != 0) {
        _PostPoll();
    }
}

// Reconstructed from eboot.elf at 0xF7930.
void Entity::_OnAddPollDep() {
    if ((mFlags & 0x16) == 0x12 && mResource != nullptr) {
        mResource->ReadyEntity(this);
    }
}

// Reconstructed from eboot.elf at 0xF7960.
void Entity::_OnRemovePollDep() {
    if ((mFlags & 0x16) == 0x12 && mResource != nullptr) {
        mResource->ReadyEntity(this);
    }
}
