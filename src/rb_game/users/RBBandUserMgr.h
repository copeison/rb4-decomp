#pragma once

#include <cstddef>

#include "os/users/UserGuid.h"
#include "rb_game/users/BandUserMgr.h"
#include "utl/containers/Vector.h"

class LocalBandUser;
class RBBandUser;
class RemoteBandUser;
class User;

// The game's band user manager (0x923190-0x926610): the BandUserMgr the
// game creates at startup. It creates eight local and three remote users,
// keeps them in separate lists, and follows the online (SOMP) session's
// leader and member flags. Newer than the map; the class name is not in the
// reference map and the evidence for it is weak. The vtable at 0x1960498
// has BandUserMgr's 11 slots; the object is 352 bytes. The session
// updates (0x925680-0x926610) and the user lists with the session filters
// (0x923AF0-0x924970) are not reconstructed.
class RBBandUserMgr : public BandUserMgr {
public:
    RBBandUserMgr();  // 0x923350
    // Slots 0-1: 0x923420, 0x9234A0. Clears TheRBBandUserMgr.
    ~RBBandUserMgr() override;
    // Slot 10: eight local users, then three remote users.
    void _CreateUsers() override;  // 0x923520

    // Whether the user has a profile and a controller. Name not in the
    // reference map.
    static bool HasProfileAndController(User* user);  // 0x923300
    // The band user in the slot. Name not in the reference map.
    RBBandUser* GetRBBandUserFromSlot(unsigned long slot) const;  // 0x9249B0
    // The remote user in the slot, or null. Name not in the reference map.
    RemoteBandUser* GetRemoteUserFromSlot(unsigned long slot) const;  // 0x9249C0
    // The band user in the slot when it is local, or null. Name not in the
    // reference map.
    RBBandUser* GetLocalUserFromSlot(unsigned long slot) const;  // 0x924A20

    // Field names are not in the reference map.
    eastl::vector<LocalBandUser*> mLocalUsers;
    eastl::vector<RemoteBandUser*> mRemoteUsers;
    // The session leader's member id (a 64-bit id and a 16-bit value, both
    // -1 when unset; null id at 0x1ADFAE0) and user id.
    unsigned char mSompLeaderMemberId[16];
    UserGuid mSompLeaderGuid;
};

static_assert(offsetof(RBBandUserMgr, mLocalUsers) == 256);
static_assert(offsetof(RBBandUserMgr, mRemoteUsers) == 288);
static_assert(offsetof(RBBandUserMgr, mSompLeaderMemberId) == 320);
static_assert(offsetof(RBBandUserMgr, mSompLeaderGuid) == 336);
static_assert(sizeof(RBBandUserMgr) == 352);

// Creates the manager, sets theBandUserMgr and TheRBBandUserMgr, creates the
// users, makes it the user manager and registers RBBandUserMgrTerminate as
// an exit callback. Name not in the reference map, whose BandUserMgrInit
// had this role.
void RBBandUserMgrInit();  // 0x9231C0
// Deletes the manager. Name not in the reference map, whose
// BandUserMgrTerminate had this role.
void RBBandUserMgrTerminate();  // 0x923190

// The manager. At 0x1ADFB18. Name not in the reference map.
extern RBBandUserMgr* TheRBBandUserMgr;
