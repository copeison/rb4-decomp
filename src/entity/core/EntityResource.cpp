#include "entity/core/EntityResource.h"

#include "entity/core/EditorCom.h"
#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/core/InstanceCom.h"


// The class's metadata at 0x19E3008.
ResourceMetaData EntityResource::sMetaData;

// The class symbol of the component that instances an entity resource,
// whose registration at 0x118E60 describes it as "Used to load an entity
// resource". Its class name is not recovered. Name not in the reference
// map.
extern Symbol gEntityInstanceComId;  // 0x19E3280

// The parts of a component class's ComMetaData that EntityResource reads.
// The type is the map's; ComMetaData is not reconstructed, so its members
// are reached through these offsets. Names not in the reference map.
namespace {

// The required component classes, a vector of class symbols at +0x80.
const eastl::vector<Symbol>& RequiredComponents(const ComMetaData& metaData) {
    return *reinterpret_cast<const eastl::vector<Symbol>*>(
        reinterpret_cast<const unsigned char*>(&metaData) + 0x80);
}

// The classes that depend on the class, a vector of class symbols at
// +0x140.
const eastl::vector<Symbol>& DependentComponents(const ComMetaData& metaData) {
    return *reinterpret_cast<const eastl::vector<Symbol>*>(
        reinterpret_cast<const unsigned char*>(&metaData) + 0x140);
}

// The entity resource classes the component class may be created in, a
// vector of class symbols at +0x30.
const eastl::vector<Symbol>& AllowedResources(const ComMetaData& metaData) {
    return *reinterpret_cast<const eastl::vector<Symbol>*>(
        reinterpret_cast<const unsigned char*>(&metaData) + 0x30);
}

}  // namespace

// Reconstructed from eboot.elf at 0xFD610. The binary forwards the
// arguments to Entity::_LoadRoot.
void EntityResource::_LoadRoot(
    BinStream& stream,
    Entity* entity,
    eastl::vector<unsigned char>* rootData,
    eastl::vector<ResourcePath>* paths) {
    Entity::_LoadRoot(stream, entity, rootData, paths);
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
            EditorCom* const editor = reinterpret_cast<EditorCom*>(entry.mCom);
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
    const unsigned long numLayers = mLayers.size();
    for (unsigned long layer = 0; layer < numLayers; ++layer) {
        eastl::vector<Resource*>& resources = mLayers[layer].mInlineResources;
        for (unsigned long index = resources.size(); index-- != 0;) {
            Resource* const resource = resources[index];
            resources.erase(resources.begin() + index);
            resource->mInlined = false;
            resource->ReleaseRef();
        }
    }
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

// Reconstructed from eboot.elf at 0xFFE80.
bool EntityResource::LayerExists(unsigned long layer) const {
    if (layer == 0) {
        return true;
    }
    return layer < mLayers.size() && mLayers[layer].mPath.mPath != Symbol();
}

// Reconstructed from eboot.elf at 0x101A20. Only the classes the object
// lacks, or holds as null, are created.
bool EntityResource::CreateRequiredComponents(GameObject* object, const ComMetaData& metaData) {
    for (const Symbol& required : RequiredComponents(metaData)) {
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
    for (const Symbol& dependent : DependentComponents(metaData)) {
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
    for (const Symbol& allowed : AllowedResources(metaData)) {
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
