#include "rb_game/users/BandUserMgr.h"

#include "os/platform/PlatformMgr.h"
#include "rb_game/users/BandUser.h"
#include "utl/data/DataArray.h"
#include "utl/messages/Message.h"
#include "utl/text/Symbol.h"

BandUserMgr* theBandUserMgr;

namespace {

// A script node holding the band user. Name not in the reference map.
DataNode BandUserNode(const BandUser* user) {
    DataNode node;
    node.mType = kDataObject;
    node.mValue.object = const_cast<BandUser*>(user);
    return node;
}

// Whether the user passes the BandUserMgr::BandUserFlags filters. Every
// query is made before the filters are tested. Inlined into _GetBandUsers
// and CountBandUsers. Name not in the reference map.
bool PassesBandUserFlags(const BandUser* user, int flags) {
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
    return (flags & BandUserMgr::kBandUsersSompLeader) == 0 || leader;
}

}  // namespace

// Reconstructed from eboot.elf at 0xA40130.
BandUserMgr::BandUserMgr() {
    mSlotsReplica = new BandSlotsReplica();
    mUserDataReplica = new BandUserDataReplica();
}

// Reconstructed from eboot.elf at 0xA401F0.
BandUserMgr::~BandUserMgr() {
    theBandUserMgr = nullptr;
    for (BandUser* const user : mUsers) {
        delete user;
    }
    mUsers.clear();
    delete mUserDataReplica;
    delete mSlotsReplica;
}

// Reconstructed from eboot.elf at 0xA40380.
void BandUserMgr::Init() {
    _CreateUsers();
    ResetSlots();
}

// Reconstructed from eboot.elf at 0xA403D0.
User* BandUserMgr::GetUser(const UserGuid& guid, bool fail) const {
    return GetBandUser(guid, fail);
}

// Reconstructed from eboot.elf at 0xA40460. The release build only formats
// the missing id for its report.
BandUser* BandUserMgr::GetBandUser(const UserGuid& guid, bool fail) const {
    for (unsigned long i = 0; i < mUsers.size(); i++) {
        if (mUsers[i]->GetUserGuid() == guid) {
            return mUsers[i];
        }
    }
    if (fail) {
        static_cast<void>(guid.ToString());
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0xA404F0.
BandUser* BandUserMgr::GetBandUser(User* user) {
    return static_cast<BandUser*>(user);
}

// Reconstructed from eboot.elf at 0xA40500.
void BandUserMgr::GetUsers(eastl::vector<User*>& users) const {
    for (unsigned long i = 0; i < mUsers.size(); i++) {
        users.push_back(mUsers[i]);
    }
}

// Reconstructed from eboot.elf at 0xA40640.
void BandUserMgr::ResetSlots() {
    for (UserGuid& slot : mSlots) {
        slot.Clear();
    }
}

// Reconstructed from eboot.elf at 0xA40690. The release build keeps an
// unused null test of the id.
unsigned long BandUserMgr::GetSlot(const UserGuid& guid) const {
    static_cast<void>(guid.IsNull());
    for (unsigned long slot = 0; slot < 4; slot++) {
        if (mSlots[slot] == guid) {
            return slot;
        }
    }
    return static_cast<unsigned long>(-1);
}

// Reconstructed from eboot.elf at 0xA40720.
void BandUserMgr::SetSlot(const BandUser* user, unsigned long slot) {
    if (slot != static_cast<unsigned long>(-1)) {
        mSlots[slot] = user->GetUserGuid();
        static Symbol userSlotSet;
        if (userSlotSet == Symbol()) {
            userSlotSet = Symbol("user_slot_set");
        }
        Message<2> msg(userSlotSet);
        msg.mData->Node(2) = DataNode(static_cast<int>(slot));
        msg.mData->Node(3) = BandUserNode(user);
        Export(msg.mData);
        return;
    }
    const unsigned long oldSlot = GetSlot(user->GetUserGuid());
    if (oldSlot == static_cast<unsigned long>(-1)) {
        return;
    }
    mSlots[oldSlot].Clear();
    static Symbol userSlotCleared;
    if (userSlotCleared == Symbol()) {
        userSlotCleared = Symbol("user_slot_cleared");
    }
    Message<2> msg(userSlotCleared);
    msg.mData->Node(2) = DataNode(static_cast<int>(oldSlot));
    msg.mData->Node(3) = BandUserNode(user);
    Export(msg.mData);
}

// Reconstructed from eboot.elf at 0xA40C30.
void BandUserMgr::ClearSlot(BandUser* user) {
    SetSlot(user, static_cast<unsigned long>(-1));
}

// Reconstructed from eboot.elf at 0xA40C40.
const UserGuid& BandUserMgr::GetUserGuidFromSlot(unsigned long slot) const {
    return mSlots[slot];
}

// Reconstructed from eboot.elf at 0xA40C50.
BandUser* BandUserMgr::GetUserFromSlot(unsigned long slot) const {
    const UserGuid& guid = GetUserGuidFromSlot(slot);
    if (guid.IsNull()) {
        return nullptr;
    }
    return GetBandUser(guid, true);
}

// Reconstructed from eboot.elf at 0xA40CF0.
BandUser* BandUserMgr::GetUserFromTrackType(TrackType type) const {
    const eastl::vector<BandUser*> users = _GetBandUsers(kBandUsersParticipating);
    for (BandUser* const user : users) {
        if (user->GetTrackType() == type) {
            return user;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0xA40DC0.
eastl::vector<BandUser*> BandUserMgr::GetBandUsers() const {
    return _GetBandUsers(kBandUsersParticipating);
}

// Reconstructed from eboot.elf at 0xA40DE0.
int BandUserMgr::MaxSlot() const {
    int maxSlot = -1;
    for (int slot = 0; slot < 4; slot++) {
        if (!mSlots[slot].IsNull()) {
            maxSlot = slot;
        }
    }
    return maxSlot;
}

// Reconstructed from eboot.elf at 0xA40E60.
unsigned long BandUserMgr::GetNumParticipants() const {
    unsigned long count = 0;
    for (unsigned long i = 0; i < mUsers.size(); i++) {
        count += mUsers[i]->mParticipating;
    }
    return count;
}

// Reconstructed from eboot.elf at 0xA40EA0.
bool BandUserMgr::IsMultiplayerGame() const {
    return GetNumParticipants() > 1;
}

// Reconstructed from eboot.elf at 0xA40EE0.
int BandUserMgr::GetNumUsersInSlots() const {
    int count = 0;
    for (unsigned long slot = 0; slot < 4; slot++) {
        count += GetUserFromSlot(slot) != nullptr;
    }
    return count;
}

// Reconstructed from eboot.elf at 0xA41110.
eastl::vector<BandUser*> BandUserMgr::_GetBandUsers(int flags) const {
    eastl::vector<BandUser*> users;
    users.reserve(mUsers.size());
    for (BandUser* const user : mUsers) {
        if (PassesBandUserFlags(user, flags)) {
            users.push_back(user);
        }
    }
    return users;
}

// Reconstructed from eboot.elf at 0xA41770.
int BandUserMgr::CountBandUsers(int flags) const {
    int count = 0;
    for (BandUser* const user : mUsers) {
        count += PassesBandUserFlags(user, flags);
    }
    return count;
}

// Reconstructed from eboot.elf at 0xA41A60.
int BandUserMgr::GetNumSignedInUsers() const {
    return CountBandUsers(kBandUsersSignedIn);
}

// Reconstructed from eboot.elf at 0xA41AD0.
BandUser* BandUserMgr::GetUserFromPad(int padNum) const {
    for (BandUser* const user : mUsers) {
        if (user->GetPadNum() == padNum) {
            return user;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0xA41B30.
bool BandUserMgr::IsAnyUserSignedIn() const {
    for (BandUser* const user : mUsers) {
        if (user->IsSignedIn()) {
            return true;
        }
    }
    return false;
}
