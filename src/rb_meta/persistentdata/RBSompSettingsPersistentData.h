#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"

class BinStream;
class RBProfile;

// The online session settings a profile used last, newer than the map (its
// object is at 0xD65CB0-0xD663EF, before RBSystemOptions.o): the setlist
// type, order and length and the gameplay modifiers of the social online
// multiplayer code (a 40-byte settings value, constructor 0xA835F0, copied
// from the defaults at 0x1B1FD30). The vtable at 0x197E098 has
// PersistentData's 7 slots; the object is 64 bytes. Name not in the
// reference map; the evidence for it is weak. Declared only.
class RBSompSettingsPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit RBSompSettingsPersistentData(RBProfile* profile);  // 0xD65CB0
    // Slots 0-1: 0xD65870, 0xD65880. The profile's destructor does not call
    // it: its body is empty.
    ~RBSompSettingsPersistentData() override;

    // Slot 5.
    void Save(BinStream& stream) const override;  // 0xD65D20
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD65F40

    // Restores the default settings. Name not in the reference map.
    void Clear();  // 0xD65D00

    // The settings; not reconstructed.
    unsigned char mSettings[64 - sizeof(PersistentData)];
};

static_assert(sizeof(RBSompSettingsPersistentData) == 64);
