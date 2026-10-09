#pragma once

#include <new>

class CritSec;
class DataArray;
class FixedString;
class MemHeap;
class TextStream;
struct MemHeapStats;

// The engine's memory manager (os/MemMgr.o): a stack of named heaps per
// thread, allocations from the current heap, and statistics. Platform and
// project code (Mem_PS4.o, MemProject.o) creates the heaps.

// The number of heap slots.
constexpr int kMaxHeaps = 16;  // Name not in the reference map.
// MemTryFindHeap's result for a missing heap, and MemPrintOverview's
// selector for all heaps. Name not in the reference map.
constexpr long kNoHeap = -3;

// The heaps, at 0x19FF240, and how many are in use, at 0x1A01B40.
extern MemHeap gHeaps[kMaxHeaps];
extern unsigned long gNumHeaps;
// The heap allocations go to when the thread has pushed none; -1 is the
// system allocator. At 0x19BDA80.
extern int gDefaultHeap;
// Whether frees are queued for MemEndBatchingAndProcessFreeListInsertions.
// At 0x19FF234.
extern bool gBatchFreeListInsertions;
// Cleared by a failed allocation off the main thread. At 0x1A01B48.
extern bool gInsideMemFunc;
// The allocator's lock, created by the first MemAlloc. At 0x1A01B90.
extern CritSec* gMemLock;

// The current heap's number for the calling thread.
long GetCurrentHeapNum();  // 0x37A6B0
// Reads the "mem" configuration: the tracker settings and the batched free
// lists. Registers mem_print_concise_heap_report.
void MemInit(DataArray* config);  // 0x37AB70
// Empty in this build.
void MemTerminate();  // 0x37AE60

// Tracked heap allocation at 0x37AE70. The global operator new (0x37BF40)
// and operator new[] (0x37BF60) forward here with the labels "new" and
// "new[]"; operator delete and delete[] (0x37BF50, 0x37BF70) jump to MemFree.
// The last argument is the alignment; zero is the heap's default.
void* MemAlloc(unsigned long size, const char* name, int align);

// MemAlloc with the calling thread's temporary-allocation depth (+0x84 in
// its heap state) raised for the call. At 0x37B6F0.
void* MemAllocTemp(unsigned long size, const char* name, int align);

// Tracked heap release at 0x37B800. Null is ignored.
void MemFree(void* allocation);
// Resizes the allocation, moving it between heaps when needed.
void* MemRealloc(void* allocation, unsigned long size, const char* name, int align);  // 0x37B930

// Pool-or-heap pair at 0x37C020 and 0x37C040: requests of at most 128 bytes
// use the small-block pool, larger ones the tracked heap.
void* MemOrPoolAlloc(unsigned long size, const char* name, int align);
void MemOrPoolFree(unsigned long size, void* allocation, const char* name);

// Named heaps. MemFindHeap (0x37BA70) returns the heap's index or -1;
// MemPushHeap (0x37A920) and MemPopHeap (0x37A9B0) select the calling
// thread's current heap.
long MemFindHeap(const char* name);
// As MemFindHeap, returning kNoHeap for a missing heap. Name not in the
// reference map.
long MemTryFindHeap(const char* name);  // 0x37BAE0
void MemPushHeap(long heap);
void MemPopHeap();
// The heap's name; "system" for the system allocator.
const char* MemHeapName(long heap);  // 0x37BB60
bool MemHeapIsTracked(long heap);  // 0x37BB90
// The heap's size in bytes.
unsigned long MemHeapSize(unsigned long heap);  // 0x37BBB0
// The heap's allocation strategy, a MemHeap::Strategy. Names not in the
// reference map.
int MemHeapStrategy(long heap);                  // 0x37BBF0
void MemSetHeapStrategy(long heap, int strategy);  // 0x37BC60

// The heap's size and free-list statistics.
void MemFreeBlockStats(long heap, MemHeapStats& stats);  // 0x37A700
// Prints the heap's statistics and the change in its free bytes since the
// last call to TheDebug, after `label`.
void MemDelta(const char* label, int heap);  // 0x37A790
// Writes the heap as JSON.
void MemPrint(long heap, TextStream& stream, bool freeOnly);  // 0x37BDD0
// Queues frees until the second call returns them to the free lists in one
// pass.
void MemBeginBatchingFreeListInsertions();          // 0x37BE60
void MemEndBatchingAndProcessFreeListInsertions();  // 0x37BEB0

// Set once an allocation failed.
bool IsInMemoryFailure();  // 0x37C060
void SetMemoryFailure();   // 0x37C070

// The tracked heap's allocated bytes and its allocation and free counts.
void MemGetStats(unsigned long& bytes, int& allocs, int& frees);  // 0x37C080

// Appends the heap report to `out`. The memory overlay passes kNoHeap for
// every heap.
void MemPrintOverview(int heap, FixedString& out);  // 0x37B350
// One line per heap to `stream`, then a new line to TheDebug.
void MemPrintConciseHeapReport(TextStream& stream);  // 0x37C0A0

// The byte count scaled to B, KB, MB or GB, in the MakeString buffer.
const char* PrintBytes(unsigned long bytes);  // 0x37A550

// Thread-local temporary-heap scope at 0x37AA30 and 0x37AAF0. The map has
// MemPushTemp() and MemPopTemp() in this position; this build saves the
// calling thread's mode word (+0x84 in its heap state) into `saved` and, when
// `apply` is set, enables or clears it; MemPopTemp restores the saved word.
void MemPushTemp(unsigned int& saved, bool enable, bool apply);
void MemPopTemp(const unsigned int& saved);

// The project's heaps: fmod, debug, metadata, raknet, profile, gpu, failure
// and main, the default. The first MemAlloc calls it. In rb_* code, the
// map's os/MemProject.o.
void ProjectMemPreInit();  // 0x92AA50

// os/Mem_PS4.o.
// Creates heap `heap` from direct memory, or from the libc heap for the
// "failure" type. A zero size takes all the available direct memory.
void AddPS4Heap(const char* name, int heap, int type, unsigned long size);  // 0x385E60
// Zero in this build.
void PlatformPhysicalMemoryUsage(unsigned long& used, unsigned long& free);  // 0x386040

// The C allocator with the size and the alignment offset stored before the
// returned memory, used when no heap applies. Names not in the reference
// map; the binary places them after the second utl block.
void* PlatformAlloc(unsigned long size, int align);  // 0x1180E00
// Returns the allocation's size.
unsigned long PlatformFree(void* allocation);  // 0x1180E60
void* PlatformRealloc(void* allocation, unsigned long size, int align,
                      unsigned long* moved);  // 0x1180E90

// Milo's per-class delete overload. Classes whose deleting destructors call
// MemFree directly in the binary declare it instead of using the global
// operator delete.
#define DELETE_OVERLOAD                     \
    static void operator delete(void* v) {  \
        MemFree(v);                         \
    }
