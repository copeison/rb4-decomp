// rb_meta/RBAchPersistentData.o (0xF07280 to 0xF073DB). In this build the
// object follows RBAchUtl.o instead of preceding RBAchPersistentData_PS4.o.
// The interface's empty PushPendingAchievements is emitted here (0xF07390),
// and the static initializer at 0xF073A0 sets the shared header globals
// every rb_meta object initializes; it is not modelled.
//
// Not reconstructed: _IsAutokickInBand (0xF07300) also asks
// theBandUserMgr for the user of band slot 1, which BandUserMgr.h does not
// declare yet.
#include "rb_meta/persistentdata/RBAchPersistentData.h"

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
