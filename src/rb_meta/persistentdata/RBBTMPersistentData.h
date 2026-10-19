#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"

class BinStream;
class RBProfile;

// A profile's Rivals story-mode statistics, newer than the map (its object
// is at 0xD46010-0xD4649B, between FTUEPersistentData.o and
// RBCampaignBandPersistentData.o): two counters and the best show record,
// a 32-byte persistent record of the "BTM" (story mode) code (vtable
// 0x19632A8). The vtable at 0x197D4A8 has PersistentData's 7 slots; the
// object is 80 bytes. Name not in the reference map; the evidence for it
// (the record type's module and the alphabetical object order) is weak.
// Declared only.
class RBBTMPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit RBBTMPersistentData(RBProfile* profile);  // 0xD46010
    // Slots 0-1: 0xD46150 (empty), 0xD46160.
    ~RBBTMPersistentData() override;

    // Slot 5: the counters and the record in a revision-0 block.
    void Save(BinStream& stream) const override;  // 0xD46190
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD46280

    // The profile's platform name for logs. Name not in the reference map.
    const char* _GetProfileName() const;  // 0xD46170
    // Clears the counters and the record. Name not in the reference map.
    void Clear();  // 0xD46080

    // Field names are not in the reference map; the record is not
    // reconstructed. The first counter grows at 0xD46380, the second at
    // 0xD463E0, which also keeps the better record.
    RBProfile* mProfile;
    int mShowsStarted;
    int mShowsCompleted;
    unsigned char mBestShow[80 - 40];
};

static_assert(offsetof(RBBTMPersistentData, mProfile) == 24);
static_assert(offsetof(RBBTMPersistentData, mShowsStarted) == 32);
static_assert(sizeof(RBBTMPersistentData) == 80);
