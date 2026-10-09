#pragma once

// Non-owning view of a contiguous element list. Field names are not in the
// reference map.
template <typename T>
struct VectorAdapter {
    const T* mData;
    unsigned long mSize;
};
