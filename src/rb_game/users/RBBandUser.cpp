#include "rb_game/users/RBBandUser.h"

#include "audio/fmod/system/FmodPlatform.h"
#include "os/locale/Locale.h"
#include "rb_game/options/GameplayOptions.h"
#include "rb_meta/profiles/RBProfile.h"
#include "utl/text/MakeString.h"

// Reconstructed from eboot.elf at 0x9A9A00.
RBBandUser::RBBandUser()
    : mPreviewCharType(kCharTypeNone),
      mCurrentCharType(kCharTypeNone),
      mCharUsage(0),
      mCharUsageFlags(),
      mCharLoadState(-1),
      mCharFlags(),
      mSompHost(false),
      mSompLeader(false),
      mSompReady(false),
      mCharIds{-1, -1, -1, -1} {}

// Reconstructed from eboot.elf at 0x9A9AE0.
int RBBandUser::GetPlatformUserId() const {
    const RBProfile* const profile = GetProfile(false);
    if (profile != nullptr) {
        return profile->mPadNum;
    }
    return mPlatformUserId;
}

// Reconstructed from eboot.elf at 0x9A9B10.
void RBBandUser::_Reset() {
    mPreviewCharName = Symbol();
    mPreviewCharType = kCharTypeNone;
    const Symbol none;
    const CharType noType = kCharTypeNone;
    SetCurrentCharName(none, noType, nullptr);
    mCharLists[0].clear();
    mCharLists[1].clear();
    mCharLoadState = -1;
    mSompReady = false;
    mCharFlags[0] = false;
    mCharFlags[1] = false;
    mSompHost = false;
    mSompLeader = false;
}

// Reconstructed from eboot.elf at 0x9A9CA0.
void RBBandUser::_ApplyTrackVolumeSettings() {
    if (GetTrackType() == kTrackGuitar) {
        FmodBusInterface* const bus = gFmodBusInterface;
        bus->SetVcaVolumeDb("vca:/IGS", 2, GetGameplayOptions()->GetIGSVolumeDecibels(), 0.0f);
    }
}

// Reconstructed from eboot.elf at 0x9A9F50.
bool RBBandUser::IsUsingOtherController() const {
    if (!IsLocal()) {
        return false;
    }
    return ConnectedControllerType() != GetControllerType();
}

// Reconstructed from eboot.elf at 0x9A9FD0.
const char* RBBandUser::GetPlatformName() const {
    const RBProfile* const profile = GetProfile(false);
    if (profile != nullptr) {
        return profile->GetPlatformName();
    }
    static Symbol guest;
    if (guest == Symbol()) {
        guest = Symbol("guest");
    }
    const char* const localized = Localize(guest, nullptr, 0, TheLocale);
    FormatString name("%s");
    name << localized;
    return name.Str();
}

// Reconstructed from eboot.elf at 0x9ABEB0.
Symbol RBBandUser::GetSompPreferenceSymbol() const {
    if (!GetPreferenceFlag0()) {
        return Symbol();
    }
    if (GetPreferenceFlag2()) {
        return GetPickedSymbol();
    }
    return GetPreferenceSymbol();
}

// Reconstructed from eboot.elf at 0x9ABF00.
long RBBandUser::GetSompPreferenceIndex() const {
    if (!GetPreferenceFlag0()) {
        return -1;
    }
    if (GetPreferenceFlag2() || GetPreferenceFlag3()) {
        return GetPickedIndex();
    }
    return GetPreferenceIndex();
}

// Reconstructed from eboot.elf at 0x9ABF70.
RBBandUser::~RBBandUser() {}
