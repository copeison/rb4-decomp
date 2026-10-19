#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"

class BinStream;
class RBProfile;

// A profile's saved Rivals clan state, newer than the map (its object is at
// 0xD4FC20-0xD52AEB, between RBCharacterPersistentData.o and
// RBGameplayOptions.o): a few values and a map from ids to lists of 72-byte
// clan entries that the clan code writes (0xB24D00). The vtable at
// 0x197D7F0 has PersistentData's 7 slots; the object is 136 bytes. Name not
// in the reference map; the evidence for it (the clan code's entries and
// the object order) is weak. Declared only.
class RBClanPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit RBClanPersistentData(RBProfile* profile);  // 0xD4FC20
    // Slots 0-1: 0xD4FD80, 0xD4FDA0.
    ~RBClanPersistentData() override;

    // Slot 5.
    void Save(BinStream& stream) const override;  // 0xD4FE00
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD500B0

    // Restores the defaults. Name not in the reference map.
    void Clear();  // 0xD4FCD0

    // The profile (+24) and the clan state; not reconstructed.
    unsigned char mClanState[136 - sizeof(PersistentData)];
};

static_assert(sizeof(RBClanPersistentData) == 136);
