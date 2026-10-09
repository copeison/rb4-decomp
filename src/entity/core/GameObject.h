#pragma once

#include <cstddef>

#include "utl/text/Symbol.h"

class Component;
class Entity;

// The id of an object within its entity: the layer in bits 12-15 and the
// object's index in that layer in bits 0-11. Only the packing that the
// renderer decodes is modelled; the accessor names are not in the reference
// map.
class GameObjectId {
public:
    unsigned int Layer() const {
        return (mId >> 12) & 0xF;
    }
    unsigned int Index() const {
        return mId & 0xFFF;
    }

    unsigned int mId;  // Name not in the reference map.
};

static_assert(sizeof(GameObjectId) == 4);

// An entity object: a named set of components found by their class
// symbols. The map has the object API on ObjPtr (ObjPtr::SetName,
// ObjPtr::CreateComponent); this build calls it on the object's own address,
// so it is declared here. Only the members the renderer uses are modelled.
class GameObject {
public:
    // One entry of the component table. The map names the type; the field
    // names are not in the reference map.
    struct ComIndex {
        Component* mCom;
        // The component class's sId.
        Symbol mId;
        // The class symbol of the component's base class. Base-class lookups
        // such as GetBaseCom<RndLightCom> match it, as does
        // Entity::NextObject.
        Symbol mBaseId;
    };

    // A description for error messages, formatted "%s object in %s" from
    // the object's name and number and its entity.
    // The map's ObjPtr::MakeErrorName() const.
    const char* MakeErrorName() const;  // 0x115FC0
    // The map's ObjPtr::SetName(Symbol).
    void SetName(Symbol name);  // 0x1160F0
    // Creates the component of the named class and returns it. The map's
    // ObjPtr::CreateComponent(Symbol, bool); this build splits it into this
    // wrapper and the body at 0x116AE0. The renderer passes the class's
    // second symbol (sClassName).
    Component* CreateComponent(Symbol className, bool unknown);  // 0x117700

    // The component of class T, or null. Inlined by every user, for example
    // RndCameraContext::_CalcWorldXfms at 0x3D9E0E. Name not in the
    // reference map.
    template <typename T>
    T* GetCom() const {
        for (unsigned int i = 0; i < mNumComs; ++i) {
            if (mComs[i].mId == T::sId) {
                return reinterpret_cast<T*>(mComs[i].mCom);
            }
        }
        return nullptr;
    }

    // The component derived from base class T, or null. Inlined into
    // RndDefaults::_LoadLighting at 0x6BF0A0. Name not in the reference map.
    template <typename T>
    T* GetBaseCom() const {
        if (T::sClassName == Symbol()) {
            return nullptr;
        }
        for (unsigned int i = 0; i < mNumComs; ++i) {
            if (mComs[i].mBaseId == T::sClassName) {
                return reinterpret_cast<T*>(mComs[i].mCom);
            }
        }
        return nullptr;
    }

    // The component of class T, which must exist: the table is searched
    // without a bound. Inlined into RndDefaults::GetLightingShadowOffset at
    // 0x6BFF19. Name not in the reference map.
    template <typename T>
    T* GetExistingCom() const {
        const ComIndex* index = mComs;
        while (index->mId != T::sId) {
            ++index;
        }
        return reinterpret_cast<T*>(index->mCom);
    }

    // The component derived from base class T, which must exist. Inlined
    // into RndDefaults::_SyncEnabledLights at 0x6BFAA4. Name not in the
    // reference map.
    template <typename T>
    T* GetExistingBaseCom() const {
        const ComIndex* index = mComs;
        while (index->mBaseId != T::sClassName) {
            ++index;
        }
        return reinterpret_cast<T*>(index->mCom);
    }

    // Field names are not in the reference map. The table is the map's
    // PropArray<GameObject::ComIndex>; only its storage and count are
    // modelled.
    Entity* mEntity;
    unsigned char mUnknown8[48];
    ComIndex* mComs;
    unsigned int mNumComs;
    unsigned char mUnknown68[20];
    GameObjectId mId;
    unsigned char mUnknown92[4];
    Symbol mName;
};

static_assert(sizeof(GameObject::ComIndex) == 24);
static_assert(offsetof(GameObject::ComIndex, mBaseId) == 16);
static_assert(offsetof(GameObject, mComs) == 56);
static_assert(offsetof(GameObject, mNumComs) == 64);
static_assert(offsetof(GameObject, mId) == 88);
static_assert(offsetof(GameObject, mName) == 96);
