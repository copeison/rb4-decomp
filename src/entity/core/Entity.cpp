#include "entity/core/Entity.h"

#include "entity/core/EntityResource.h"
#include "utl/text/MakeString.h"

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

// Reconstructed from eboot.elf at 0xF25B0.
bool Entity::LayerExists(unsigned long layer) const {
    return mResource->LayerExists(layer);
}

// Reconstructed from eboot.elf at 0xF3050.
ResourcePath Entity::GetLayerPath(unsigned long layer) const {
    return mResource->GetLayerPath(layer);
}
