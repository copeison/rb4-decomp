#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"
#include "utl/text/Symbol.h"

class BinStream;
class RBProfile;

// The result of a playthrough of the story mode ("BTM", the "btm_*"
// strings): a value that ranks playthroughs and a symbol. It is
// persistent data of the rb_game BTM code (vtable 0x1963298), which the
// game fills when a playthrough ends (0x9312E0, next to
// "campaign_restart"). Only the members RBBTMPersistentData uses are
// declared; the class is not reconstructed, and it is declared here because
// no rb_game header models it. Class and field names are not in the
// reference map; the evidence for them is weak.
class RBBTMPlaythroughRecord : public PersistentData {
public:
    // Inline: the game builds records on the stack (0x9313C2) and in
    // RBBTMPersistentData's constructor.
    explicit RBBTMPlaythroughRecord(IDirty* parent = nullptr) {
        mParent = parent;
        Reset();
    }
    // Slots 0-1: 0x9D40F0, 0x9D4100 (identical-code copies). Inline: no
    // destructor call follows the stack records.
    ~RBBTMPlaythroughRecord() override {}

    // Slot 5: the value and the symbol in a revision block.
    void Save(BinStream& stream) const override;  // 0x9C82D0
    // Slot 6.
    void Load(BinStream& stream) override;  // 0x9C83B0

    // Copies the value and the symbol, not the parent, and marks the record
    // dirty.
    RBBTMPlaythroughRecord& operator=(const RBBTMPlaythroughRecord& other);  // 0x9C81E0
    // Sets the value to -1 and the symbol to the empty one.
    void Reset();  // 0x9C8260

    // Field names are not in the reference map. The value is compared to
    // keep the best playthrough.
    int mScore;
    Symbol mName;
};

static_assert(offsetof(RBBTMPlaythroughRecord, mScore) == 24);
static_assert(offsetof(RBBTMPlaythroughRecord, mName) == 32);
static_assert(sizeof(RBBTMPlaythroughRecord) == 40);

// A profile's story-mode ("BTM") playthrough statistics, newer than the map
// (its object is at 0xD46010-0xD4649B, between FTUEPersistentData.o and
// RBCampaignBandPersistentData.o): how many playthroughs started and ended
// and the best one. The vtable at 0x197D498 has PersistentData's 7 slots;
// the object is 80 bytes. Name not in the reference map; the evidence for
// it (the record type's module and the alphabetical object order) is weak.
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
    // Clears the counters and the record without marking the data dirty.
    // Name not in the reference map.
    void Clear();  // 0xD46080
    // Counts a new playthrough, without marking the data dirty (the game
    // calls it at 0x931A73). Name not in the reference map.
    void StartPlaythrough();  // 0xD46380
    // Counts an ended playthrough and keeps its record when it beats the
    // best one (the game calls it at 0x931452). Name not in the reference
    // map.
    void EndPlaythrough(const RBBTMPlaythroughRecord& record);  // 0xD463E0

    // Field names are not in the reference map.
    RBProfile* mProfile;
    int mPlaythroughsStarted;
    int mPlaythroughsEnded;
    // Its parent stays null: the assignment does not copy it.
    RBBTMPlaythroughRecord mBestPlaythrough;
};

static_assert(offsetof(RBBTMPersistentData, mProfile) == 24);
static_assert(offsetof(RBBTMPersistentData, mPlaythroughsStarted) == 32);
static_assert(offsetof(RBBTMPersistentData, mPlaythroughsEnded) == 36);
static_assert(offsetof(RBBTMPersistentData, mBestPlaythrough) == 40);
static_assert(sizeof(RBBTMPersistentData) == 80);
