#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"

class BinStream;
class RBProfile;

// A profile's gameplay options (rb_meta/RBGameplayOptions.o,
// 0xD52AF0-0xD5342F): the lefty flags, guitar solo mode, drum fill logic,
// vocal improv, breakneck speed and the in-game, vocal stem and microphone
// volumes. In this build the values live in a second base class from the
// rb_game code (constructor 0xA4C280, destructor 0x92C580, vtable
// 0x197D908 at offset 24), which reads the "sound",
// "track_graphics" and "breakneck_multipliers" config and serves the
// gameplay code; RBGameplayOptions adds the saving. The primary vtable at
// 0x197D8A0 has 11 slots: PersistentData's, then four overriders of the
// second base's slots (0xD53000, 0xD530E0, 0xD53160, 0xD53230; the second
// and third set the "vca:/IGS" volume). The object is 208 bytes. Declared
// only: the object is not reconstructed, and the second base is kept as
// storage.
class RBGameplayOptions : public PersistentData {
public:
    DELETE_OVERLOAD

    explicit RBGameplayOptions(RBProfile* profile);  // 0xD52AF0
    // Slots 0-1: 0xD53300, 0xD53310; second-base thunks 0xD53330,
    // 0xD53340. The destructor only destroys the second base (0x92C580).
    ~RBGameplayOptions() override;

    // Slot 5.
    void Save(BinStream& stream) const override;  // 0xD52B40
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xD52CE0

    // The second base's slot 0 (0xA4D050), which restores the default
    // options; RBProfile::DeleteAll calls it. Declared here because the
    // second base is not modelled. Name not in the reference map.
    void ResetOptions();  // 0xA4D050

    // The second base: its vtable pointer and the option values. Its slot 0
    unsigned char mOptions[208 - sizeof(PersistentData)];
};

static_assert(sizeof(RBGameplayOptions) == 208);
