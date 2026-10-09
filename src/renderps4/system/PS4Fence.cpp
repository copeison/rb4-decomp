#include "renderps4/system/PS4Fence.h"

#include <limits>

#include "render/core/transition_aliases.h"
#include "renderps4/system/PS4Device.h"

namespace {

unsigned int* AllocateLabel() {
    auto* label =
        static_cast<unsigned int*>(MemAlloc(sizeof(unsigned int), "PS4Fence", 4));
    *label = 0;
    return label;
}

void ReleaseLabel(unsigned int* label) {
    if (gPS4Device != nullptr) {
        gPS4Device->DeferredDelete(label);
    } else {
        MemFree(label);
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x8E1570.
PS4Fence::PS4Fence() : mLabel(nullptr), mSequence(0) {
    mLabel = AllocateLabel();
}

// Reconstructed from eboot.elf at 0x8E15F0. The deleting destructor at
// 0x8E1680 releases the fence through MemFree.
PS4Fence::~PS4Fence() {
    FreeLabel();
}

void PS4Fence::FreeLabel() {
    ReleaseLabel(mLabel);
    mLabel = nullptr;
}

unsigned int PS4Fence::NextValue() {
    auto sequence = mSequence;
    if (sequence == std::numeric_limits<unsigned int>::max()) {
        mSequence = 0;
        ReleaseLabel(mLabel);
        mLabel = AllocateLabel();
        sequence = mSequence;
    }

    ++sequence;
    mSequence = sequence;
    return sequence;
}
