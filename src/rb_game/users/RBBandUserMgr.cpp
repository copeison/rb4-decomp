#include "rb_game/users/RBBandUserMgr.h"

#include "os/debug/Debug.h"
#include "os/platform/PlatformMgr.h"
#include "rb_game/users/LocalBandUser.h"
#include "rb_game/users/RemoteBandUser.h"
#include "rb_meta/persistentdata/FTUEPersistentData.h"
#include "rb_meta/profiles/RBProfile.h"
#include "rb_meta/profiles/RBProfileMgr.h"
#include "utl/messages/Message.h"

RBBandUserMgr* TheRBBandUserMgr;

namespace {

// Whether the user passes the BandUserMgr::BandUserFlags and
// RBBandUserMgr::RBBandUserFlags filters. The session and platform queries
// are made before the first filters, the joypad and instrument queries
// before the others. Inlined into every user list and count. Name not in
// the reference map.
bool PassesRBBandUserFlags(const BandUser* user, int flags) {
    const bool participating = user->mParticipating;
    const bool signedIn = (flags & BandUserMgr::kBandUsersSignedIn) == 0 || user->IsSignedIn();
    const bool loggedIn =
        (flags & BandUserMgr::kBandUsersLoggedIn) == 0 || ThePlatformMgr->IsUserLoggedIn(user->GetPlatformUserId());
    const bool leader = user->IsSompLeader();
    if ((flags & BandUserMgr::kBandUsersNotParticipating) != 0 && participating) {
        return false;
    }
    if ((flags & BandUserMgr::kBandUsersParticipating) != 0 && !participating) {
        return false;
    }
    if ((flags & BandUserMgr::kBandUsersExcluded) != 0) {
        return false;
    }
    if (!signedIn || !loggedIn) {
        return false;
    }
    if ((flags & BandUserMgr::kBandUsersSompLeader) != 0 && !leader) {
        return false;
    }
    const bool joypad = user->IsJoypadConnected();
    const ControllerType connected = user->ConnectedControllerType();
    if ((flags & RBBandUserMgr::kBandUsersNotDrum) != 0 && connected == kControllerDrum) {
        return false;
    }
    if ((flags & RBBandUserMgr::kBandUsersNotVocals) != 0 && connected == kControllerVocals) {
        return false;
    }
    if ((flags & RBBandUserMgr::kBandUsersNoJoypad) != 0 && joypad) {
        return false;
    }
    if ((flags & RBBandUserMgr::kBandUsersWithJoypad) != 0 && !joypad) {
        return false;
    }
    return (flags & RBBandUserMgr::kBandUsersNotGuitar) == 0 || connected != kControllerGuitar;
}

// The users of the list that pass the filters, in a list reserved for all
// of them. Inlined into the four list queries. Name not in the reference
// map.
template <typename T, typename U>
eastl::vector<T*> FilterUsers(const eastl::vector<U*>& users, int flags) {
    eastl::vector<T*> result;
    result.reserve(users.size());
    for (U* const user : users) {
        if (PassesRBBandUserFlags(user, flags)) {
            result.push_back(static_cast<T*>(user));
        }
    }
    return result;
}

// Adds the events unless the list has them. Inlined into
// _GetFTUEDataForAllUsers. Name not in the reference map.
void AddFTUEData(eastl::vector<FTUEPersistentData*>& list, FTUEPersistentData* data) {
    FTUEPersistentData** found = list.begin();
    while (found != list.end() && *found != data) {
        ++found;
    }
    if (found == list.end()) {
        list.push_back(data);
    }
}

}  // namespace

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

// Reconstructed from eboot.elf at 0x923350.
RBBandUserMgr::RBBandUserMgr() : mSompLeaderMemberId(RakNet::UNASSIGNED_RAKNET_GUID), mSompLeaderGuid(UserGuid()) {}

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

// Reconstructed from eboot.elf at 0x923AF0.
eastl::vector<BandUser*> RBBandUserMgr::_GetBandUsers(int flags) const {
    return FilterUsers<BandUser>(mUsers, flags);
}

// Reconstructed from eboot.elf at 0x923E70.
eastl::vector<RBBandUser*> RBBandUserMgr::_GetRBBandUsers(int flags) const {
    return FilterUsers<RBBandUser>(mUsers, flags);
}

// Reconstructed from eboot.elf at 0x9241F0.
eastl::vector<LocalBandUser*> RBBandUserMgr::_GetLocalUsers(int flags) const {
    return FilterUsers<LocalBandUser>(mLocalUsers, flags);
}

// Reconstructed from eboot.elf at 0x924570.
eastl::vector<RemoteBandUser*> RBBandUserMgr::_GetRemoteUsers(int flags) const {
    return FilterUsers<RemoteBandUser>(mRemoteUsers, flags);
}

// Reconstructed from eboot.elf at 0x9248F0. The release build only reads
// the profile's name for its report.
BandUser* RBBandUserMgr::GetBandUserByProfile(const RBProfile* profile, bool fail, bool settingsOwner) const {
    if (profile == nullptr) {
        return nullptr;
    }
    for (BandUser* const user : mUsers) {
        if (user->GetProfile(false) != profile) {
            continue;
        }
        if (!settingsOwner || theRBProfileMgr->SlotOwnsProfileSettings(user->GetSlot())) {
            return user;
        }
    }
    if (fail) {
        static_cast<void>(profile->GetPlatformName());
    }
    return nullptr;
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

// Reconstructed from eboot.elf at 0x924A60.
int RBBandUserMgr::CountBandUsers(int flags) const {
    int count = 0;
    for (BandUser* const user : mUsers) {
        count += PassesRBBandUserFlags(user, flags);
    }
    return count;
}

// Reconstructed from eboot.elf at 0x925240.
unsigned long RBBandUserMgr::GetNumLocalParticipants() const {
    return GetLocalParticipants().size();
}

// Reconstructed from eboot.elf at 0x9252C0.
eastl::vector<LocalBandUser*> RBBandUserMgr::GetLocalParticipants() const {
    return _GetLocalUsers(kBandUsersLocal | kBandUsersParticipating);
}

// Reconstructed from eboot.elf at 0x9252E0.
int RBBandUserMgr::GetNumConnectedUsers() const {
    return CountBandUsers(kBandUsersWithJoypad);
}

// Reconstructed from eboot.elf at 0x925360.
eastl::vector<RBBandUser*> RBBandUserMgr::GetParticipatingBandUsers() const {
    return _GetRBBandUsers(kBandUsersParticipating);
}

// Reconstructed from eboot.elf at 0x925380.
eastl::vector<LocalBandUser*> RBBandUserMgr::GetLocalUsersWithAnyController() const {
    return _GetLocalUsers(kBandUsersWithJoypad);
}

// Reconstructed from eboot.elf at 0x9253A0.
eastl::vector<LocalBandUser*> RBBandUserMgr::GetLocalUsersNotParticipatingWithAnyController() const {
    return _GetLocalUsers(kBandUsersWithJoypad | kBandUsersNotParticipating | kBandUsersLocal);
}

// Reconstructed from eboot.elf at 0x9253C0.
eastl::vector<LocalBandUser*> RBBandUserMgr::GetLocalNonParticipantsWithControllerType(ControllerType type) const {
    int flags = kBandUsersWithJoypad | kBandUsersNotParticipating | kBandUsersLocal;
    switch (type) {
    case kControllerDrum:
        flags |= kBandUsersNotGuitar | kBandUsersNotVocals;
        break;
    case kControllerGuitar:
        flags |= kBandUsersNotVocals | kBandUsersNotDrum;
        break;
    case kControllerVocals:
        flags |= kBandUsersNotGuitar | kBandUsersNotDrum;
        break;
    default:
        break;
    }
    return _GetLocalUsers(flags);
}

// Reconstructed from eboot.elf at 0x925410.
LocalBandUser* RBBandUserMgr::GetLocalSompLeader() const {
    const eastl::vector<LocalBandUser*> users = _GetLocalUsers(kBandUsersSompLeader);
    return users.empty() ? nullptr : users.front();
}

// Reconstructed from eboot.elf at 0x925480.
LocalBandUser* RBBandUserMgr::GetLocalParticipantFromPad(int padNum) const {
    const eastl::vector<LocalBandUser*> users = TheRBBandUserMgr->GetLocalParticipants();
    for (LocalBandUser* const user : users) {
        if (user->GetPlatformUserId() == padNum) {
            return user;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x925540.
eastl::vector<RemoteBandUser*> RBBandUserMgr::GetRemoteNonParticipants() const {
    return _GetRemoteUsers(kBandUsersNotParticipating);
}

// Reconstructed from eboot.elf at 0x925560.
void RBBandUserMgr::GetSlotsWithoutSompHost(eastl::vector<unsigned long>& slots) const {
    for (unsigned long slot = 0; slot < 4; slot++) {
        const RBBandUser* const user = static_cast<const RBBandUser*>(GetUserFromSlot(slot));
        if (user == nullptr || !user->mSompHost) {
            slots.push_back(slot);
        }
    }
}

// Reconstructed from eboot.elf at 0x925680. The message type is built on
// each change.
void RBBandUserMgr::UpdateSompLeader(const SompSessionMembers& session) {
    const eastl::vector<RBBandUser*> users = _GetRBBandUsers(kBandUsersParticipating);
    for (RBBandUser* const user : users) {
        const SompSessionMember* const member = _FindSompMember(user, session);
        user->mSompLeader = member != nullptr ? member->mLeader : false;
    }
    for (unsigned long i = 0; i < session.mNumMembers; i++) {
        const SompSessionMember& member = session.mMembers[i];
        if (!member.mLeader || member.mMemberId == mSompLeaderMemberId || member.mUserGuid == mSompLeaderGuid) {
            continue;
        }
        Message<4> msg(Symbol("somp_leader_changed"));
        msg.mData->Node(2) = DataNode(mSompLeaderMemberId.ToString());
        msg.mData->Node(3) = DataNode(mSompLeaderGuid.ToString());
        msg.mData->Node(4) = DataNode(member.mMemberId.ToString());
        msg.mData->Node(5) = DataNode(member.mUserGuid.ToString());
        mSompLeaderMemberId = member.mMemberId;
        mSompLeaderGuid = member.mUserGuid;
        Export(msg.mData);
    }
}

// Reconstructed from eboot.elf at 0x925D20.
const SompSessionMember* RBBandUserMgr::_FindSompMember(RBBandUser* user, const SompSessionMembers& session) const {
    for (unsigned long i = 0; i < session.mNumMembers; i++) {
        const SompSessionMember& member = session.mMembers[i];
        if (user->IsLocal()) {
            if (member.mLocal && user->GetUserGuid() == member.mUserGuid) {
                return &member;
            }
        } else {
            const RemoteBandUser* const remote = static_cast<const RemoteBandUser*>(user);
            if (!member.mLocal && remote->mMemberId == member.mMemberId && remote->mOwnerGuid == member.mUserGuid) {
                return &member;
            }
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x925E10.
void RBBandUserMgr::ResetSompState() {
    mSompLeaderMemberId = RakNet::UNASSIGNED_RAKNET_GUID;
    mSompLeaderGuid = UserGuid();
    const eastl::vector<RBBandUser*> users = _GetRBBandUsers(kBandUsersParticipating);
    for (RBBandUser* const user : users) {
        user->mSompLeader = false;
        user->mSompHost = false;
        user->mSompReady = false;
    }
}

// Reconstructed from eboot.elf at 0x925EE0.
void RBBandUserMgr::UpdateSompHosts(const SompSessionMembers& session) {
    const eastl::vector<RBBandUser*> users = _GetRBBandUsers(kBandUsersParticipating);
    for (RBBandUser* const user : users) {
        const SompSessionMember* const member = _FindSompMember(user, session);
        user->mSompHost = member != nullptr ? member->mHost : false;
    }
}

// Reconstructed from eboot.elf at 0x926050.
void RBBandUserMgr::UpdateSompReady(const SompSessionMembers& session) {
    const eastl::vector<RBBandUser*> users = _GetRBBandUsers(kBandUsersParticipating);
    for (RBBandUser* const user : users) {
        const SompSessionMember* const member = _FindSompMember(user, session);
        user->mSompReady = member != nullptr ? member->mReady : false;
    }
}

// Reconstructed from eboot.elf at 0x9261C0.
bool RBBandUserMgr::HasAnyoneSeenEvent(Symbol event) const {
    const eastl::vector<FTUEPersistentData*> data = _GetFTUEDataForAllUsers();
    for (FTUEPersistentData* const events : data) {
        if (events->HasSeenEvent(event)) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x926270.
eastl::vector<FTUEPersistentData*> RBBandUserMgr::_GetFTUEDataForAllUsers() const {
    eastl::vector<FTUEPersistentData*> data;
    const eastl::vector<LocalBandUser*> users = mLocalUsers;
    for (LocalBandUser* const user : users) {
        AddFTUEData(data, user->GetFTUEData());
    }
    const eastl::vector<RBProfile*> profiles = theRBProfileMgr->GetProfiles();
    for (RBProfile* const profile : profiles) {
        AddFTUEData(data, &profile->mFTUEData);
    }
    return data;
}

// Reconstructed from eboot.elf at 0x926620.
void RBBandUserMgr::MarkParticipatingUsersEventSeen(Symbol event) {
    const eastl::vector<FTUEPersistentData*> data = _GetFTUEDataForAllUsers();
    for (FTUEPersistentData* const events : data) {
        events->MarkEventSeen(event);
    }
}

// Reconstructed from eboot.elf at 0x9266B0.
bool RBBandUserMgr::DoAnyUsersHaveTheirOvershellSlotOpen() const {
    const eastl::vector<LocalBandUser*> users = GetLocalParticipants();
    for (LocalBandUser* const user : users) {
        if (!user->IsOvershellSlotClosed()) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x926770.
bool RBBandUserMgr::DoAllUsersHaveTheirOvershellSlotOpen() const {
    const eastl::vector<LocalBandUser*> users = GetLocalParticipants();
    for (LocalBandUser* const user : users) {
        if (user->IsOvershellSlotClosed()) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x926830.
bool RBBandUserMgr::DoAllUsersHaveTheirOvershellSlotClosed() const {
    const eastl::vector<LocalBandUser*> users = GetLocalParticipants();
    for (LocalBandUser* const user : users) {
        if (!user->IsOvershellSlotClosed()) {
            return false;
        }
    }
    return true;
}
