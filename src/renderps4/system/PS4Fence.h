#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "render/system/RndFence.h"

// GPU fence backed by a 32-bit label the GPU writes. Its vtable is at
// 0x195F6D8. Name from the "PS4Fence" allocation label.
class PS4Fence : public RndFence {
public:
    PS4Fence();            // 0x8E1570
    ~PS4Fence() override;  // 0x8E15F0, 0x8E1680

    // Reconstructed from eboot.elf at 0x8E16D0. Advances the sequence; on
    // wraparound the label is reallocated. Name not in the reference map.
    unsigned int NextValue();

    // Reconstructed from eboot.elf at 0x8E1640: the destructor body without
    // the vtable reset. Name not in the reference map.
    void FreeLabel();

    DELETE_OVERLOAD

    // Field names are not in the reference map.
    unsigned int* mLabel;
    unsigned int mSequence;
    // Alignment padding; nothing reads or writes it.
    unsigned int mPad;
};

static_assert(offsetof(PS4Fence, mLabel) == 8);
static_assert(sizeof(PS4Fence) == 24);
