#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"
#include "utl/text/Symbol.h"

class BinStream;
class RBProfile;

// A band user's preferences that follow the profile, newer than the map (its
// object is at 0xD68800-0xD6907F, after RBSystemOptions.o): a band user
// (0x92AFB0) copies two flags, a symbol, a 64-bit value and two more flags
// from its fields at +512 to +531 into it through six setters
// (0xD68C60-0xD68F30). The vtable at 0x197E210 has PersistentData's 7
// slots; the object is 56 bytes. Name not in the reference map; the
// evidence for it is weak. Declared only.
class RBUserPreferencesPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit RBUserPreferencesPersistentData(RBProfile* profile);  // 0xD68800
    // Slots 0-1: 0xD687E0, 0xD687F0. The profile's destructor does not call
    // it: its body is empty.
    ~RBUserPreferencesPersistentData() override;

    // Slot 5.
    void Save(BinStream& stream) const override;  // 0xD68920
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD68A50

    // Restores the defaults. Name not in the reference map.
    void Clear();  // 0xD688A0

    // Field names are not in the reference map; what the values mean is
    // not known.
    Symbol mSymbolValue;
    long mLongValue;
    bool mFlags[4];
    RBProfile* mProfile;
};

static_assert(offsetof(RBUserPreferencesPersistentData, mSymbolValue) == 24);
static_assert(offsetof(RBUserPreferencesPersistentData, mLongValue) == 32);
static_assert(offsetof(RBUserPreferencesPersistentData, mProfile) == 48);
static_assert(sizeof(RBUserPreferencesPersistentData) == 56);
