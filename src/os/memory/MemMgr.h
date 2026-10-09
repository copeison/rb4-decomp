#pragma once

#include <new>

// Tracked heap allocation at 0x37AE70. The global operator new (0x37BF40)
// and operator new[] (0x37BF60) forward here with the labels "new" and
// "new[]"; operator delete and delete[] (0x37BF50, 0x37BF70) jump to MemFree.
// The last argument is passed as zero by every recovered caller; the map
// gives only its type.
void* MemAlloc(unsigned long size, const char* name, int unknown);

// Tracked heap release at 0x37B800. Null is ignored.
void MemFree(void* allocation);

// Pool-or-heap pair at 0x37C020 and 0x37C040: requests of at most 128 bytes
// use the small-block pool, larger ones the tracked heap.
void* MemOrPoolAlloc(unsigned long size, const char* name, int unknown);
void MemOrPoolFree(unsigned long size, void* allocation, const char* name);

// Named heaps. MemFindHeap (0x37BA70) returns the heap's index or -1;
// MemPushHeap (0x37A920) and MemPopHeap (0x37A9B0) select the calling
// thread's current heap.
long MemFindHeap(const char* name);
void MemPushHeap(long heap);
void MemPopHeap();
// The heap's size in bytes.
unsigned long MemHeapSize(unsigned long heap);  // 0x37BBB0

// The tracked heap's allocated bytes and its allocation and free counts.
void MemGetStats(unsigned long& bytes, int& allocs, int& frees);  // 0x37C080

class FixedString;

// Appends the heap report to `out`. The memory overlay passes -3.
void MemPrintOverview(int heap, FixedString& out);  // 0x37B350

// Thread-local temporary-heap scope at 0x37AA30 and 0x37AAF0. The map has
// MemPushTemp() and MemPopTemp() in this position; this build saves the
// calling thread's mode word (+0x84 in its heap state) into `saved` and, when
// `apply` is set, enables or clears it; MemPopTemp restores the saved word.
void MemPushTemp(unsigned int& saved, bool enable, bool apply);
void MemPopTemp(const unsigned int& saved);

// Milo's per-class delete overload. Classes whose deleting destructors call
// MemFree directly in the binary declare it instead of using the global
// operator delete.
#define DELETE_OVERLOAD                     \
    static void operator delete(void* v) {  \
        MemFree(v);                         \
    }
