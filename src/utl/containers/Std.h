#pragma once

namespace HmxAllocator {

// EASTL allocator used by the engine's containers. The sized thunks ignore
// the allocator object: allocate forwards to MemOrPoolAlloc with the label
// "StlAlloc", and deallocate to MemOrPoolFree.
class allocator {
public:
    // Reconstructed from eboot.elf at 0x252CF0.
    void* allocate(unsigned long size, int flags = 0);
    // Reconstructed from eboot.elf at 0x252D30.
    void deallocate(void* allocation, unsigned long size);
};

// The reconstruction does not yet model each container's allocator member,
// so converted code allocates through this shared instance. Name not in the
// reference map.
extern allocator gStlAllocator;

}  // namespace HmxAllocator
