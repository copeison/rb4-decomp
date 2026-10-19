#include "os/users/User.h"

#include "os/joypads/Joypad.h"
#include "os/platform/PlatformMgr.h"
#include "utl/data/DataArray.h"
#include "utl/text/MakeString.h"
#include "utl/text/Symbol.h"

// Reconstructed from eboot.elf at 0x3A8050. A new user gets a generated id
// and no platform user.
User::User() : mPlatformUserId(-1), mPlatformUserIdSet(false) {
    mUserGuid.Generate();
}

// Reconstructed from eboot.elf at 0x3A8090.
bool User::operator==(const User& other) const {
    return mUserGuid == other.mUserGuid;
}

// Reconstructed from eboot.elf at 0x3A80A0.
bool User::operator<(const User& other) const {
    return mUserGuid < other.mUserGuid;
}

// Reconstructed from eboot.elf at 0x3A80B0.
void User::SetUserGuid(const UserGuid& guid) {
    mUserGuid = guid;
}

// Reconstructed from eboot.elf at 0x3A80C0.
int User::GetPadNum() const {
    return JoypadGetUsersPadNum(this);
}

// Reconstructed from eboot.elf at 0x3A80D0.
bool User::IsSignedIn() const {
    return ThePlatformMgr->IsSignedIn(mPlatformUserId);
}

// Reconstructed from eboot.elf at 0x3A80F0.
bool User::IsJoypadConnected() const {
    return JoypadIsConnectedPadNum(GetPadNum());
}

// Reconstructed from eboot.elf at 0x3A8110.
const char* User::UserName() const {
    if (mPlatformUserId != -1) {
        return ThePlatformMgr->GetName(mPlatformUserId);
    }
    FormatString name("Player %d");
    name << GetPadNum();
    return name.Str();
}

// Reconstructed from eboot.elf at 0x3A81C0.
void User::SetPlatformUserId(int platformUserId) {
    if (mPlatformUserIdSet && mPlatformUserId == platformUserId) {
        return;
    }
    mPlatformUserIdSet = true;
    mPlatformUserId = platformUserId;
    _OnPlatformUserIdChanged();
}

// Reconstructed from eboot.elf at 0x3A81E0.
void User::OnProfileLoaded() {
    _OnProfileLoaded();
}

// Reconstructed from eboot.elf at 0x3A81F0. The other user of
// "comes_before" arrives as its MsgSink; the binary evaluates the node
// through a copy of the map's DataNode::GetSink at 0xF9450.
DataNode User::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol getPlayerName;
    if (getPlayerName == Symbol()) {
        getPlayerName = Symbol("get_player_name");
    }
    if (type == getPlayerName) {
        return DataNode(UserName());
    }
    static Symbol comesBefore;
    if (comesBefore == Symbol()) {
        comesBefore = Symbol("comes_before");
    }
    if (type == comesBefore) {
        const User* const other = static_cast<User*>(static_cast<MsgSink*>(msg->Evaluate(2).LiteralSink(msg)));
        return DataNode(static_cast<int>(mUserGuid < other->mUserGuid));
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0x3A83D0.
User::~User() {}

// Reconstructed from eboot.elf at 0x3A83F0.
int User::GetPlatformUserId() const {
    return mPlatformUserId;
}

// Reconstructed from eboot.elf at 0x3A8400.
void User::_OnProfileLoaded() {}

// Reconstructed from eboot.elf at 0x3A8410.
void User::_OnPlatformUserIdChanged() {}
