#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"
#include "utl/containers/Set.h"
#include "utl/text/Symbol.h"

class BinStream;
class RBProfile;

// A set of symbols a profile has marked as seen, newer than the map (its
// object is at 0xD69080-0xD697EB, after RBSystemOptions.o). RBProfile
// forwards a check (0xD59110 to 0xD695D0) and an insertion that reports
// whether the symbol was new (0xD59120 to 0xD69640). The vtable at 0x197E280
// has PersistentData's 7 slots; the object is 88 bytes. Name not in the
// reference map; what the symbols name is not known, so the evidence for
// the name is weak.
class RBSeenIdsPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit RBSeenIdsPersistentData(RBProfile* profile);  // 0xD69080
    // Slots 0-1: 0xD69240, 0xD69260.
    ~RBSeenIdsPersistentData() override;

    // Slot 5: the symbols in a revision-0 block.
    void Save(BinStream& stream) const override;  // 0xD692C0
    // Slot 6: adds the saved symbols to the set.
    void Load(BinStream& stream) override;  // 0xD693C0

    // The profile's platform name for logs. Name not in the reference map.
    const char* _GetProfileName() const;  // 0xD692A0
    // Forgets the symbols without marking the data dirty. Name not in the
    // reference map.
    void Clear();  // 0xD691B0
    // Names not in the reference map.
    bool HasSeenId(Symbol id) const;  // 0xD695D0
    // Adds the symbol and marks the data dirty; false when it was there.
    bool MarkIdSeen(Symbol id);  // 0xD69640

    // Field names are not in the reference map.
    RBProfile* mProfile;
    eastl::set<Symbol> mSeenIds;
};

static_assert(offsetof(RBSeenIdsPersistentData, mProfile) == 24);
static_assert(offsetof(RBSeenIdsPersistentData, mSeenIds) == 32);
static_assert(sizeof(RBSeenIdsPersistentData) == 88);
