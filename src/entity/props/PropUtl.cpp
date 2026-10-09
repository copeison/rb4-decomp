#include "entity/props/PropUtl.h"

#include "entity/props/PropArray.h"

// PropArrayBase's out-of-line methods. The map keeps Resize and _Insert
// with the weak copies of entity/PropUtl.o's templates; this build places
// them in the early shared code.

thread_local bool PropArrayBase::sImprinting;

// Reconstructed from eboot.elf at 0xA660. Shrinking destroys the dropped
// elements; growing reallocates to exactly the size and value-initializes
// the new elements.
void PropArrayBase::Resize(unsigned long size) {
    const unsigned long current = mSize;
    if (current == size) {
        return;
    }
    if (current > size) {
        Destruct(current - size, ElementAt(size));
    }
    if (mCapacity < size) {
        void* const old = mData;
        mData = MemAlloc(size * mElemSize, "PropArrayBase", 0);
        if (mSize != 0) {
            Move(mSize, mData, old);
        }
        if (mStatic == 0) {
            MemFree(old);
        }
        mStatic = 0;
        mCapacity = static_cast<unsigned int>(size);
    }
    if (mSize < size) {
        Copy(size - mSize, ElementAt(mSize), nullptr);
    }
    mSize = static_cast<unsigned int>(size);
}

// Reconstructed from eboot.elf at 0xB4D0.
ScopedImprint::ScopedImprint() : mWasImprinting(PropArrayBase::sImprinting) {
    PropArrayBase::sImprinting = true;
}

// Reconstructed from eboot.elf at 0x2ABE0. An element inside the array is
// located before the storage moves, and taken from its new place.
unsigned int PropArrayBase::_Insert(unsigned long index, const void* element) {
    const unsigned char* const old = static_cast<const unsigned char*>(mData);
    const unsigned char* const source = static_cast<const unsigned char*>(element);
    unsigned long sourceIndex = static_cast<unsigned long>(-1);
    unsigned int size = mSize;
    if (old <= source && source < old + size * mElemSize) {
        sourceIndex = static_cast<unsigned long>(source - old) / mElemSize;
    }
    const unsigned int newSize = size + 1;
    if (newSize > mCapacity) {
        const unsigned long capacity = 2UL * newSize;
        if (mCapacity < capacity) {
            mData = MemAlloc(capacity * mElemSize, "PropArrayBase", 0);
            if (mSize != 0) {
                Move(mSize, mData, const_cast<unsigned char*>(old));
            }
            if (mStatic == 0) {
                MemFree(const_cast<unsigned char*>(old));
            }
            mStatic = 0;
            mCapacity = static_cast<unsigned int>(capacity);
            size = mSize;
        }
    }
    if (size > index) {
        Move(size - index, ElementAt(index + 1), ElementAt(index));
    }
    const void* copy = element;
    if (sourceIndex != static_cast<unsigned long>(-1)) {
        copy = ElementAt(sourceIndex + (sourceIndex >= index ? 1 : 0));
    }
    Copy(1, ElementAt(index), copy);
    mSize = newSize;
    return newSize;
}
