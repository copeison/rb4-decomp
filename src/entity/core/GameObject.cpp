#include "entity/core/GameObject.h"

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "os/files/File.h"
#include "os/memory/MemMgr.h"
#include "utl/text/MakeString.h"

// Reconstructed from eboot.elf at 0x115F60. Every component is asked, even
// after one is not ready.
bool GameObject::AreResourcesReady() {
    bool ready = true;
    for (const ComIndex& index : mComs) {
        ready = index.mCom->AreResourcesReady() && ready;
    }
    return ready;
}

// Reconstructed from eboot.elf at 0x115FC0. The object is named with the
// serial in the top 16 bits of its id.
const char* GameObject::MakeErrorName() const {
    const char* name;
    {
        FormatString format("%s (%u)");
        format << mName << (mId.mId >> 16);
        name = format.Str();
    }
    const char* const entity = mEntity->MakeErrorName();
    FormatString format("%s object in %s");
    format << name << entity;
    return format.Str();
}

// Reconstructed from eboot.elf at 0x1160A0.
void GameObject::_EditEnterComponents() {
    mEntered = true;
    const int count = static_cast<int>(mComs.mSize);
    for (int index = 0; index < count; ++index) {
        Component* const component = mComs[index].mCom;
        component->mEntered = true;
        component->_EditEnter();
    }
}

// Reconstructed from eboot.elf at 0x1160F0.
void GameObject::SetName(Symbol name) {
    mName = name;
    mEntity->mResource->_OnObjectRenamed(this);
}

// Reconstructed from eboot.elf at 0x116110.
const char* GameObject::GetSafeName() const {
    const unsigned int serial = mId.mId >> 16;
    FormatString format("%s (%u)");
    format << mName << serial;
    return format.Str();
}

// Reconstructed from eboot.elf at 0x1162F0. The component table is copied
// in its sorted order through a temporary array, then the poll order is
// rebuilt for it.
void GameObject::_SortComponents() {
    if (!mPollOrderDirty) {
        return;
    }
    PropArray<unsigned int> order;
    PropArray<ComIndex> sorted;
    const unsigned int count = mComs.mSize;
    unsigned int temp;
    MemPushTemp(temp, true, true);
    if (count != 0) {
        order.Reserve(count);
    }
    sorted.Resize(count);
    MemPopTemp(temp);
    _BuildComponentOrder(&order, false);
    for (unsigned int index = 0; index < count; ++index) {
        sorted[index] = mComs[order[index]];
    }
    mComs._Copy(sorted);
    mPollOrder.Reserve(mComs.mSize);
    _BuildComponentOrder(&mPollOrder, true);
    mAllComsFlagged = true;
    for (const ComIndex& index : mComs) {
        if (!index.mCom->_GetMetaData().mLightweight) {
            mAllComsFlagged = false;
            break;
        }
    }
    mPollOrderDirty = false;
}

// Reconstructed from eboot.elf at 0x116AE0. An object holds one component
// per interface and, unless the class allows several, one per class; asking
// for one it holds returns that one. A class may be refused by the entity
// resource, by the archive mode (editor and debug classes) or for being
// root-only. Unless `deferSort` is set, the required classes are created
// first, the resource and the component are told, and the class's default
// components are created after it. The failure reports are compiled out but
// still format the object's name and read the class ids.
Component* GameObject::_CreateComponent(Symbol className, bool deferSort) {
    Entity* const entity = mEntity;
    ComMetaData* const metaData = ComMetaData::GetMetaData(className, false);
    if (metaData == nullptr) {
        MakeErrorName();
        EntityResource* const resource = entity->mResource;
        if (resource != nullptr) {
            resource->AddMissingComponent(className);
        }
        return nullptr;
    }
    if (!metaData->mCreatable) {
        return nullptr;
    }

    if (metaData->mInterface != Symbol() && mComs.mSize != 0) {
        for (const ComIndex& index : mComs) {
            if (index.mBaseId != metaData->mInterface) {
                continue;
            }
            Component* const existing = index.mCom;
            if (existing != nullptr) {
                if (existing->GetId() != className) {
                    static_cast<void>(GetSafeName());
                    static_cast<void>(existing->GetId());
                    return nullptr;
                }
                MakeErrorName();
                return existing;
            }
            break;
        }
    }
    if (!metaData->mAllowMultiple && mComs.mSize != 0) {
        for (const ComIndex& index : mComs) {
            if (index.mId == metaData->mClassName) {
                if (index.mCom != nullptr) {
                    MakeErrorName();
                    return index.mCom;
                }
                break;
            }
        }
    }

    if (gFileArchiveMode != 0 &&
        static_cast<unsigned int>(metaData->mCategory - ComMetaData::kCategoryEditor) < 2) {
        return nullptr;
    }
    EntityResource* const resource = entity->mResource;
    if (resource != nullptr && !resource->IsAllowedComponent(*metaData)) {
        static_cast<void>(resource->GetId());
        return nullptr;
    }
    if (metaData->mRootOnly && entity->GetRoot() != this && !Component::sRegressionTesting) {
        return nullptr;
    }

    if (!deferSort && resource != nullptr) {
        if (!resource->CreateRequiredComponents(this, *metaData)) {
            return nullptr;
        }
        if (metaData->mInterface != Symbol() && mComs.mSize != 0) {
            for (const ComIndex& index : mComs) {
                if (index.mBaseId != metaData->mInterface) {
                    continue;
                }
                Component* const existing = index.mCom;
                if (existing != nullptr) {
                    if (existing->GetId() != className) {
                        static_cast<void>(GetSafeName());
                        static_cast<void>(existing->GetId());
                        return nullptr;
                    }
                    return existing;
                }
                break;
            }
        }
        if (!metaData->mAllowMultiple && mComs.mSize != 0) {
            Component* const existing = FindCom(metaData->mClassName);
            if (existing != nullptr) {
                MakeErrorName();
                return existing;
            }
        }
    }

    const auto factory = Component::sFactory.find(metaData->mId);
    Component* const component =
        factory != Component::sFactory.end() ? factory->second() : nullptr;
    component->mObject = this;
    mComsAdded = true;
    ComIndex index;
    index.mCom = component;
    index.mId = metaData->mId;
    index.mBaseId = metaData->mInterface;
    mComs._Insert(mComs.mSize, &index);
    if (!deferSort) {
        entity->mResource->_OnComponentCreated(component);
        component->_PostCreate();
        for (const ComMetaData::DefaultComponent& dependency : metaData->mDefaultComponentIds) {
            bool held = false;
            for (const ComIndex& entry : mComs) {
                if (entry.mId == dependency.mId) {
                    held = entry.mCom != nullptr;
                    break;
                }
            }
            if (held) {
                continue;
            }
            if (dependency.mInterface != Symbol()) {
                for (const ComIndex& entry : mComs) {
                    if (entry.mBaseId == dependency.mInterface) {
                        held = entry.mCom != nullptr;
                        break;
                    }
                }
            }
            if (!held) {
                _CreateComponent(dependency.mId, false);
            }
        }
    }
    return component;
}

// Reconstructed from eboot.elf at 0x117160.
void GameObject::_UpdatePollOrder() {
    mPollOrder.Reserve(mComs.mSize);
    _BuildComponentOrder(&mPollOrder, true);
}

// Reconstructed from eboot.elf at 0x1174D0.
void GameObject::_PreDestroy(DestroyType type) {
    const int count = static_cast<int>(mComs.mSize);
    for (int index = 0; index < count; ++index) {
        mComs[index].mCom->_PreDestroy(type);
    }
}

// Reconstructed from eboot.elf at 0x117700. A component table that changed
// is re-sorted now unless the caller defers it, and the entity is told.
Component* GameObject::CreateComponent(Symbol className, bool deferSort) {
    Component* const component = _CreateComponent(className, deferSort);
    if (mComsAdded) {
        mPollOrderDirty = true;
        mAllComsFlagged = false;
        if (!deferSort) {
            _SortComponents();
        }
        mEntity->mFlags |= 0x400;
    }
    return component;
}

// Reconstructed from eboot.elf at 0x117760. The empty symbol matches a
// component registered without a class.
void GameObject::DestroyComponent(Symbol className, bool clearReferences) {
    Component* const component = FindCom(className);
    if (component == nullptr) {
        return;
    }
    Entity* const entity = mEntity;
    if (component->mEntered) {
        const unsigned int flags = entity->mFlags;
        const unsigned int mode = (flags & 0x10) != 0 ? (flags >> 1) & 3 : 0;
        if (mode == 2) {
            component->mEntered = false;
            component->_EditExit(kDestroyComponent);
        } else if (mode == 1) {
            component->mEntered = false;
            component->_Exit(kDestroyComponent);
        }
    }
    entity->mResource->DestroyDependentComponents(this, component->_GetMetaData());
    _DestroyComponent(className, kDestroyComponent, true, clearReferences);
}

// Reconstructed from eboot.elf at 0x117890. The entry leaves the table
// before the component is told; the entity then drops its references to the
// object's component of the class.
void GameObject::_DestroyComponent(
    Symbol className,
    DestroyType type,
    bool resort,
    bool clearReferences) {
    const long count = static_cast<int>(mComs.mSize);
    for (long index = 0; index < count; ++index) {
        Component* const component = mComs[index].mCom;
        if (component->GetId() != className && component->GetInterfaceId() != className) {
            continue;
        }
        const Symbol id = component->GetId();
        Entity* const entity = mEntity;
        mComs.Destruct(1, mComs.ElementAt(index));
        const unsigned int size = mComs.mSize;
        if (static_cast<unsigned int>(index) < size - 1) {
            mComs.Move(size - 1 - index, mComs.ElementAt(index), mComs.ElementAt(index + 1));
        }
        mComs.mSize = size - 1;
        component->_PreDestroy(type);
        component->Destroy();
        if (resort) {
            mPollOrderDirty = true;
            mAllComsFlagged = false;
            _SortComponents();
        }
        mComsAdded = true;
        if (clearReferences) {
            GameObjectId invalid;
            invalid.mId = static_cast<unsigned int>(-1);
            entity->_ReplaceObject(mId, invalid, id);
        }
        return;
    }
}

// Reconstructed from eboot.elf at 0x1179F0.
void GameObject::_ExitComponents(DestroyType type) {
    const int count = static_cast<int>(mComs.mSize);
    for (int index = 0; index < count; ++index) {
        Component* const component = mComs[index].mCom;
        component->mEntered = false;
        component->_Exit(type);
    }
    mEntered = false;
}

// Reconstructed from eboot.elf at 0x117A40.
void GameObject::_EditExitComponents(DestroyType type) {
    const int count = static_cast<int>(mComs.mSize);
    for (int index = 0; index < count; ++index) {
        Component* const component = mComs[index].mCom;
        component->mEntered = false;
        component->_EditExit(type);
    }
    mEntered = false;
}
