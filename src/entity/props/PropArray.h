#pragma once

#include <cstddef>
#include <new>

#include "os/memory/MemMgr.h"
#include "utl/text/Symbol.h"

class PropInfo;

// The untyped base of every PropArray<T>, a property-visible array whose
// element operations go through its vtable (the map's PropArrayBase in
// entity/PropUtl.o; the vtable is at 0x18DC3D0). Each PropArray<T> has its
// own vtable, such as 0x18E6938 for PropArray<GameObject::ComIndex>. The
// names of slots 2, 3, 5, 6 and 7 and of the fields are not in the
// reference map.
class PropArrayBase {
public:
    // Slots 0-1: 0xA640, 0xA650.
    virtual ~PropArrayBase() {}
    // Slot 2: default-constructs `count` elements at `data` and returns it.
    virtual void* Construct(unsigned long count, void* data) const = 0;
    // Slot 3: the element alignment (8 for GameObject::ComIndex).
    virtual unsigned long Alignment() const = 0;
    // Slot 4 at 0xA400: updates the element info of an array whose element
    // type comes from its PropInfo; empty here and for typed arrays.
    virtual void _FixupItemInfo(const PropInfo* info) {
        static_cast<void>(info);
    }
    // Slot 5: copies `count` elements, or value-initializes them when
    // `source` is null.
    virtual void Copy(unsigned long count, void* data, const void* source) const = 0;
    // Slot 6: moves `count` elements, handling overlap.
    virtual void Move(unsigned long count, void* data, void* source) const = 0;
    // Slot 7: destroys `count` elements; Resize calls it for the elements it
    // drops.
    virtual void Destruct(unsigned long count, void* data) const = 0;

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

    // The element at the index. Name not in the reference map.
    void* ElementAt(unsigned long index) const {
        return static_cast<unsigned char*>(mData) + index * mElemSize;
    }

    // Resizes the array, destroying the dropped elements and
    // value-initializing the new ones through the vtable.
    void Resize(unsigned long size);  // 0xA660
    // Inserts a copy of the element at the index, doubling the storage when
    // it is full; the element may live in the array. Returns the new size.
    unsigned int _Insert(unsigned long index, const void* element);  // 0x2ABE0
    // Replaces the elements with copies of the other array's, unless the
    // calling thread is imprinting. Not reconstructed.
    void _Copy(const PropArrayBase& other);  // 0xBA70

    void* mData;
    unsigned int mSize;
    unsigned int mCapacity;
    unsigned int mElemSize;
    // Set while mData points at storage the array does not own.
    unsigned int mStatic;
    // The element type's symbol; PropArray<T>::sType, which is empty for
    // the types seen.
    Symbol mType;
};

static_assert(offsetof(PropArrayBase, mData) == 8);
static_assert(offsetof(PropArrayBase, mSize) == 16);
static_assert(offsetof(PropArrayBase, mElemSize) == 24);
static_assert(offsetof(PropArrayBase, mType) == 32);
static_assert(sizeof(PropArrayBase) == 40);

// A typed PropArrayBase. The element operations are the template's, which
// the compiler emits for each element type as the binary does.
template <typename T>
class PropArray : public PropArrayBase {
public:
    // Inlined by every user, for example Entity::_CreateAndInsertNewGameObject
    // at 0xF0AD0.
    PropArray() {
        mData = nullptr;
        mSize = 0;
        mCapacity = 0;
        mElemSize = sizeof(T);
        mStatic = 0;
        mType = sType;
    }
    ~PropArray() override {
        Resize(0);
        if (mStatic == 0) {
            MemFree(mData);
        }
        mData = nullptr;
        mCapacity = 0;
    }

    void* Construct(unsigned long count, void* data) const override {
        T* element = static_cast<T*>(data);
        for (unsigned long index = 0; index < count; ++index) {
            new (element + index) T;
        }
        return data;
    }
    unsigned long Alignment() const override {
        return alignof(T);
    }
    void Copy(unsigned long count, void* data, const void* source) const override {
        T* element = static_cast<T*>(data);
        if (source == nullptr) {
            for (unsigned long index = 0; index < count; ++index) {
                new (element + index) T();
            }
            return;
        }
        const T* from = static_cast<const T*>(source);
        for (unsigned long index = 0; index < count; ++index) {
            new (element + index) T(from[index]);
        }
    }
    void Move(unsigned long count, void* data, void* source) const override {
        T* element = static_cast<T*>(data);
        T* from = static_cast<T*>(source);
        if (element < from) {
            for (unsigned long index = 0; index < count; ++index) {
                new (element + index) T(from[index]);
            }
        } else {
            for (unsigned long index = count; index != 0; --index) {
                new (element + index - 1) T(from[index - 1]);
            }
        }
    }
    void Destruct(unsigned long count, void* data) const override {
        T* element = static_cast<T*>(data);
        for (unsigned long index = 0; index < count; ++index) {
            element[index].~T();
        }
    }

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

    // The element type's symbol, empty unless a type sets it. Each
    // instantiation has a guarded copy, such as 0x19C53B0 for
    // PropArray<unsigned int>.
    static Symbol sType;
};

template <typename T>
Symbol PropArray<T>::sType;
