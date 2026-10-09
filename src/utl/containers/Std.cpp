#include "utl/containers/Std.h"

#include "os/memory/MemMgr.h"

namespace HmxAllocator {

allocator gStlAllocator;

// Reconstructed from eboot.elf at 0x252CF0.
void* allocator::allocate(unsigned long size, int) {
    return MemOrPoolAlloc(size, "StlAlloc", 0);
}

// Reconstructed from eboot.elf at 0x252D30. The thunk leaves the third
// argument register holding the size; the free path does not read it.
void allocator::deallocate(void* allocation, unsigned long size) {
    MemOrPoolFree(size, allocation, nullptr);
}

}  // namespace HmxAllocator
