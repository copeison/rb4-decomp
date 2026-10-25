#include "rb_game/users/LocalBandUser.h"

#include "rb_meta/profiles/RBProfile.h"
#include "rb_meta/profiles/RBProfileMgr.h"
#include "rb_meta/somp/RBSompSessionMgr.h"

// Reconstructed from eboot.elf at 0x92AB60. The preferences start with the
// first two flags on.
LocalBandUser::LocalBandUser()
    : mOptions(nullptr),
      mSessionOptions(this),
      mFTUEData(nullptr),
      mPreferenceIndex(-1),
      mPreferenceFlags{true, true, false, false},
      mPickedIndex(-1) {}

// Reconstructed from eboot.elf at 0x92AC10.
void LocalBandUser::_Reset() {
    RBBandUser::_Reset();
    mPreferenceSymbol = Symbol();
    mPreferenceIndex = -1;
    mPreferenceFlags[0] = true;
    mPreferenceFlags[1] = true;
    mPreferenceFlags[2] = false;
    mPreferenceFlags[3] = false;
    mPickedSymbol = Symbol();
    mPickedIndex = -1;
}

// Reconstructed from eboot.elf at 0x92AC70.
FTUEPersistentData* LocalBandUser::GetFTUEData() {
    RBProfile* const profile = GetProfile(false);
    if (profile != nullptr) {
        const unsigned long slot = GetSlot();
        if (slot != static_cast<unsigned long>(-1) && theRBProfileMgr->SlotOwnsProfileSettings(slot)) {
            return &profile->mFTUEData;
        }
    }
    return &mFTUEData;
}

// Reconstructed from eboot.elf at 0x92AD00.
bool LocalBandUser::IsSompIdle() const {
    return false;
}

// Reconstructed from eboot.elf at 0x92AD10.
RBProfile* LocalBandUser::GetProfile(bool any) const {
    const unsigned long slot = GetSlot();
    if (slot != static_cast<unsigned long>(-1)) {
        return theRBProfileMgr->GetProfileFromSlotIdx(slot);
    }
    if (!any || theRBProfileMgr == nullptr) {
        return nullptr;
    }
    const int padNum = GetPlatformUserId();
    for (Profile* const profile : theRBProfileMgr->mProfiles) {
        if (profile->mPadNum == padNum) {
            return static_cast<RBProfile*>(profile);
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x92ADB0. GetGameplayOptions() const is
// inlined.
IGameplayOptions* LocalBandUser::GetGameplayOptions() {
    if (theRBSompSessionMgr.mClient != nullptr) {
        return &mSessionOptions;
    }
    const LocalBandUser* const user = this;
    return user->GetGameplayOptions();
}

// Reconstructed from eboot.elf at 0x92AE30.
IGameplayOptions* LocalBandUser::GetGameplayOptions() const {
    RBProfile* const profile = GetProfile(false);
    if (profile != nullptr) {
        const unsigned long slot = GetSlot();
        if (slot != static_cast<unsigned long>(-1) && theRBProfileMgr->SlotOwnsProfileSettings(slot)) {
            return &profile->mGameplayOptions;
        }
    }
    return const_cast<RBGameplayOptions*>(&mOptions);
}

// Reconstructed from eboot.elf at 0x92AE90.
Symbol LocalBandUser::GetPreferenceSymbol() const {
    RBProfile* const profile = GetProfile(false);
    if (profile != nullptr && theRBProfileMgr->SlotOwnsProfileSettings(GetSlot())) {
        return profile->mUserPreferences.GetSymbolValue();
    }
    return mPreferenceSymbol;
}

// Reconstructed from eboot.elf at 0x92AF00.
long LocalBandUser::GetPreferenceIndex() const {
    RBProfile* const profile = GetProfile(false);
    if (profile != nullptr && theRBProfileMgr->SlotOwnsProfileSettings(GetSlot())) {
        return static_cast<long>(profile->mUserPreferences.GetLongValue());
    }
    return mPreferenceIndex;
}

// Reconstructed from eboot.elf at 0x92AF70.
void LocalBandUser::SetPreferenceSymbol(Symbol value) {
    if (GetPreferenceSymbol() == value) {
        return;
    }
    mPreferenceSymbol = value;
    _SavePreferences();
    _SendSompPreferences();
}

// Reconstructed from eboot.elf at 0x92AFB0.
void LocalBandUser::_SavePreferences() {
    RBProfile* const profile = GetProfile(false);
    if (profile == nullptr || !theRBProfileMgr->SlotOwnsProfileSettings(GetSlot())) {
        return;
    }
    RBUserPreferencesPersistentData& preferences = profile->mUserPreferences;
    preferences.SetFirstFlag(mPreferenceFlags[0]);
    preferences.SetSecondFlag(mPreferenceFlags[1]);
    preferences.SetSymbolValue(mPreferenceSymbol);
    preferences.SetLongValue(static_cast<unsigned long>(mPreferenceIndex));
    preferences.SetThirdFlag(mPreferenceFlags[2]);
    preferences.SetFourthFlag(mPreferenceFlags[3]);
}

// Reconstructed from eboot.elf at 0x92B070.
void LocalBandUser::SetPreferenceIndex(long value) {
    if (GetPreferenceIndex() == value) {
        return;
    }
    mPreferenceIndex = value;
    _SavePreferences();
    _SendSompPreferences();
}

// Reconstructed from eboot.elf at 0x92B0B0.
bool LocalBandUser::GetPreferenceFlag0() const {
    RBProfile* const profile = GetProfile(false);
    if (profile != nullptr && theRBProfileMgr->SlotOwnsProfileSettings(GetSlot())) {
        return profile->mUserPreferences.IsFirstFlagSet();
    }
    return mPreferenceFlags[0];
}

// Reconstructed from eboot.elf at 0x92B120.
void LocalBandUser::SetPreferenceFlag0(bool value) {
    if (GetPreferenceFlag0() == value) {
        return;
    }
    mPreferenceFlags[0] = value;
    _SavePreferences();
    _SendSompPreferences();
}

// Reconstructed from eboot.elf at 0x92B170.
bool LocalBandUser::GetPreferenceFlag2() const {
    RBProfile* const profile = GetProfile(false);
    if (profile != nullptr && theRBProfileMgr->SlotOwnsProfileSettings(GetSlot())) {
        return profile->mUserPreferences.IsThirdFlagSet();
    }
    return mPreferenceFlags[2];
}

// Reconstructed from eboot.elf at 0x92B1E0.
void LocalBandUser::SetPreferenceFlag2(bool value) {
    if (GetPreferenceFlag2() == value) {
        return;
    }
    mPreferenceFlags[2] = value;
    if (value) {
        PickSymbol();
        mPickedIndex = -1;
    }
    _SavePreferences();
    _SendSompPreferences();
}

// Reconstructed from eboot.elf at 0x92B240.
bool LocalBandUser::GetPreferenceFlag3() const {
    RBProfile* const profile = GetProfile(false);
    if (profile != nullptr && theRBProfileMgr->SlotOwnsProfileSettings(GetSlot())) {
        return profile->mUserPreferences.IsFourthFlagSet();
    }
    return mPreferenceFlags[3];
}

// Reconstructed from eboot.elf at 0x92B2B0.
void LocalBandUser::SetPreferenceFlag3(bool value) {
    if (GetPreferenceFlag3() == value) {
        return;
    }
    mPreferenceFlags[3] = value;
    if (value) {
        mPickedIndex = -1;
    }
    _SavePreferences();
    _SendSompPreferences();
}

// Reconstructed from eboot.elf at 0x92B310.
Symbol LocalBandUser::GetPickedSymbol() const {
    return mPickedSymbol;
}

// Reconstructed from eboot.elf at 0x92B320.
long LocalBandUser::GetPickedIndex() const {
    return mPickedIndex;
}

// Reconstructed from eboot.elf at 0x92B370.
void LocalBandUser::SetPickedIndex(long value) {
    if (GetPickedIndex() == value) {
        return;
    }
    mPickedIndex = value;
    _SavePreferences();
    _SendSompPreferences();
}

// Reconstructed from eboot.elf at 0x92B440. _LoadPreferences and
// _ResetPreferences are inlined.
void LocalBandUser::_OnProfileLoaded() {
    RBProfile* const profile = GetProfile(true);
    if (profile != nullptr) {
        _LoadPreferences(profile);
    } else {
        _ResetPreferences();
    }
    _SendSompPreferences();
}

// Reconstructed from eboot.elf at 0x92B540.
void LocalBandUser::_ResetPreferences() {
    mOptions.Clear();
    mOptions.MakeClean();
    mPreferenceSymbol = Symbol();
    mPreferenceIndex = -1;
    mPreferenceFlags[0] = true;
    mPreferenceFlags[1] = true;
    mPreferenceFlags[2] = false;
    mPreferenceFlags[3] = false;
    mPickedSymbol = Symbol();
    mPickedIndex = -1;
}

// Reconstructed from eboot.elf at 0x92B5B0.
void LocalBandUser::_LoadPreferences(RBProfile* profile) {
    mOptions.SetOptions(profile->mGameplayOptions);
    RBUserPreferencesPersistentData& preferences = profile->mUserPreferences;
    mPreferenceFlags[0] = preferences.IsFirstFlagSet();
    mPreferenceFlags[1] = preferences.IsSecondFlagSet();
    mPreferenceSymbol = preferences.GetSymbolValue();
    mPreferenceIndex = static_cast<long>(preferences.GetLongValue());
    mPreferenceFlags[2] = preferences.IsThirdFlagSet();
    mPreferenceFlags[3] = preferences.IsFourthFlagSet();
}

// Reconstructed from eboot.elf at 0x92B640. _LoadPreferences and
// _ResetPreferences are inlined.
void LocalBandUser::_OnPlatformUserIdChanged() {
    RBProfile* const profile = GetProfile(true);
    if (profile != nullptr) {
        _LoadPreferences(profile);
    } else {
        _ResetPreferences();
    }
    _SendSompPreferences();
}

// Reconstructed from eboot.elf at 0x92B740. The options' destructor only
// destroys their second base (0x92C580).
LocalBandUser::~LocalBandUser() {}

// Reconstructed from eboot.elf at 0x92B8D0.
void LocalBandUser::SessionGameplayOptions::Clear() {
    const LocalBandUser* const user = mUser;
    user->GetGameplayOptions()->Clear();
}

// Reconstructed from eboot.elf at 0x92B940.
bool LocalBandUser::SessionGameplayOptions::IsLefty(ControllerType type) const {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->IsLefty(type);
}

// Reconstructed from eboot.elf at 0x92B9C0.
void LocalBandUser::SessionGameplayOptions::SetLefty(ControllerType type, bool lefty) {
    const LocalBandUser* const user = mUser;
    user->GetGameplayOptions()->SetLefty(type, lefty);
}

// Reconstructed from eboot.elf at 0x92BA40.
int LocalBandUser::SessionGameplayOptions::GetGuitarSoloMode() const {
    return 1;
}

// Reconstructed from eboot.elf at 0x92BA50.
void LocalBandUser::SessionGameplayOptions::SetGuitarSoloMode(int mode) {
    static_cast<void>(mode);
}

// Reconstructed from eboot.elf at 0x92BA60.
int LocalBandUser::SessionGameplayOptions::ToggleGuitarSoloMode() {
    return 1;
}

// Reconstructed from eboot.elf at 0x92BA70.
int LocalBandUser::SessionGameplayOptions::GetDrumFillLogic() const {
    return 1;
}

// Reconstructed from eboot.elf at 0x92BA80.
void LocalBandUser::SessionGameplayOptions::SetDrumFillLogic(int logic) {
    static_cast<void>(logic);
}

// Reconstructed from eboot.elf at 0x92BA90.
bool LocalBandUser::SessionGameplayOptions::IsVocalImprovEnabled() const {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->IsVocalImprovEnabled();
}

// Reconstructed from eboot.elf at 0x92BB00.
void LocalBandUser::SessionGameplayOptions::SetVocalImprovEnabled(bool enabled) {
    const LocalBandUser* const user = mUser;
    user->GetGameplayOptions()->SetVocalImprovEnabled(enabled);
}

// Reconstructed from eboot.elf at 0x92BB80.
unsigned int LocalBandUser::SessionGameplayOptions::GetBreakneckSpeedModifierIndex() const {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->GetBreakneckSpeedModifierIndex();
}

// Reconstructed from eboot.elf at 0x92BBF0.
bool LocalBandUser::SessionGameplayOptions::IsBreakneckSpeedEnabled() const {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->IsBreakneckSpeedEnabled();
}

// Reconstructed from eboot.elf at 0x92BC60.
float LocalBandUser::SessionGameplayOptions::GetBreakneckSpeedModifier() const {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->GetBreakneckSpeedModifier();
}

// Reconstructed from eboot.elf at 0x92BCD0.
float LocalBandUser::SessionGameplayOptions::GetBreakneckSpeedModifierNormalized() const {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->GetBreakneckSpeedModifierNormalized();
}

// Reconstructed from eboot.elf at 0x92BD40.
void LocalBandUser::SessionGameplayOptions::SetBreakneckSpeedModifierIndex(unsigned int index) {
    const LocalBandUser* const user = mUser;
    user->GetGameplayOptions()->SetBreakneckSpeedModifierIndex(index);
}

// Reconstructed from eboot.elf at 0x92BDC0.
bool LocalBandUser::SessionGameplayOptions::RaiseBreakneckSpeedModifier() {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->RaiseBreakneckSpeedModifier();
}

// Reconstructed from eboot.elf at 0x92BE30.
bool LocalBandUser::SessionGameplayOptions::LowerBreakneckSpeedModifier() {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->LowerBreakneckSpeedModifier();
}

// Reconstructed from eboot.elf at 0x92BEA0.
float LocalBandUser::SessionGameplayOptions::GetIGSVolumeDecibels() const {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->GetIGSVolumeDecibels();
}

// Reconstructed from eboot.elf at 0x92BF10.
float LocalBandUser::SessionGameplayOptions::GetIGSVolumeNormalized() const {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->GetIGSVolumeNormalized();
}

// Reconstructed from eboot.elf at 0x92BF80.
bool LocalBandUser::SessionGameplayOptions::RaiseIGSVolume() {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->RaiseIGSVolume();
}

// Reconstructed from eboot.elf at 0x92BFF0.
bool LocalBandUser::SessionGameplayOptions::LowerIGSVolume() {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->LowerIGSVolume();
}

// Reconstructed from eboot.elf at 0x92C060.
float LocalBandUser::SessionGameplayOptions::GetVocalStemVolumeDecibels() const {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->GetVocalStemVolumeDecibels();
}

// Reconstructed from eboot.elf at 0x92C0D0.
float LocalBandUser::SessionGameplayOptions::GetVocalStemVolumeNormalized() const {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->GetVocalStemVolumeNormalized();
}

// Reconstructed from eboot.elf at 0x92C140.
bool LocalBandUser::SessionGameplayOptions::RaiseVocalStemVolume() {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->RaiseVocalStemVolume();
}

// Reconstructed from eboot.elf at 0x92C1B0.
bool LocalBandUser::SessionGameplayOptions::LowerVocalStemVolume() {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->LowerVocalStemVolume();
}

// Reconstructed from eboot.elf at 0x92C220.
float LocalBandUser::SessionGameplayOptions::GetMicVolumeDecibels(int mic) const {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->GetMicVolumeDecibels(mic);
}

// Reconstructed from eboot.elf at 0x92C2A0.
float LocalBandUser::SessionGameplayOptions::GetMicVolumeNormalized(int mic) const {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->GetMicVolumeNormalized(mic);
}

// Reconstructed from eboot.elf at 0x92C320.
bool LocalBandUser::SessionGameplayOptions::RaiseMicVolume(int mic) {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->RaiseMicVolume(mic);
}

// Reconstructed from eboot.elf at 0x92C3A0.
bool LocalBandUser::SessionGameplayOptions::LowerMicVolume(int mic) {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->LowerMicVolume(mic);
}

// Reconstructed from eboot.elf at 0x92C420.
bool LocalBandUser::SessionGameplayOptions::IsGameplayFlagEnabled() const {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->IsGameplayFlagEnabled();
}

// Reconstructed from eboot.elf at 0x92C490.
void LocalBandUser::SessionGameplayOptions::SetGameplayFlagEnabled(bool enabled) {
    const LocalBandUser* const user = mUser;
    user->GetGameplayOptions()->SetGameplayFlagEnabled(enabled);
}

// Reconstructed from eboot.elf at 0x92C510.
bool LocalBandUser::SessionGameplayOptions::ToggleGameplayFlag() {
    const LocalBandUser* const user = mUser;
    return user->GetGameplayOptions()->ToggleGameplayFlag();
}
