#pragma once

// Fixed-capacity vector with inline storage. Name from the map's
// FixedVector<T, N>; mData points at mStorage.
template <typename T, unsigned long N>
struct FixedVector {
    FixedVector() : mData(mStorage), mSize(0), mCapacity(N) {}

    T* mData;
    unsigned long mSize;
    unsigned long mCapacity;
    T mStorage[N];
};
