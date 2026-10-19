#pragma once

#include <cstddef>

#include "os/users/UserGuid.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"

class DataArray;

// A player of the application, identified by its UserGuid (os/User.o,
// 0x3A8050-0x3A8420). It answers "get_player_name" and "comes_before". The
// game derives its band users from it. The vtable at 0x18FC990 has eight
// slots; the object is 32 bytes.
class User : public MsgSink {
public:
    User();  // 0x3A8050
    // Slots 0-1: 0x3A83D0, 0x3A83E0.
    ~User() override;
    // Slot 2: "get_player_name" gives UserName; "comes_before" whether the
    // user's id orders before the other user's.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x3A81F0
    // Slot 4: the platform's user id, or -1 when it has none. The map has
    // BandUser::GetPlatformUserId() const; this build moves it into User.
    virtual int GetPlatformUserId() const;  // 0x3A83F0
    // Slot 5: whether a joypad is connected on the user's pad.
    virtual bool IsJoypadConnected() const;  // 0x3A80F0
    // Slot 6: a hook for the user's profile having loaded; empty here. Name
    // not in the reference map; the evidence is weak.
    virtual void _OnProfileLoaded();  // 0x3A8400
    // Slot 7: a hook for the platform user id having changed; empty here.
    // Name not in the reference map.
    virtual void _OnPlatformUserIdChanged();  // 0x3A8410

    bool operator==(const User& other) const;  // 0x3A8090
    bool operator<(const User& other) const;  // 0x3A80A0
    void SetUserGuid(const UserGuid& guid);  // 0x3A80B0
    const UserGuid& GetUserGuid() const {
        return mUserGuid;
    }
    int GetPadNum() const;  // 0x3A80C0
    // Whether the platform reports the user's platform id signed in.
    bool IsSignedIn() const;  // 0x3A80D0
    // The platform's name for the user, or "Player <pad>" without a
    // platform id.
    const char* UserName() const;  // 0x3A8110
    // Records the platform user id and calls _OnPlatformUserIdChanged the
    // first time and whenever it changes. Name not in the reference map.
    void SetPlatformUserId(int platformUserId);  // 0x3A81C0
    // Calls _OnProfileLoaded. The user manager calls it when a profile load
    // for the user's pad ends. Name not in the reference map.
    void OnProfileLoaded();  // 0x3A81E0

    // Field names are not in the reference map.
    UserGuid mUserGuid;
    int mPlatformUserId;
    // Set by the first SetPlatformUserId.
    bool mPlatformUserIdSet;
};

static_assert(offsetof(User, mUserGuid) == 8);
static_assert(offsetof(User, mPlatformUserId) == 24);
static_assert(offsetof(User, mPlatformUserIdSet) == 28);
static_assert(sizeof(User) == 32);
