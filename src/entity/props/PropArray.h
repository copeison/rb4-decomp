#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "utl/text/Symbol.h"

// The untyped base of every PropArray<T>, a property-visible array whose
// element operations go through its vtable (the map's PropArrayBase in
// entity/PropUtl.o). Each PropArray<T> has its own vtable, such as
// 0x18E6928 for PropArray<GameObject::ComIndex>; the element operations are
// not reconstructed, so they are only declared. Names of the slots and the
// fields are not in the reference map.
class PropArrayBase {
public:
    virtual ~PropArrayBase();  // slots 0-1
    // Slot 2: default-constructs `count` elements at `data` and returns it.
    virtual void* Construct(unsigned long count, void* data) const;
    // Slot 3: the element alignment (8 for GameObject::ComIndex).
    virtual unsigned long Alignment() const;
    // Slot 4: destroys `count` elements; empty for plain elements.
    virtual void Destruct(unsigned long count, void* data) const;
    // Slot 5: copies `count` elements, or default-constructs them when
    // `source` is null.
    virtual void Copy(unsigned long count, void* data, const void* source) const;
    // Slot 6: moves `count` elements, handling overlap.
    virtual void Move(unsigned long count, void* data, void* source) const;
    // Slot 7: empty in every vtable seen; its purpose is not identified.
    virtual void Reserved7() const;

    // Grows the storage to at least `count` elements, moving the elements
    // across; the old storage is freed unless it is static. Inlined into its
    // users, for example Entity::CreateObject at 0xF0A10.
    void Reserve(unsigned int count) {
        if (mCapacity >= count) {
            return;
        }
        void* const old = mData;
        mData = MemAlloc(static_cast<unsigned long>(count) * mElemSize, "PropArrayBase", 0);
        if (mSize != 0) {
            Move(mSize, mData, old);
        }
        if (!mStatic) {
            MemFree(old);
        }
        mStatic = false;
        mCapacity = count;
    }

    // Resizes the array, constructing or destroying elements through the
    // vtable. Not reconstructed.
    void Resize(unsigned int size);  // 0xA660
    // Inserts the element at the index, growing the array. The map's
    // signature is not known. Not reconstructed.
    void Insert(unsigned int index, const void* element);  // 0x2ABE0

    void* mData;
    unsigned int mSize;
    unsigned int mCapacity;
    unsigned int mElemSize;
    // Set while mData points at storage the array does not own.
    unsigned int mStatic;
    // The element type's symbol.
    Symbol mType;
};

static_assert(offsetof(PropArrayBase, mData) == 8);
static_assert(offsetof(PropArrayBase, mSize) == 16);
static_assert(offsetof(PropArrayBase, mElemSize) == 24);
static_assert(offsetof(PropArrayBase, mType) == 32);
static_assert(sizeof(PropArrayBase) == 40);

// A typed view of a PropArrayBase. Only the element access is declared.
template <typename T>
class PropArray : public PropArrayBase {
public:
    T* data() const {
        return static_cast<T*>(mData);
    }
    unsigned int size() const {
        return mSize;
    }
    T& operator[](unsigned long index) const {
        return data()[index];
    }
    T* begin() const {
        return data();
    }
    T* end() const {
        return data() + mSize;
    }
};
