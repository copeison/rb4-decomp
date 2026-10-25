#include "rb_game/users/LocalBandUser.h"

#include "rb_meta/profiles/RBProfile.h"
#include "rb_meta/profiles/RBProfileMgr.h"

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

// Reconstructed from eboot.elf at 0x92AF70.
void LocalBandUser::SetPreferenceSymbol(Symbol value) {
    if (GetPreferenceSymbol() == value) {
        return;
    }
    mPreferenceSymbol = value;
    _SavePreferences();
    _SendSompPreferences();
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

// Reconstructed from eboot.elf at 0x92B120.
void LocalBandUser::SetPreferenceFlag0(bool value) {
    if (GetPreferenceFlag0() == value) {
        return;
    }
    mPreferenceFlags[0] = value;
    _SavePreferences();
    _SendSompPreferences();
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

// Reconstructed from eboot.elf at 0x92B740. The options' destructor only
// destroys their second base (0x92C580).
LocalBandUser::~LocalBandUser() {}
