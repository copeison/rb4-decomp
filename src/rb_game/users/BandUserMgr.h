#pragma once

#include <cstddef>

#include "os/users/UserGuid.h"
#include "os/users/UserMgr.h"
#include "rb_beatmatch/tracks/TrackType.h"
#include "utl/containers/Vector.h"

class BandUser;

// A RakNet replica of the band's four slot ids (constructor 0xA84AF0,
// vtable 0x1969840; 7624 bytes from operator new). Newer than the map; the
// class name is not in the reference map and the evidence for it is weak.
// Declared only.
class BandSlotsReplica {
public:
    BandSlotsReplica();  // 0xA84AF0
    // Slots 0-1: 0xA8AA00, 0xA8AAA0.
    virtual ~BandSlotsReplica();
};

// A RakNet replica of the band users' data, with its own lock ("hx crit
// sec"; constructor 0xA8F1B0, vtable 0x19699C0; 36936 bytes from operator
// new). Newer than the map; the class name is not in the reference map and
// the evidence for it is weak. Declared only.
class BandUserDataReplica {
public:
    BandUserDataReplica();  // 0xA8F1B0
    virtual ~BandUserDataReplica();
};

// The band's users and their slots (rb_game/BandUserMgr.o,
// 0xA40130-0xA41BBC with the static initializer at 0xA41B80). It owns the
// band users, which its subclass creates (slot 10), keeps the id of the
// user in each of the four band slots, exporting "user_slot_set" and
// "user_slot_cleared", and lists the users by the flags below. The vtable
// at 0x1968568 has 11 slots (slot 10 pure); the object is 256 bytes. The
// map's ForEachUser, Handle, the FTUE and overshell queries and the debug
// controller overrides are not in this build's object. Its static
// initializer only builds statics of included headers that nothing in the
// object reads (0x1AEF588-0x1AEF5D0); it is not reconstructed.
class BandUserMgr : public UserMgr {
public:
    // The filters of _GetBandUsers and CountBandUsers. Names not in the
    // reference map.
    enum BandUserFlags {
        // Users who do not take part.
        kBandUsersNotParticipating = 0x4,
        // Users who take part.
        kBandUsersParticipating = 0x8,
        // Excludes every user: the test it stood for is constant in this
        // build. The evidence for the name is weak.
        kBandUsersExcluded = 0x10,
        // Users signed into the platform.
        kBandUsersSignedIn = 0x800,
        // Users the platform's user service lists as logged in.
        kBandUsersLoggedIn = 0x1000,
        // Users the online session marks as its leader
        // (BandUser::IsSompLeader). The evidence for the name is weak.
        kBandUsersSompLeader = 0x2000,
    };

    // Sets no globals; the subclass's creator sets theBandUserMgr.
    BandUserMgr();  // 0xA40130
    // Slots 0-1: 0xA401F0, 0xA402B0. Clears theBandUserMgr and deletes the
    // users and both replicas.
    ~BandUserMgr() override;
    // Slot 7.
    void GetUsers(eastl::vector<User*>& users) const override;  // 0xA40500
    // Slot 8: the band user with the id, or null.
    User* GetUser(const UserGuid& guid, bool fail) const override;  // 0xA403D0
    // Slot 10: creates the band users. Name not in the reference map.
    virtual void _CreateUsers() = 0;

    // Creates the users and empties the slots. The creator calls it once
    // the object is built. Name not in the reference map.
    void Init();  // 0xA40380
    // The band user with the id, or null; `fail` asks for a missing user to
    // be reported.
    BandUser* GetBandUser(const UserGuid& guid, bool fail) const;  // 0xA40460
    // Every user is a band user.
    static BandUser* GetBandUser(User* user);  // 0xA404F0
    void ResetSlots();  // 0xA40640
    // The slot holding the id, or -1.
    unsigned long GetSlot(const UserGuid& guid) const;  // 0xA40690
    // Puts the user in the slot and exports "user_slot_set" (slot, user);
    // slot -1 takes the user out of its slot and exports "user_slot_cleared"
    // (slot, user).
    void SetSlot(const BandUser* user, unsigned long slot);  // 0xA40720
    void ClearSlot(BandUser* user);  // 0xA40C30
    const UserGuid& GetUserGuidFromSlot(unsigned long slot) const;  // 0xA40C40
    // The band user in the slot, or null.
    BandUser* GetUserFromSlot(unsigned long slot) const;  // 0xA40C50
    // The participating user who plays the track, or null.
    BandUser* GetUserFromTrackType(TrackType type) const;  // 0xA40CF0
    // The participating band users. The map has the no-argument
    // GetBandUsers(); this build returns the list.
    eastl::vector<BandUser*> GetBandUsers() const;  // 0xA40DC0
    // The highest occupied slot, or -1.
    int MaxSlot() const;  // 0xA40DE0
    unsigned long GetNumParticipants() const;  // 0xA40E60
    // Whether more than one user takes part.
    bool IsMultiplayerGame() const;  // 0xA40EA0
    // The slots whose user exists. Name not in the reference map.
    int GetNumUsersInSlots() const;  // 0xA40EE0
    // The users that pass the BandUserFlags filters. The map has
    // _GetBandUsers(eastl::vector<BandUser*>*, int) const; this build
    // returns the list.
    eastl::vector<BandUser*> _GetBandUsers(int flags) const;  // 0xA41110
    // The number of users that pass the filters.
    int CountBandUsers(int flags) const;  // 0xA41770
    // CountBandUsers(kBandUsersSignedIn), inlined.
    int GetNumSignedInUsers() const;  // 0xA41A60
    // The user on the pad, or null.
    BandUser* GetUserFromPad(int padNum) const;  // 0xA41AD0
    // Whether any user is signed in. Name not in the reference map.
    bool IsAnyUserSignedIn() const;  // 0xA41B30

    // Field names are not in the reference map.
    eastl::vector<BandUser*> mUsers;
    // The id of each band slot's user; null when the slot is empty.
    UserGuid mSlots[4];
    BandSlotsReplica* mSlotsReplica;
    // Neither the constructor nor any reconstructed code writes it.
    void* mSlotsReplicaUnused;
    BandUserDataReplica* mUserDataReplica;
    // Neither the constructor nor any reconstructed code writes it.
    void* mUserDataReplicaUnused;
};

static_assert(offsetof(BandUserMgr, mUsers) == 128);
static_assert(offsetof(BandUserMgr, mSlots) == 160);
static_assert(offsetof(BandUserMgr, mSlotsReplica) == 224);
static_assert(offsetof(BandUserMgr, mUserDataReplica) == 240);
static_assert(sizeof(BandUserMgr) == 256);

// The band user manager. At 0x1AEF5D8.
extern BandUserMgr* theBandUserMgr;
