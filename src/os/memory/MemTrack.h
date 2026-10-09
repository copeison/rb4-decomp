#pragma once

#include "utl/text/Str.h"

class TextStream;

// The allocation tracker's settings, read by MemInit from the "mem" block of
// the system configuration (os/MemTrack.o). The vtable is at 0x18FBD68.
class MemTrackParams {
public:
    MemTrackParams(int heap, int numAllocs, const String& type, bool callstacks)
        : mHeap(heap), mNumAllocs(numAllocs), mType(type), mCallstacks(callstacks) {}
    virtual ~MemTrackParams() {}

    // Field names are not in the reference map.
    // The track_heap, tracked_allocs, track_type and callstack_tracking
    // values.
    int mHeap;
    int mNumAllocs;
    String mType;
    bool mCallstacks;
};

static_assert(sizeof(MemTrackParams) == 0x28);

// One tracked allocation (os/AllocInfo.o). Not reconstructed.
class AllocInfo;

// Starts the allocation tracker.
void MemTrackInit(const MemTrackParams& params);  // 0x37F1F0
// Moves the tracked allocation at `old` to `allocation`.
void MemTrackRealloc(void* old, long size, long alignedSize, void* allocation);  // 0x37F8C0
// The tracked allocation at the address, or null.
AllocInfo* MemTrackGetInfo(void* allocation);  // 0x37F970

// Writes the allocation's JSON fields.
TextStream& operator<<(TextStream& stream, const AllocInfo& info);  // 0x3AB630
