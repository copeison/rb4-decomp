#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"

class BinStream;
class RBProfile;

// A profile's characters (rb_meta/RBCharacterPersistentData.o,
// 0xD4D520-0xD4FC1F): the characters it created, the one it saved last and
// which band users use which character, the last behind a lock. The vtable
// at 0x197D7A8 has PersistentData's 7 slots; the object is 128 bytes.
// Declared only: the object is not reconstructed.
class RBCharacterPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit RBCharacterPersistentData(RBProfile* profile);  // 0xD4D520
    // Slots 0-1: 0xD4D610, 0xD4D6C0.
    ~RBCharacterPersistentData() override;

    // Slot 5.
    void Save(BinStream& stream) const override;  // 0xD4D890
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD4D9E0

    // The profile's platform name for logs, or "NULL_PROFILE_POINTER!!!".
    const char* _GetProfileName() const;  // 0xD4D6E0
    // Forgets the characters.
    void Clear();  // 0xD4D700

    // The profile (+24), the characters, the saved character and the users'
    // characters with their lock; not reconstructed.
    unsigned char mCharacterState[128 - sizeof(PersistentData)];
};

static_assert(sizeof(RBCharacterPersistentData) == 128);
