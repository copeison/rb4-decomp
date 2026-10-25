#include "rb_game/users/RBBandUserMgr.h"

#include <cstring>

#include "os/debug/Debug.h"
#include "rb_game/users/LocalBandUser.h"
#include "rb_game/users/RemoteBandUser.h"

RBBandUserMgr* TheRBBandUserMgr;

// Reconstructed from eboot.elf at 0x923190.
void RBBandUserMgrTerminate() {
    delete theBandUserMgr;
    theBandUserMgr = nullptr;
}

// Reconstructed from eboot.elf at 0x9231C0.
void RBBandUserMgrInit() {
    RBBandUserMgr* const mgr = new RBBandUserMgr();
    TheRBBandUserMgr = mgr;
    theBandUserMgr = mgr;
    mgr->Init();
    SetTheUserMgr(theBandUserMgr);
    TheDebug.AddExitCallback(RBBandUserMgrTerminate);
}

// Reconstructed from eboot.elf at 0x923300.
bool RBBandUserMgr::HasProfileAndController(User* user) {
    if (user == nullptr) {
        return false;
    }
    BandUser* const bandUser = GetBandUser(user);
    if (bandUser == nullptr) {
        return false;
    }
    const RBProfile* const profile = bandUser->GetProfile(false);
    const ControllerType controller = bandUser->GetControllerType();
    return profile != nullptr && controller != kControllerNone;
}

// Reconstructed from eboot.elf at 0x923350. The binary copies the leader's
// member id from the null id at 0x1ADFAE0, a 64-bit -1 and a 16-bit -1.
RBBandUserMgr::RBBandUserMgr() {
    memset(mSompLeaderMemberId, 0xFF, 10);
    memset(mSompLeaderMemberId + 10, 0, sizeof(mSompLeaderMemberId) - 10);
}

// Reconstructed from eboot.elf at 0x923420.
RBBandUserMgr::~RBBandUserMgr() {
    TheRBBandUserMgr = nullptr;
}

// Reconstructed from eboot.elf at 0x923520.
void RBBandUserMgr::_CreateUsers() {
    mUsers.reserve(11);
    mLocalUsers.reserve(8);
    for (int i = 0; i < 8; i++) {
        LocalBandUser* const user = new LocalBandUser();
        mUsers.push_back(user);
        mLocalUsers.push_back(user);
    }
    mRemoteUsers.reserve(3);
    for (int i = 0; i < 3; i++) {
        RemoteBandUser* const user = new RemoteBandUser();
        mUsers.push_back(user);
        mRemoteUsers.push_back(user);
    }
}

// Reconstructed from eboot.elf at 0x9249B0.
RBBandUser* RBBandUserMgr::GetRBBandUserFromSlot(unsigned long slot) const {
    return static_cast<RBBandUser*>(GetUserFromSlot(slot));
}

// Reconstructed from eboot.elf at 0x9249C0.
RemoteBandUser* RBBandUserMgr::GetRemoteUserFromSlot(unsigned long slot) const {
    for (RemoteBandUser* const user : mRemoteUsers) {
        if (user->GetSlot() == slot) {
            return user;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x924A20.
RBBandUser* RBBandUserMgr::GetLocalUserFromSlot(unsigned long slot) const {
    RBBandUser* const user = static_cast<RBBandUser*>(GetUserFromSlot(slot));
    if (user == nullptr) {
        return nullptr;
    }
    return user->IsLocal() ? user : nullptr;
}
