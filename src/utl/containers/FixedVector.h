#pragma once

#include <new>

// Fixed-capacity vector with inline storage. Name from the map's
// FixedVector<T, N>; mData points at mStorage. Elements are constructed only
// when added: the storage is a union member, so the vector's constructors
// leave it raw. Copies clear the target and append each element, as the
// binary's inlined copies do (for example RndCameraContext's at 0x41DA90).
template <typename T, unsigned long N>
struct FixedVector {
    FixedVector() : mData(mStorage), mSize(0), mCapacity(N) {}

    FixedVector(const FixedVector& other) : mData(mStorage), mSize(0), mCapacity(N) {
        for (unsigned long i = 0; i < other.mSize; ++i) {
            push_back(other.mData[i]);
        }
    }

    FixedVector& operator=(const FixedVector& other) {
        clear();
        for (unsigned long i = 0; i < other.mSize; ++i) {
            push_back(other.mData[i]);
        }
        return *this;
    }

    unsigned long size() const { return mSize; }
    bool empty() const { return mSize == 0; }
    T& operator[](unsigned long index) { return mData[index]; }
    const T& operator[](unsigned long index) const { return mData[index]; }
    T* begin() { return mData; }
    T* end() { return mData + mSize; }
    const T* begin() const { return mData; }
    const T* end() const { return mData + mSize; }

    void push_back(const T& value) {
        new (&mData[mSize++]) T(value);
    }

    // Appends default-constructed elements up to the count, or drops the
    // elements past it.
    void resize(unsigned long count) {
        while (mSize < count) {
            new (&mData[mSize++]) T();
        }
        while (mSize > count) {
            mData[--mSize].~T();
        }
    }

    void clear() { resize(0); }

    T* mData;
    unsigned long mSize;
    unsigned long mCapacity;
    union {
        T mStorage[N];
    };
};
