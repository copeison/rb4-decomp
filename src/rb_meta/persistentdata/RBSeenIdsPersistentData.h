#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"
#include "utl/containers/Set.h"

class BinStream;
class RBProfile;

// A set of 64-bit ids a profile has marked, newer than the map (its object is
// at 0xD69080-0xD697EB, after RBSystemOptions.o). RBProfile forwards a
// check (0xD59110 to 0xD695D0) and an insertion that reports whether the id
// was new (0xD59120 to 0xD69640). The vtable at 0x197E280 has
// PersistentData's 7 slots; the object is 88 bytes. Name not in the
// reference map; what the ids are is not known, so the evidence for the
// name is weak. Declared only.
class RBSeenIdsPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit RBSeenIdsPersistentData(RBProfile* profile);  // 0xD69080
    // Slots 0-1: 0xD69240, 0xD69260.
    ~RBSeenIdsPersistentData() override;

    // Slot 5.
    void Save(BinStream& stream) const override;  // 0xD692C0
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD693C0

    // Forgets the ids. Name not in the reference map.
    void Clear();  // 0xD691B0
    // Names not in the reference map.
    bool HasSeenId(unsigned long id) const;  // 0xD695D0
    bool MarkIdSeen(unsigned long id);       // 0xD69640

    // Field names are not in the reference map.
    RBProfile* mProfile;
    eastl::set<unsigned long> mSeenIds;
};

static_assert(offsetof(RBSeenIdsPersistentData, mProfile) == 24);
static_assert(offsetof(RBSeenIdsPersistentData, mSeenIds) == 32);
static_assert(sizeof(RBSeenIdsPersistentData) == 88);
