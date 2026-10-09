#include <cstdlib>
#include <cstring>
#include <kernel.h>

#include "os/memory/MemHeap.h"
#include "os/memory/MemMgr.h"

// Reconstructed from eboot.elf at 0x385E60. Type 0 is GPU memory: write-
// combined direct memory, booked in a separate block of 1/256 of its size.
// Type 1 is CPU direct memory and type 2 the C heap.
void AddPS4Heap(const char* name, int heap, int type, unsigned long size) {
    const unsigned long bookkeeping = type == 0 ? size >> 8 : 0;
    size_t bytes = size;
    if (size == 0 && (type | 1) == 1) {
        off_t start = 0;
        sceKernelAvailableDirectMemorySize(0, sceKernelGetDirectMemorySize(), 0x10000, &start, &bytes);
    }
    int* mem;
    if (type == 2) {
        mem = static_cast<int*>(memalign(0x10000, bytes));
    } else {
        off_t physical = 0;
        sceKernelAllocateDirectMemory(0, sceKernelGetDirectMemorySize(), bytes, 0x10000,
                                      type == 0 ? SCE_KERNEL_WC_GARLIC : SCE_KERNEL_WB_ONION,
                                      &physical);
        void* address = nullptr;
        sceKernelMapDirectMemory(&address, bytes,
                                 SCE_KERNEL_PROT_CPU_READ | SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW,
                                 0, physical, 0x10000);
        mem = static_cast<int*>(address);
    }
    if (bookkeeping == 0) {
        gHeaps[heap].Init(name, heap, mem, bytes >> 2, MemHeap::kFirstFit, 0, true, nullptr, 0, 8, true);
        return;
    }
    off_t physical = 0;
    sceKernelAllocateDirectMemory(0, sceKernelGetDirectMemorySize(), bookkeeping, 0x10000,
                                  SCE_KERNEL_WB_ONION, &physical);
    void* address = nullptr;
    sceKernelMapDirectMemory(&address, bookkeeping,
                             SCE_KERNEL_PROT_CPU_READ | SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW,
                             0, physical, 0x10000);
    gHeaps[heap].Init(name, heap, static_cast<int*>(address), bookkeeping >> 2, MemHeap::kFirstFit, 0,
                      true, mem, bytes >> 2, 8, true);
}

// Reconstructed from eboot.elf at 0x386040.
void PlatformPhysicalMemoryUsage(unsigned long& used, unsigned long& free) {
    free = 0;
    used = 0;
}

// Reconstructed from eboot.elf at 0x1180E00. The size sits 8 bytes before
// the returned memory and the offset from the C allocation 2 bytes before.
void* PlatformAlloc(unsigned long size, int align) {
    if (size == 0) {
        return nullptr;
    }
    int alignment = align & ~7;
    if (alignment == 0) {
        alignment = 8;
    }
    auto* block = static_cast<char*>(malloc(alignment + size));
    const unsigned short offset =
        static_cast<unsigned short>(alignment - (reinterpret_cast<unsigned long>(block) & (alignment - 1)));
    char* allocation = block + offset;
    reinterpret_cast<unsigned long*>(allocation)[-1] = size;
    reinterpret_cast<unsigned short*>(allocation)[-1] = offset;
    return allocation;
}

// Reconstructed from eboot.elf at 0x1180E60.
unsigned long PlatformFree(void* allocation) {
    if (allocation == nullptr) {
        return 0;
    }
    auto* bytes = static_cast<char*>(allocation);
    const unsigned long size = reinterpret_cast<unsigned long*>(bytes)[-1] & 0xFFFFFF;
    free(bytes - reinterpret_cast<unsigned short*>(bytes)[-1]);
    return size;
}

// Reconstructed from eboot.elf at 0x1180E90.
void* PlatformRealloc(void* allocation, unsigned long size, int align, unsigned long* moved) {
    char* resized = nullptr;
    if (size != 0) {
        int alignment = align & ~7;
        if (alignment == 0) {
            alignment = 8;
        }
        auto* block = static_cast<char*>(malloc(alignment + size));
        const unsigned short offset = static_cast<unsigned short>(
            alignment - (reinterpret_cast<unsigned long>(block) & (alignment - 1)));
        resized = block + offset;
        reinterpret_cast<unsigned long*>(resized)[-1] = size;
        reinterpret_cast<unsigned short*>(resized)[-1] = offset;
    }
    if (allocation != nullptr) {
        auto* bytes = static_cast<char*>(allocation);
        const unsigned long oldSize = reinterpret_cast<unsigned long*>(bytes)[-1] & 0xFFFFFF;
        memcpy(resized, allocation, oldSize < size ? oldSize : size);
        free(bytes - reinterpret_cast<unsigned short*>(bytes)[-1]);
    }
    if (moved != nullptr) {
        *moved = 0;
    }
    return resized;
}
