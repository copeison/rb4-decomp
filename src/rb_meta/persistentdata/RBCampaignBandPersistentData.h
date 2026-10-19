#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

class BinStream;
class RBProfile;

// A campaign band's saved progress (rb_meta/RBCampaignBandPersistentData.o,
// 0xD464A0-0xD4B65F): its name and guid, the cities and tours it reached,
// its session musicians, its songs and cash, and its Rivals progress. The
// profile keeps its bands in a vector (RBProfile::mCampaignBands). The
// vtable at 0x197D4F0 has PersistentData's 7 slots; the object is 768
// bytes. Declared only: the object is not reconstructed, and only the
// members the profile uses are laid out.
class RBCampaignBandPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit RBCampaignBandPersistentData(RBProfile* profile);  // 0xD464A0
    // Not in the map's build; the profile's band vector copies and moves
    // bands with them.
    RBCampaignBandPersistentData(const RBCampaignBandPersistentData& other);  // 0xD46C70
    RBCampaignBandPersistentData& operator=(const RBCampaignBandPersistentData& other);  // 0xD47050
    // Slots 0-1: 0xD46B50, 0xD46C50.
    ~RBCampaignBandPersistentData() override;

    // Slot 5.
    void Save(BinStream& stream) const override;  // 0xD475D0
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD47B60

    // Gives a new band its guid.
    void GenerateGuid();  // 0xD48530
    void MarkAsRecentlyPlayed(bool recentlyPlayed);  // 0xD49E30

    // The band's name, home city, cities, tours and the rest of its state;
    // not reconstructed.
    unsigned char mBandState[352 - sizeof(PersistentData)];
    // Set on the band the profile played last. Name not in the reference
    // map; MarkAsRecentlyPlayed sets it.
    bool mRecentlyPlayed;
    // The band's songs and Rivals progress; not reconstructed.
    unsigned char mSongState[768 - 353];
};

static_assert(offsetof(RBCampaignBandPersistentData, mRecentlyPlayed) == 352);
static_assert(sizeof(RBCampaignBandPersistentData) == 768);
