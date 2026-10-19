#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class BinStream;
class RBProfile;

// The first-time user experience events a profile has seen
// (rb_meta/FTUEPersistentData.o, 0xD45270-0xD456EB): the names of the tips
// and introductions already shown, so that each shows once. The vtable at
// 0x197D408 has PersistentData's 7 slots; the object is 56 bytes. The
// profile keeps one (RBProfile::mFTUEData).
class FTUEPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit FTUEPersistentData(RBProfile* profile);  // 0xD45270
    // Slots 0-1: 0xD452B0, 0xD452E0.
    ~FTUEPersistentData() override;

    // Slot 5: the events in a revision-0 block.
    void Save(BinStream& stream) const override;  // 0xD45320
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD45420

    // Records the event and marks the data dirty, also when it was seen
    // before.
    void MarkEventSeen(Symbol event);  // 0xD45540
    bool HasSeenEvent(Symbol event) const;  // 0xD45670
    // Forgets the events without marking the data dirty.
    void Reset();  // 0xD456A0

    // Field name not in the reference map.
    eastl::vector<Symbol> mSeenEvents;
};

static_assert(offsetof(FTUEPersistentData, mSeenEvents) == 24);
static_assert(sizeof(FTUEPersistentData) == 56);
