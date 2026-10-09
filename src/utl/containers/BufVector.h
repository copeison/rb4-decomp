#pragma once

#include <new>

// A vector over storage its owner provides, often a stack buffer: the data,
// the size and the capacity, as FixedVector begins. The name is the map's
// BufVector<T>; the member names are not in the reference map. Elements are
// constructed only when added, and nothing checks the capacity.
template <typename T>
struct BufVector {
    BufVector() : mData(nullptr), mSize(0), mCapacity(0) {}
    BufVector(T* data, unsigned long capacity) : mData(data), mSize(0), mCapacity(capacity) {}

    unsigned long size() const { return mSize; }
    bool empty() const { return mSize == 0; }
    T& operator[](unsigned long index) { return mData[index]; }
    const T& operator[](unsigned long index) const { return mData[index]; }
    T* begin() { return mData; }
    T* end() { return mData + mSize; }
    const T* begin() const { return mData; }
    const T* end() const { return mData + mSize; }
    T& back() { return mData[mSize - 1]; }
    const T& back() const { return mData[mSize - 1]; }

    void push_back(const T& value) {
        new (&mData[mSize++]) T(value);
    }
    void pop_back() {
        mData[--mSize].~T();
    }

    // Appends value-initialized elements up to the count, or drops the
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
};
