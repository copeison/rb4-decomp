#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"

class BinStream;
class RBProfile;

// A song's saved results for a profile (rb_meta/SongPersistentData.o, from
// 0xD69800): the scores, stars and accuracies per score type and
// difficulty, the play count and when it was last played. The vtable at
// 0x197E2C8 has PersistentData's slots, MakeDirty (which also flags the
// song for a leaderboard upload) and an empty eighth slot that
// QuickplaySongPersistentData overrides. Declared only: the object is not
// reconstructed.
class SongPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    SongPersistentData(RBProfile* profile, int songId);  // 0xD69800
    SongPersistentData(const SongPersistentData& other);  // 0xD6A080
    // Slots 0-1: 0xD6A8F0 (empty), 0xD6A900.
    ~SongPersistentData() override;

    // Slot 2.
    void MakeDirty() override;  // 0xD6A910
    // Slot 5.
    void Save(BinStream& stream) const override;  // 0xD6A920
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD6AD10
    // Slot 7: empty here. The map's QuickplaySongPersistentData::SetChecksum
    // overrides it; the base's slot is newer than the map.
    virtual void SetChecksum(const unsigned char* checksum);  // 0xD6C820

    // The song's id. Inline in this build; the binary's copy is at
    // 0xD6BF00.
    int GetID() const;  // 0xD6BF00
    // The score the profile's career total counts. Name not in the
    // reference map; the evidence for it is weak.
    int GetCareerScore() const;  // 0xD6BE20

    // The results; not reconstructed.
    unsigned char mResults[408 - sizeof(PersistentData)];
};

static_assert(sizeof(SongPersistentData) == 408);

// A quickplay song's results with the checksum of the song they were played
// on (rb_meta/SongPersistentData.o). The profile keeps them in two maps keyed
// by song id. The vtable is at 0x197E318; the object is 464 bytes. Declared
// only.
class QuickplaySongPersistentData : public SongPersistentData {
public:
    QuickplaySongPersistentData(RBProfile* profile, int songId);  // 0xD6C0A0
    QuickplaySongPersistentData(const QuickplaySongPersistentData& other);  // 0xD6C120
    QuickplaySongPersistentData& operator=(const QuickplaySongPersistentData& other);  // 0xD6C210
    // Slots 0-1: 0xD6C830 (empty), 0xD6C840.
    ~QuickplaySongPersistentData() override;

    // Slot 5.
    void Save(BinStream& stream) const override;  // 0xD6C2B0
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD6C3B0
    // Slot 7: a changed checksum resets the results.
    void SetChecksum(const unsigned char* checksum) override;  // 0xD6C550

    // The checksum; not reconstructed.
    unsigned char mChecksum[464 - 408];
};

static_assert(sizeof(QuickplaySongPersistentData) == 464);
