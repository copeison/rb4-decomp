#include <gnm.h>

#include "os/memory/MemMgr.h"

// Reconstructed from eboot.elf at 0x92AA50.
void ProjectMemPreInit() {
    AddPS4Heap("fmod", 1, 1, 0x10000000);
    AddPS4Heap("debug", 2, 1, 0x300000);
    AddPS4Heap("metadata", 3, 1, 0x7800000);
    AddPS4Heap("raknet", 4, 1, 0xC00000);
    AddPS4Heap("profile", 5, 1, 0x3200000);
    // The NEO GPU mode leaves 512 MB more for the GPU heap.
    AddPS4Heap("gpu", 6, 0,
               sce::Gnm::getGpuMode() == sce::Gnm::kGpuModeBase ? 0xAFC00000UL : 0xCFC00000UL);
    AddPS4Heap("failure", 7, 2, 0x100000);
    AddPS4Heap("main", 0, 1, 0);
    gNumHeaps = 8;
    gDefaultHeap = 0;
}
