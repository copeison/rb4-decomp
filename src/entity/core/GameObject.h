#pragma once

#include <cstddef>

class Component;

// An entity: a set of components looked up by their class ids. Only the
// component table that the renderer searches is modelled.
class GameObject {
public:
    // One entry of the component table. The map names the type; the field
    // names are not in the reference map.
    struct ComIndex {
        Component* mCom;
        unsigned long mId;  // The component class's sId.
        unsigned long mUnknown16;
    };

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

    // Field names are not in the reference map. The table is the map's
    // PropArray<GameObject::ComIndex>; only its storage and count are
    // modelled.
    unsigned char mUnknown0[56];
    ComIndex* mComs;
    unsigned int mNumComs;
};

static_assert(sizeof(GameObject::ComIndex) == 24);
static_assert(offsetof(GameObject, mComs) == 56);
static_assert(offsetof(GameObject, mNumComs) == 64);
