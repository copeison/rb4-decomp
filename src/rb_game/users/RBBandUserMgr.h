#pragma once

#include <cstddef>

#include "net/RakNetTypes.h"
#include "os/users/UserGuid.h"
#include "rb_game/core/Defines.h"
#include "rb_game/users/BandUserMgr.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class FTUEPersistentData;
class LocalBandUser;
class RBBandUser;
class RBProfile;
class RemoteBandUser;
class User;

// One member of the online (SOMP) session's member table: the member's
// RakNet id, the user's id and the member's flags (56 bytes). The session
// manager keeps it (see SompSessionMembers). Names not in the reference map;
// the meanings of the host and ready flags follow BandUser's slots 9 and 11
// and the evidence for them is weak.
struct SompSessionMember {
    RakNet::RakNetGUID mMemberId;
    UserGuid mUserGuid;
    // Not read by the band user code.
    unsigned char mOpaque20[16];
    bool mLeader;
    bool mHost;
    bool mReady;
    // Whether the member plays on this console.
    bool mLocal;
};

static_assert(offsetof(SompSessionMember, mUserGuid) == 16);
static_assert(offsetof(SompSessionMember, mLeader) == 48);
static_assert(offsetof(SompSessionMember, mLocal) == 51);
static_assert(sizeof(SompSessionMember) == 56);

// The session's members as the session manager keeps them: the part of
// theRBSompSessionMgr from offset 0x180, which RBOvershellCom::_PollRemoteUsers
// hands to the band user manager's updates. Only the member table is
// modelled; it belongs with RBSompSessionMgr (rb_meta/somp). Names not in
// the reference map.
struct SompSessionMembers {
    // The session state before the table; not modelled.
    unsigned char mOpaque[0x820];
    SompSessionMember mMembers[6];
    unsigned long mNumMembers;
};

static_assert(offsetof(SompSessionMembers, mMembers) == 0x820);
static_assert(offsetof(SompSessionMembers, mNumMembers) == 0x970);

// The game's band user manager (0x923190-0x9268E0, with the static
// initializer at 0x9268F0): the BandUserMgr the game creates at startup. It
// creates eight local and three remote users and keeps them in separate
// lists, answers the user list queries the map's BandUserMgr had (they moved
// here, with filters on the joypad and the connected instrument added), the
// first-time-user events and the overshell slot checks, and copies the
// online (SOMP) session's leader, host and ready flags into the users. Newer
// than the map; the class name is not in the reference map and the evidence
// for it is weak. The vtable at 0x1960498 has BandUserMgr's 11 slots; the
// object is 352 bytes.
//
// The static initializer builds the RakNet header's
// UNASSIGNED_SYSTEM_ADDRESS (0x1ADFAC8) and UNASSIGNED_RAKNET_GUID
// (0x1ADFAE0) copies and the SOMP session settings' defaults (0x1ADFAF0); it
// is not reconstructed.
class RBBandUserMgr : public BandUserMgr {
public:
    // The filters of _GetBandUsers and the other user lists, on top of
    // BandUserMgr::BandUserFlags. Names not in the reference map.
    enum RBBandUserFlags {
        // Passed by the local user queries; no filter tests it (the list
        // queried decides). The evidence for the name is weak.
        kBandUsersLocal = 0x2,
        // Users whose joypad is not connected.
        kBandUsersNoJoypad = 0x40,
        // Users whose joypad is connected.
        kBandUsersWithJoypad = 0x80,
        // Leave out the users whose connected instrument is a guitar, the
        // microphone or the drums (BandUser::ConnectedControllerType).
        kBandUsersNotGuitar = 0x100,
        kBandUsersNotVocals = 0x200,
        kBandUsersNotDrum = 0x400,
    };

    RBBandUserMgr();  // 0x923350
    // Slots 0-1: 0x923420, 0x9234A0. Clears TheRBBandUserMgr.
    ~RBBandUserMgr() override;
    // Slot 10: eight local users, then three remote users.
    void _CreateUsers() override;  // 0x923520

    // Whether the user has a profile and a controller. Name not in the
    // reference map.
    static bool HasProfileAndController(User* user);  // 0x923300

    // The users of each list that pass the BandUserFlags and RBBandUserFlags
    // filters. The map's BandUserMgr has _GetBandUsers(eastl::vector<
    // BandUser*>*, int) const; this build returns the list, and the copies
    // for the other lists are new. The binary keeps an uncalled copy of
    // _GetBandUsers. Names other than _GetBandUsers not in the reference
    // map.
    eastl::vector<BandUser*> _GetBandUsers(int flags) const;  // 0x923AF0
    eastl::vector<RBBandUser*> _GetRBBandUsers(int flags) const;  // 0x923E70
    eastl::vector<LocalBandUser*> _GetLocalUsers(int flags) const;  // 0x9241F0
    eastl::vector<RemoteBandUser*> _GetRemoteUsers(int flags) const;  // 0x924570
    // The user whose profile it is, or null. `settingsOwner` also asks for
    // the user's slot to own the profile's settings; `fail` reports a
    // missing user (only the profile's name survives in this build). The
    // map has GetBandUserByProfile(RBProfile const*, bool) const.
    BandUser* GetBandUserByProfile(const RBProfile* profile, bool fail, bool settingsOwner) const;  // 0x9248F0
    // The band user in the slot. Name not in the reference map.
    RBBandUser* GetRBBandUserFromSlot(unsigned long slot) const;  // 0x9249B0
    // The remote user in the slot, or null. Name not in the reference map.
    RemoteBandUser* GetRemoteUserFromSlot(unsigned long slot) const;  // 0x9249C0
    // The band user in the slot when it is local, or null. Name not in the
    // reference map.
    RBBandUser* GetLocalUserFromSlot(unsigned long slot) const;  // 0x924A20
    // The number of users that pass the filters. The binary keeps an
    // uncalled copy.
    int CountBandUsers(int flags) const;  // 0x924A60

    // The binary keeps uncalled copies of GetNumLocalParticipants,
    // GetNumConnectedUsers and GetLocalUsersWithAnyController.
    unsigned long GetNumLocalParticipants() const;  // 0x925240
    eastl::vector<LocalBandUser*> GetLocalParticipants() const;  // 0x9252C0
    // The users whose joypad is connected (CountBandUsers inlined).
    int GetNumConnectedUsers() const;  // 0x9252E0
    eastl::vector<RBBandUser*> GetParticipatingBandUsers() const;  // 0x925360
    eastl::vector<LocalBandUser*> GetLocalUsersWithAnyController() const;  // 0x925380
    eastl::vector<LocalBandUser*> GetLocalUsersNotParticipatingWithAnyController() const;  // 0x9253A0
    // The local users who do not take part and whose connected instrument
    // is the type; any other type gives every such user.
    eastl::vector<LocalBandUser*> GetLocalNonParticipantsWithControllerType(ControllerType type) const;  // 0x9253C0
    // The first local user the session marks as its leader, or null. Name
    // not in the reference map.
    LocalBandUser* GetLocalSompLeader() const;  // 0x925410
    // The local participant on the platform user id, or null. The binary
    // reads the list of TheRBBandUserMgr, not of this object. Name not in
    // the reference map.
    LocalBandUser* GetLocalParticipantFromPad(int padNum) const;  // 0x925480
    // The remote users who do not take part. Name not in the reference
    // map.
    eastl::vector<RemoteBandUser*> GetRemoteNonParticipants() const;  // 0x925540
    // Adds the slots whose user is missing or is not the session's host.
    // Name not in the reference map; the evidence is weak.
    void GetSlotsWithoutSompHost(eastl::vector<unsigned long>& slots) const;  // 0x925560

    // Copies the members' leader flags into the participants, then follows
    // the leader the table names: a new leader is recorded and exported as
    // "somp_leader_changed" (the old member and user ids, then the new
    // ones, as text). Name not in the reference map.
    void UpdateSompLeader(const SompSessionMembers& session);  // 0x925680
    // The member record of the user: a local user's by the user's id among
    // the local members, a remote user's by the member and owner ids among
    // the others; null without one. Name not in the reference map.
    const SompSessionMember* _FindSompMember(RBBandUser* user, const SompSessionMembers& session) const;  // 0x925D20
    // Forgets the session's leader and clears the participants' session
    // flags. Name not in the reference map.
    void ResetSompState();  // 0x925E10
    // Copy the members' host and ready flags into the participants. Names
    // not in the reference map; the evidence is weak.
    void UpdateSompHosts(const SompSessionMembers& session);  // 0x925EE0
    void UpdateSompReady(const SompSessionMembers& session);  // 0x926050

    // Whether any user's or profile's first-time-user events have the
    // event.
    bool HasAnyoneSeenEvent(Symbol event) const;  // 0x9261C0
    // The first-time-user events of the local users (the profile's or the
    // user's own) and of every profile, each once.
    eastl::vector<FTUEPersistentData*> _GetFTUEDataForAllUsers() const;  // 0x926270
    // Marks the event seen in every first-time-user event list.
    void MarkParticipatingUsersEventSeen(Symbol event);  // 0x926620
    // Whether some, or all, local participants have their overshell slot
    // out of its closed states (BandUser::IsOvershellSlotClosed), and
    // whether all are in them.
    bool DoAnyUsersHaveTheirOvershellSlotOpen() const;  // 0x9266B0
    bool DoAllUsersHaveTheirOvershellSlotOpen() const;  // 0x926770
    bool DoAllUsersHaveTheirOvershellSlotClosed() const;  // 0x926830

    // Field names are not in the reference map.
    eastl::vector<LocalBandUser*> mLocalUsers;
    eastl::vector<RemoteBandUser*> mRemoteUsers;
    // The session leader's member id and user id.
    RakNet::RakNetGUID mSompLeaderMemberId;
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
