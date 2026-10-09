#pragma once

// Small-block pool shared by objects of up to 128 bytes, at 0x3852F0 and
// 0x385460. The map has PoolAlloc(unsigned long, unsigned long, char const*)
// and PoolFree(unsigned long, void*, char const*); this build's callers pass
// the size and, for allocation, a label.
void* PoolAlloc(unsigned long size, const char* name);
void PoolFree(unsigned long size, void* allocation);

// Milo's per-class pool allocation overload.
#define POOL_OVERLOAD(class_name)                                   \
    static void* operator new(unsigned long size) {                 \
        return PoolAlloc(size, nullptr);                            \
    }                                                               \
    static void* operator new(unsigned long, void* place) {         \
        return place;                                               \
    }                                                               \
    static void operator delete(void* allocation) {                 \
        PoolFree(sizeof(class_name), allocation);                   \
    }
