#pragma once

#include <cstring>

#include "os/memory/MemMgr.h"

// A vector of plain-old-data elements, grown with MemAlloc and copied with
// memcpy (the map's PodVector<T>). The map has no object for it: its only
// emitted members are in render/RndSceneDrawer.o and render/
// RndBasicCuller.o, so it lives with the scene drawer. Only the members the
// reconstructed code uses are declared; field names are not in the
// reference map.
//
// The element buffer is released by Free() rather than a destructor:
// FixedVector keeps its elements in union storage, which needs trivially
// destructible elements, and the scene drawer keeps its PodVectors in
// FixedVectors. The owners call Free() where the binary inlines the
// destructor.
template <typename T>
class PodVector {
public:
    typedef T* iterator;

    PodVector() : mData(nullptr), mSize(0), mCapacity(0) {}

    unsigned long size() const { return mSize; }
    unsigned long capacity() const { return mCapacity; }
    bool empty() const { return mSize == 0; }
    T* begin() { return mData; }
    T* end() { return mData + mSize; }
    const T* begin() const { return mData; }
    const T* end() const { return mData + mSize; }
    T& operator[](unsigned long index) { return mData[index]; }
    const T& operator[](unsigned long index) const { return mData[index]; }

    // Forgets the elements and keeps the buffer.
    void clear() { mSize = 0; }

    // Grows the buffer to hold at least `count` elements; it never shrinks.
    void reserve(unsigned long count) {
        if (mCapacity < count) {
            T* data = static_cast<T*>(MemAlloc(count * sizeof(T), "PodVector buffer", 8));
            if (mSize != 0) {
                memcpy(data, mData, mSize * sizeof(T));
            }
            if (mData != nullptr) {
                MemFree(mData);
            }
            mData = data;
            mCapacity = count;
        }
    }

    // Appends a copy, doubling a full buffer to at least ten elements (the
    // map's push_back; RndDrawInstance's at 0x45E8E0).
    void push_back(const T& value) {
        if (mSize == mCapacity) {
            reserve(mSize * 2 > 10 ? mSize * 2 : 10);
        }
        mData[mSize++] = value;
    }

    // Frees the buffer: the map's destructor. Name not in the reference
    // map.
    void Free() {
        mSize = 0;
        if (mData != nullptr) {
            MemFree(mData);
        }
        mData = nullptr;
        mCapacity = 0;
    }

    T* mData;
    unsigned long mSize;
    unsigned long mCapacity;
};

static_assert(sizeof(PodVector<int>) == 24);
