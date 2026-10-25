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
// from its fields at +512 to +531 into it through the six setters, and its
// getters fall back to them. The vtable at 0x197E210 has PersistentData's 7
// slots; the object is 56 bytes. Name not in the reference map; the
// evidence for it is weak.
class RBUserPreferencesPersistentData : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit RBUserPreferencesPersistentData(RBProfile* profile);  // 0xD68800
    // Slots 0-1: 0xD68FB0, 0xD68FC0. The profile's destructor does not call
    // it: its body is empty.
    ~RBUserPreferencesPersistentData() override;

    // Slot 5: the values in a revision-1 block; the 64-bit value is saved
    // as 32 bits.
    void Save(BinStream& stream) const override;  // 0xD68920
    // Slot 6: the last two flags need revision 1.
    void Load(BinStream& stream) override;  // 0xD68A50

    // Restores the defaults, except the second flag, without marking the
    // data dirty. Name not in the reference map.
    void Clear();  // 0xD688A0

    // Each setter marks the data dirty only when the value changes. Names
    // not in the reference map; what the values mean is not known.
    bool IsFirstFlagSet() const;               // 0xD68C50
    void SetFirstFlag(bool set);               // 0xD68C60
    bool IsSecondFlagSet() const;              // 0xD68CE0
    void SetSecondFlag(bool set);              // 0xD68CF0
    Symbol GetSymbolValue() const;             // 0xD68D70
    unsigned long GetLongValue() const;        // 0xD68D80
    void SetSymbolValue(Symbol value);         // 0xD68D90
    void SetLongValue(unsigned long value);    // 0xD68E10
    bool IsThirdFlagSet() const;               // 0xD68E90
    bool IsFourthFlagSet() const;              // 0xD68EA0
    void SetThirdFlag(bool set);               // 0xD68EB0
    void SetFourthFlag(bool set);              // 0xD68F30

    // Field names are not in the reference map; what the values mean is
    // not known. The defaults are the empty symbol, -1, true and false.
    Symbol mSymbolValue;
    unsigned long mLongValue;
    bool mFlags[4];
    RBProfile* mProfile;
};

static_assert(offsetof(RBUserPreferencesPersistentData, mSymbolValue) == 24);
static_assert(offsetof(RBUserPreferencesPersistentData, mLongValue) == 32);
static_assert(offsetof(RBUserPreferencesPersistentData, mFlags) == 40);
static_assert(offsetof(RBUserPreferencesPersistentData, mProfile) == 48);
static_assert(sizeof(RBUserPreferencesPersistentData) == 56);
