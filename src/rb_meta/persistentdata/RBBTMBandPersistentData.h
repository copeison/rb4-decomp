#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"
#include "rb_meta/persistentdata/RBBTMPersistentData.h"

class BinStream;
class RBCampaignBandPersistentData;

// A campaign band's story-mode ("BTM") progress, newer than the map (its
// object is at 0xD456F0-0xD4600F, between FTUEPersistentData.o and
// RBBTMPersistentData.o): the band's best playthrough and a second record
// of the rb_game BTM code (vtable 0x1963370, 168 bytes, with three
// vectors; Reset 0x9CA950, assignment 0x9CAA70). Each band holds one
// (RBCampaignBandPersistentData::mBTMData); its parent is the band. The
// vtable at 0x197D450 has PersistentData's 7 slots; the object is 240
// bytes. The game calls 0xD45DF0 (assigns the second record), 0xD45E60
// (resets it), 0xD45ED0 (resets it with 0x9CB4D0) and 0xD45F50 (keeps the
// better playthrough), all of which mark the data dirty; the static
// initializer is at 0xD45FD0. Class and field names are not in the
// reference map; the evidence for them (the record types and the
// alphabetical object order) is weak. Declared only: only the members the
// band uses are declared.
class RBBTMBandPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit RBBTMBandPersistentData(RBCampaignBandPersistentData* band);  // 0xD456F0
    // Copies the parent and the band, resets, then assigns.
    RBBTMBandPersistentData(const RBBTMBandPersistentData& other);  // 0xD45A00
    // Assigns both records and marks the data dirty.
    RBBTMBandPersistentData& operator=(const RBBTMBandPersistentData& other);  // 0xD45B30
    // Slots 0-1: 0xD459B0, 0xD459D0.
    ~RBBTMBandPersistentData() override;

    // Slot 5: both records in a revision-0 block.
    void Save(BinStream& stream) const override;  // 0xD45C30
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD45D00

    // Resets both records. Name not in the reference map.
    void Clear();  // 0xD45810
    // Sets the parent and the band. Name not in the reference map.
    void SetBand(RBCampaignBandPersistentData* band);  // 0xD45BC0

    // Field names are not in the reference map.
    RBCampaignBandPersistentData* mBand;
    RBBTMPlaythroughRecord mBestPlaythrough;
    // The second record; not reconstructed.
    unsigned char mProgress[240 - 72];
};

static_assert(offsetof(RBBTMBandPersistentData, mBand) == 24);
static_assert(offsetof(RBBTMBandPersistentData, mBestPlaythrough) == 32);
static_assert(offsetof(RBBTMBandPersistentData, mProgress) == 72);
static_assert(sizeof(RBBTMBandPersistentData) == 240);
