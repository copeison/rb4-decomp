#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"
#include "rb_game/options/GameplayOptions.h"

class BinStream;
class RBProfile;

// A profile's gameplay options (rb_meta/RBGameplayOptions.o,
// 0xD52AF0-0xD5342F). In the map's build the class held the values itself;
// in this build they live in GameplayOptions (the rb_game code, second base
// at offset 24, constructor 0xA4C280, destructor 0x92C580), and
// RBGameplayOptions saves them and reacts to their changes: any change marks
// the profile dirty and a volume change reaches the mixer. The primary vtable
// at 0x197D8A0 has 11 slots: PersistentData's 7 and the overriders of
// GameplayOptions' change hooks (its slots 34-37), whose thunks fill the
// GameplayOptions vtable at 0x197D908. The object is 208 bytes; the profile
// keeps one (RBProfile::mGameplayOptions) and the local band users one more.
class RBGameplayOptions : public PersistentData, public GameplayOptions {
public:
    DELETE_OVERLOAD

    explicit RBGameplayOptions(RBProfile* profile);  // 0xD52AF0
    // Slots 0-1: 0xD53300, 0xD53310; GameplayOptions thunks 0xD53330,
    // 0xD53340. Only GameplayOptions has something to destroy.
    ~RBGameplayOptions() override;

    // Slot 5: the options in a revision-11 block.
    void Save(BinStream& stream) const override;  // 0xD52B40
    // Slot 6. Revision 9 had a flag that is read and dropped; before
    // revision 3 the guitar solo mode and the drum fill logic get their
    // defaults (1 and 2), and the one microphone volume of revision 6 sets
    // all three.
    void Load(BinStream& stream) override;  // 0xD52CE0

    // Slots 7-10, overriding GameplayOptions' slots 34-37 (thunks 0xD53070,
    // 0xD53120, 0xD531D0, 0xD53290). Names not in the reference map.
    // Marks the profile dirty.
    void _OnOptionsChanged() override;  // 0xD53000
    // Sets the "vca:/IGS" volume to the option's decibels.
    void _OnIGSVolumeChanged() override;  // 0xD530E0
    // While a game exists (the map's theGame at 0x1AEF770), passes the
    // option's decibels to slot 37 of the object at offset 0xB0 of the
    // mutable game (theMutableGame, 0x1AEF778; accessor 0xA48560). Not
    // reconstructed: Game is rb_game code and is not modelled.
    void _OnVocalStemVolumeChanged() override;  // 0xD53160
    // Passes the microphone's decibels to slot 10 of the object slot 8 of the
    // optional service at 0x1ADF350 returns (created at 0x8EC9C0, updated by
    // optional_service_update), when there is one. Not reconstructed: the
    // service is not modelled.
    void _OnMicVolumeChanged(int mic) override;  // 0xD53230

    // Copies another player's options into the GameplayOptions part, without
    // marking the profile dirty; the local band users take their profile's
    // options with it (0x92B46E, 0x92B5CB, 0x92B66E). Name not in the
    // reference map.
    void SetOptions(const GameplayOptions& options);  // 0xD532F0

private:
    // The values GameplayOptions keeps from its offset 8, which the save
    // reads and writes directly; GameplayOptions declares them as opaque
    // storage (mOptions). The layout follows GameplayOptions' accessors
    // (0xA4D050-0xA4D5F0) and Clear. Names not in the reference map.
    struct SavedValues {
        int mGuitarSoloMode;
        int mDrumFillLogic;
        unsigned int mBreakneckSpeedModifierIndex;
        // IsLefty's controller types 1 and 0.
        bool mGuitarLefty;
        bool mDrumLefty;
        bool mVocalImprovEnabled;
        // GameplayOptions::IsGameplayFlagEnabled.
        bool mGameplayFlagEnabled;
        // The steps Clear restores, from the config.
        unsigned int mDefaultIGSVolumeIndex;
        unsigned int mDefaultVocalStemVolumeIndex;
        unsigned int mDefaultMicVolumeIndex;
        unsigned int mIGSVolumeIndex;
        unsigned int mVocalStemVolumeIndex;
        unsigned int mMicVolumeIndices[3];
    };

    SavedValues& _Values() {
        return *reinterpret_cast<SavedValues*>(mOptions);
    }
    const SavedValues& _Values() const {
        return *reinterpret_cast<const SavedValues*>(mOptions);
    }
};

static_assert(sizeof(RBGameplayOptions) == 208);
