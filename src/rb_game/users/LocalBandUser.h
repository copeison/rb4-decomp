#pragma once

#include <cstddef>

#include "rb_game/options/GameplayOptions.h"
#include "rb_game/users/RBBandUser.h"
#include "rb_meta/persistentdata/FTUEPersistentData.h"
#include "rb_meta/persistentdata/RBGameplayOptions.h"

class RBProfile;

// A band user playing on this console (0x92AB60-0x92C580): its options,
// first-time-user events and preferences come from the profile of its band
// slot, or from its own copies while it has none. The band user manager
// creates eight. Newer than the map; the class name is not in the reference
// map (the older game's LocalBandUser had the same role) and the evidence
// for it is weak. The vtable at 0x1960620 has 43 slots; the object is 552
// bytes.
class LocalBandUser : public RBBandUser {
public:
    // The options the user plays with during an online session: a view
    // that forwards every slot to the user's options (vtable 0x1960788, 32
    // slots at 0x92B8D0-0x92C510). Name not in the reference map; the
    // evidence is weak. Declared only.
    class SessionGameplayOptions : public IGameplayOptions {
    public:
        explicit SessionGameplayOptions(LocalBandUser* user) : mUser(user) {}

        void Clear() override;
        bool IsLefty(ControllerType type) const override;
        void SetLefty(ControllerType type, bool lefty) override;
        int GetGuitarSoloMode() const override;
        void SetGuitarSoloMode(int mode) override;
        int ToggleGuitarSoloMode() override;
        int GetDrumFillLogic() const override;
        void SetDrumFillLogic(int logic) override;
        bool IsVocalImprovEnabled() const override;
        void SetVocalImprovEnabled(bool enabled) override;
        unsigned int GetBreakneckSpeedModifierIndex() const override;
        bool IsBreakneckSpeedEnabled() const override;
        float GetBreakneckSpeedModifier() const override;
        float GetBreakneckSpeedModifierNormalized() const override;
        void SetBreakneckSpeedModifierIndex(unsigned int index) override;
        bool RaiseBreakneckSpeedModifier() override;
        bool LowerBreakneckSpeedModifier() override;
        float GetIGSVolumeDecibels() const override;
        float GetIGSVolumeNormalized() const override;
        bool RaiseIGSVolume() override;
        bool LowerIGSVolume() override;
        float GetVocalStemVolumeDecibels() const override;
        float GetVocalStemVolumeNormalized() const override;
        bool RaiseVocalStemVolume() override;
        bool LowerVocalStemVolume() override;
        float GetMicVolumeDecibels(int mic) const override;
        float GetMicVolumeNormalized(int mic) const override;
        bool RaiseMicVolume(int mic) override;
        bool LowerMicVolume(int mic) override;
        bool IsGameplayFlagEnabled() const override;
        void SetGameplayFlagEnabled(bool enabled) override;
        bool ToggleGameplayFlag() override;

        // Field name not in the reference map.
        LocalBandUser* mUser;
    };

    LocalBandUser();  // 0x92AB60
    // Slots 0-1: 0x92B740, 0x92B7E0.
    ~LocalBandUser() override;
    // Slots 6-7: both load the options and preferences from the user's
    // profile (or restore the user's own defaults without one) and send the
    // preferences to the session. Not reconstructed: they read the profile
    // manager and the preferences through calls not declared yet.
    void _OnProfileLoaded() override;  // 0x92B440
    void _OnPlatformUserIdChanged() override;  // 0x92B640
    // Slot 12. Inline; the copy kept is at 0x92B8A0.
    bool IsRemoteJoined() const override {
        return false;
    }
    // Slot 18: also restores the preference defaults.
    void _Reset() override;  // 0x92AC10
    // Slot 19: the session view during an online session, else the
    // profile's options or the user's own. Not reconstructed.
    IGameplayOptions* GetGameplayOptions() override;  // 0x92ADB0
    // Slot 21: the profile manager's profile of the user's slot; without a
    // slot and with `any`, the profile of the user's pad.
    RBProfile* GetProfile(bool any) const override;  // 0x92AD10
    // Slot 24. Inline; the copy kept is at 0x92B8C0.
    bool IsLocal() const override {
        return true;
    }
    // Slots 25-26, 29, 31 and 33: the profile's preferences, or the user's
    // own copies. Not reconstructed.
    Symbol GetPreferenceSymbol() const override;  // 0x92AE90
    long GetPreferenceIndex() const override;  // 0x92AF00
    // Slots 27-28, 30, 32, 34 and 38: a change is stored, saved to the
    // profile and sent to the session.
    void SetPreferenceSymbol(Symbol value) override;  // 0x92AF70
    void SetPreferenceIndex(long value) override;  // 0x92B070
    bool GetPreferenceFlag0() const override;  // 0x92B0B0
    void SetPreferenceFlag0(bool value) override;  // 0x92B120
    bool GetPreferenceFlag2() const override;  // 0x92B170
    // Turning the flag on also picks a symbol and forgets the picked index.
    void SetPreferenceFlag2(bool value) override;  // 0x92B1E0
    bool GetPreferenceFlag3() const override;  // 0x92B240
    // Turning the flag on also forgets the picked index.
    void SetPreferenceFlag3(bool value) override;  // 0x92B2B0
    // Slots 35-36.
    Symbol GetPickedSymbol() const override;  // 0x92B310
    long GetPickedIndex() const override;  // 0x92B320
    // Slot 37: picks the symbol from a list owned by the global at
    // 0x1B44F60 (0xF64660) and forgets the picked index. Not
    // reconstructed.
    void PickSymbol() override;  // 0x92B330
    void SetPickedIndex(long value) override;  // 0x92B370
    // Slots 39-41: the session calls. Not reconstructed: they call into the
    // SOMP session manager through functions not declared yet.
    void _UpdateSompMember() override;  // 0x92B3B0
    void _SendSompPreferences() override;  // 0x92B3E0
    bool IsSompLagging() const override;  // 0x92ACD0
    // Slot 42.
    bool IsSompIdle() const override;  // 0x92AD00

    // The profile's first-time-user events, else the user's own. The map's
    // BandUser::GetFTUEData(); not reconstructed.
    FTUEPersistentData* GetFTUEData();  // 0x92AC70
    // GetGameplayOptions without the session view. The map's
    // BandUser::GetGameplayOptions() const; not reconstructed.
    IGameplayOptions* GetGameplayOptions() const;  // 0x92AE30
    // Writes the preferences to the slot's profile. Name not in the
    // reference map; not reconstructed.
    void _SavePreferences();  // 0x92AFB0
    // Restores the user's own options and preference defaults. Name not in
    // the reference map.
    void _ResetPreferences();  // 0x92B540
    // Copies the profile's options and preferences. Name not in the
    // reference map; not reconstructed.
    void _LoadPreferences(RBProfile* profile);  // 0x92B5B0

    // Field names are not in the reference map. The options and events
    // used while the user has no profile.
    RBGameplayOptions mOptions;
    SessionGameplayOptions mSessionOptions;
    FTUEPersistentData mFTUEData;
    // The preferences (RBUserPreferencesPersistentData's values) used while
    // the user has no profile.
    Symbol mPreferenceSymbol;
    long mPreferenceIndex;
    bool mPreferenceFlags[4];
    Symbol mPickedSymbol;
    long mPickedIndex;
};

static_assert(sizeof(LocalBandUser::SessionGameplayOptions) == 16);
static_assert(offsetof(LocalBandUser, mOptions) == 232);
static_assert(offsetof(LocalBandUser, mSessionOptions) == 440);
static_assert(offsetof(LocalBandUser, mFTUEData) == 456);
static_assert(offsetof(LocalBandUser, mPreferenceSymbol) == 512);
static_assert(offsetof(LocalBandUser, mPreferenceIndex) == 520);
static_assert(offsetof(LocalBandUser, mPreferenceFlags) == 528);
static_assert(offsetof(LocalBandUser, mPickedSymbol) == 536);
static_assert(offsetof(LocalBandUser, mPickedIndex) == 544);
static_assert(sizeof(LocalBandUser) == 552);
