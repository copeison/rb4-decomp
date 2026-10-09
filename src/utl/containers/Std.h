#pragma once

namespace HmxAllocator {

// EASTL allocator used by the engine's containers. It carries a debug name;
// the sized thunks ignore the object: allocate forwards to MemOrPoolAlloc
// with the label "StlAlloc", and deallocate to MemOrPoolFree.
class allocator {
public:
    explicit allocator(const char* name = nullptr) : mName(name) {}

    // Reconstructed from eboot.elf at 0x252CF0.
    void* allocate(unsigned long size, int flags = 0);
    // Reconstructed from eboot.elf at 0x252D30.
    void deallocate(void* allocation, unsigned long size);

    const char* mName;  // Name not in the reference map.
};

static_assert(sizeof(allocator) == 8);

// Shared instance for converted code that does not yet model its owning
// container's allocator member. Name not in the reference map.
extern allocator gStlAllocator;

}  // namespace HmxAllocator
