#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"

class BinStream;
class RBProfile;

// A profile's saved setlists, newer than the map (its object is at
// 0xD650F0-0xD65CAF, after RBRockShopPersistentData.o): a vector behind a
// lock that the setlist screens edit ("setlist_changed"). The vtable at
// 0x197E028 has PersistentData's 7 slots; the object is 80 bytes. Name not
// in the reference map; the evidence for it is weak. Declared only.
class RBSetlistPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit RBSetlistPersistentData(RBProfile* profile);  // 0xD650F0
    // Slots 0-1: 0xD651A0, 0xD65250.
    ~RBSetlistPersistentData() override;

    // Slot 5.
    void Save(BinStream& stream) const override;  // 0xD65290
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD653D0

    // Forgets the setlists. Name not in the reference map.
    void Clear();  // 0xD65530

    // The profile (+24), the lock and the setlists; not reconstructed.
    unsigned char mSetlistState[80 - sizeof(PersistentData)];
};

static_assert(sizeof(RBSetlistPersistentData) == 80);
