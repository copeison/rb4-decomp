#pragma once

#include <cstddef>

#include "entity/props/PropArray.h"
#include "utl/text/Symbol.h"

class Component;
class Entity;

// The id of an object within its entity: the serial in bits 16-31, the
// layer in bits 12-15 and the object's index in that layer in bits 0-11.
// Only the packing that the reconstructed code decodes is modelled; the
// accessor names are not in the reference map.
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
// symbols (entity/GameObject.o, 0x115B90-0x117A90). The map has most of
// the object API on ObjPtr (ObjPtr::SetName, ObjPtr::CreateComponent); this
// build calls it on the object's own address, so it is declared here.
// Entity::_CreateAndInsertNewGameObject (0xF0AD0) allocates and builds the
// 128-byte object inline.
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
    // "<name> (<index>)" and the entity's MakeErrorName. The map's
    // ObjPtr::MakeErrorName() const.
    const char* MakeErrorName() const;  // 0x115FC0
    // Sets the name and tells the entity's resource. The map's
    // ObjPtr::SetName(Symbol).
    void SetName(Symbol name);  // 0x1160F0
    // Creates the component of the named class, or returns the existing
    // one, and re-sorts the poll order unless `deferSort` is set. The map's
    // ObjPtr::CreateComponent(Symbol, bool) is split in this build into
    // this wrapper and _CreateComponent. The renderer passes the class's
    // second symbol (sClassName).
    Component* CreateComponent(Symbol className, bool deferSort);  // 0x117700
    // The body of CreateComponent: finds the class's ComMetaData, checks
    // the entity resource allows it, creates it with its required
    // components and adds it to the table. The map's
    // ObjPtr::CreateComponent(Symbol, bool). Name not in the reference map.
    Component* _CreateComponent(Symbol className, bool deferSort);  // 0x116AE0
    // Destroys the component of the class. The map's
    // ObjPtr::DestroyComponent(Symbol); this build adds a flag, which
    // EntityResource::DestroyDependentComponents sets. Not reconstructed.
    void DestroyComponent(Symbol className, bool unknown);  // 0x117760
    // Rebuilds the poll order of a component table changed since the last
    // sort. Name not in the reference map; the map's
    // PollComponentBefore(Symbol, Symbol) suggests the ordering, so the
    // evidence is weak.
    void _SortComponents();  // 0x1162F0

    // The component with the class or base-class symbol, or null; the empty
    // symbol matches a component registered without a class. Inlined into
    // GetDataCom at 0x23C2E0. Name not in the reference map.
    Component* FindCom(Symbol name) const {
        for (const ComIndex& index : mComs) {
            if (name == Symbol() ? index.mId == name : (index.mId == name || index.mBaseId == name)) {
                return index.mCom;
            }
        }
        return nullptr;
    }

    // The component of class T, or null. Inlined by every user, for example
    // RndCameraContext::_CalcWorldXfms at 0x3D9E0E. Name not in the
    // reference map.
    template <typename T>
    T* GetCom() const {
        for (const ComIndex& index : mComs) {
            if (index.mId == T::sId) {
                return reinterpret_cast<T*>(index.mCom);
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
        for (const ComIndex& index : mComs) {
            if (index.mBaseId == T::sClassName) {
                return reinterpret_cast<T*>(index.mCom);
            }
        }
        return nullptr;
    }

    // The component of class T, which must exist: the table is searched
    // without a bound. Inlined into RndDefaults::GetLightingShadowOffset at
    // 0x6BFF19. Name not in the reference map.
    template <typename T>
    T* GetExistingCom() const {
        const ComIndex* index = mComs.data();
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
        const ComIndex* index = mComs.data();
        while (index->mBaseId != T::sClassName) {
            ++index;
        }
        return reinterpret_cast<T*>(index->mCom);
    }

    // Field names are not in the reference map.
    Entity* mEntity;
    // The components' poll order as indices into mComs, rebuilt by
    // _SortComponents.
    PropArray<unsigned int> mPollOrder;
    // The component table, the map's PropArray<GameObject::ComIndex>.
    PropArray<ComIndex> mComs;
    GameObjectId mId;
    unsigned char mPadding[4];  // Never read or written.
    Symbol mName;
    // Set for an object built from an imprint (0x106850, 0x1171F0).
    // Inferred from the writers only, so the name is weakly supported.
    bool mImprinted;
    // Set by the allocator; Entity's poll (0xF5580) visits only objects
    // with it set.
    bool mActive;
    // Set while the object's components are entered (0x1160A0) and
    // cleared when they exit (0x116890).
    bool mEntered;
    // Set when the component table changed and the poll order must be
    // rebuilt; _SortComponents clears it.
    bool mPollOrderDirty;
    // Set by _CreateComponent when it adds a component.
    bool mComsAdded;
    unsigned char mPadding2[3];  // Never read or written.
    // Cleared by the allocator; its readers are not identified.
    void* mReserved;
    // Set by _SortComponents when every component's metadata has the flag
    // at +82 set; cleared when a component is added. The flag's meaning is
    // not identified.
    bool mAllComsFlagged;
};

static_assert(sizeof(GameObject::ComIndex) == 24);
static_assert(offsetof(GameObject::ComIndex, mBaseId) == 16);
static_assert(offsetof(GameObject, mPollOrder) == 8);
static_assert(offsetof(GameObject, mComs) == 48);
static_assert(offsetof(GameObject, mId) == 88);
static_assert(offsetof(GameObject, mName) == 96);
static_assert(offsetof(GameObject, mActive) == 105);
static_assert(offsetof(GameObject, mComsAdded) == 108);
static_assert(offsetof(GameObject, mAllComsFlagged) == 120);
static_assert(sizeof(GameObject) == 128);
