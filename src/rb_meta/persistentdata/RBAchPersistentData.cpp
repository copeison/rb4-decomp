// rb_meta/RBAchPersistentData.o (0xF07280 to 0xF073DB). In this build the
// object follows RBAchUtl.o instead of preceding RBAchPersistentData_PS4.o.
// The interface's empty PushPendingAchievements is emitted here (0xF07390),
// and the static initializer at 0xF073A0 sets the shared header globals
// every rb_meta object initializes; it is not modelled.
#include "rb_meta/persistentdata/RBAchPersistentData.h"

#include "rb_game/users/BandUserMgr.h"
#include "rb_meta/profiles/RBProfile.h"
#include "rb_meta/profiles/RBProfileMgr.h"

// Reconstructed from eboot.elf at 0xF07280.
RBAchPersistentData::RBAchPersistentData(RBProfile* profile) : mProfile(profile) {
    mParent = profile;
}

// Reconstructed from eboot.elf at 0xF07370. The deleting destructor is at
// 0xF07380.
RBAchPersistentData::~RBAchPersistentData() {}

// Reconstructed from eboot.elf at 0xF072B0.
bool RBAchPersistentData::_IsAutokickPlayer() const {
    if (theRBProfileMgr == nullptr || theRBProfileMgr->GetProfileFromSlotIdx(1) != mProfile) {
        return false;
    }
    return theRBProfileMgr->mSystemOptions.mAutokick;
}

// Reconstructed from eboot.elf at 0xF07300. Without a profile manager it
// still reads the options once a user holds slot 1.
bool RBAchPersistentData::_IsAutokickInBand() const {
    if (theRBProfileMgr != nullptr && theRBProfileMgr->GetProfileFromSlotIdx(1) == mProfile
        && theRBProfileMgr->mSystemOptions.mAutokick) {
        return true;
    }
    if (theBandUserMgr->GetUserFromSlot(1) == nullptr) {
        return false;
    }
    return theRBProfileMgr->mSystemOptions.mAutokick;
}
