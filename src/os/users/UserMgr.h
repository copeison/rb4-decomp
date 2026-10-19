#pragma once

#include <cstddef>

#include "os/threading/CritSec.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"

class DataArray;
class User;
class UserGuid;
class UserSignedOutMsg;

// The application's users (os/UserMgr.o, 0x36AD30-0x36B865). It clears the
// platform user of the users who sign out and tells the users of a pad
// when a profile load for the pad ends. The game derives its band user
// manager from it and keeps the users. Its exports take its own lock. The
// vtable at 0x18FB530 has ten slots; the object is 128 bytes.
class UserMgr : public SyncMsgSource {
public:
    // Sets the "user_mgr" global and subscribes to the platform's
    // "user_signed_out".
    UserMgr();  // 0x36AD40
    // Slots 0-1: 0x36AF20, 0x36B020.
    ~UserMgr() override;
    // Slot 2: "user_signed_out", "get_user_from_pad_num" and "load_end".
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x36B520
    // Slot 7: appends every user.
    virtual void GetUsers(eastl::vector<User*>& users) const = 0;
    // Slot 8: the user with the id, or null; `fail` asks for a missing user
    // to be reported.
    virtual User* GetUser(const UserGuid& guid, bool fail) const = 0;
    // Slot 9: a profile load for the pad ended; unless it failed with an
    // error other than 2, tells the pad's users. Name not in the reference
    // map.
    virtual void OnLoadEnd(int error, int padNum);  // 0x36B490

    // Subscribes to the save manager's "load_end". The application calls it
    // after the profile manager's PostInit. Name not in the reference map;
    // the pairing of these two identical bodies follows ProfileMgr's
    // PostInit pair, and the evidence is weak.
    static void PostInit(UserMgr* mgr);  // 0x36B040
    void PostInit();  // 0x36B110

    User* GetUserFromPadNum(int padNum) const;  // 0x36B1E0
    // The users whose platform user id is the one given. The map has
    // GetUsersFromPlatformUserId(int) const; this build returns the list.
    eastl::vector<User*> GetUsersFromPlatformUserId(int platformUserId) const;  // 0x36B1F0
    // Clears the platform user of the signed-out user's users.
    DataNode OnMsg(const UserSignedOutMsg& msg);  // 0x36B3C0

    // Field names are not in the reference map.
    CritSec mCritSec;
    MsgSource::EventSinkElem mUserSignedOutSink;
    MsgSource::EventSinkElem mLoadEndSink;
};

static_assert(offsetof(UserMgr, mCritSec) == 32);
static_assert(offsetof(UserMgr, mUserSignedOutSink) == 48);
static_assert(offsetof(UserMgr, mLoadEndSink) == 88);
static_assert(sizeof(UserMgr) == 128);

// Sets the user manager and reassigns the users' pads.
void SetTheUserMgr(UserMgr* mgr);  // 0x36AD30

// The user manager. At 0x19FE748; the map places it in os/Joypad_PS4.o.
extern UserMgr* TheUserMgr;
