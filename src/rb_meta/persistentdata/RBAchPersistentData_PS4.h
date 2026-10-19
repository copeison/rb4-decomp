#pragma once

#include "os/memory/MemMgr.h"
#include "rb_meta/persistentdata/RBAchPersistentData.h"

class BinStream;
class RBProfile;

// The PS4 half of a profile's achievement progress
// (rb_meta/RBAchPersistentData_PS4.o, from 0xF02810): the trophy context and
// handle of the profile's pad, the trophies' unlock state and a threaded
// trophy unlocker. The profile creates it from the "profile" heap (512
// bytes) and owns it through its base (RBProfile::mAchData). The vtable is
// at 0x198F558, with the achievement interface's at 0x198F698 (offset 24).
// Declared only: the class is not reconstructed.
class RBAchPersistentData_PS4 : public RBAchPersistentData {
public:
    // Allocated from the "profile" heap; the profile's constructor inlines
    // it.
    static void* operator new(unsigned long size) {
        static const long sHeap = MemFindHeap("profile");
        MemPushHeap(sHeap);
        void* const allocation = MemAlloc(size, "RBAchPersistentData_PS4", 0);
        MemPopHeap();
        return allocation;
    }
    DELETE_OVERLOAD

    explicit RBAchPersistentData_PS4(RBProfile* profile);  // 0xF02810
    ~RBAchPersistentData_PS4() override;

    // Slot 5.
    void Save(BinStream& stream) const override;
    // Slot 6.
    void Load(BinStream& stream) override;

    // The trophy state; not reconstructed.
    unsigned char mTrophyState[512 - sizeof(RBAchPersistentData)];
};

static_assert(sizeof(RBAchPersistentData_PS4) == 512);
