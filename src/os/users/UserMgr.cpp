#include "os/users/UserMgr.h"

#include "os/joypads/Joypad.h"
#include "os/platform/PlatformMgr.h"
#include "os/users/User.h"
#include "persistence/saveload/SaveLoadMgr.h"
#include "utl/data/DataArray.h"
#include "utl/text/Symbol.h"

UserMgr* TheUserMgr;

// Reconstructed from eboot.elf at 0x36AD30.
void SetTheUserMgr(UserMgr* mgr) {
    TheUserMgr = mgr;
    ResetAllUsersPads();
}

// Reconstructed from eboot.elf at 0x36AD40.
UserMgr::UserMgr() : SyncMsgSource(&mCritSec) {
    for (MsgSource::EventSinkElem* sink : {&mUserSignedOutSink, &mLoadEndSink}) {
        sink->mSink = nullptr;
        sink->mSource = nullptr;
    }
    DataNode self;
    self.mType = kDataObject;
    self.mValue.object = static_cast<MsgSink*>(this);
    DataSetGlobal(Symbol("user_mgr"), self);
    ThePlatformMgr->AddSink(&mUserSignedOutSink, this, UserSignedOutMsg::Event(), Symbol());
}

// Reconstructed from eboot.elf at 0x36AF20. A linked subscription leaves its
// source before the link unlinks itself, the later one first.
UserMgr::~UserMgr() {
    for (MsgSource::EventSinkElem* sink : {&mLoadEndSink, &mUserSignedOutSink}) {
        LinkedList::Node& link = sink->mLink;
        if (link.mNext != &link && link.mPrev != &link) {
            sink->mSource->RemoveSink(sink);
        }
    }
}

// Reconstructed from eboot.elf at 0x36B040.
void UserMgr::PostInit(UserMgr* mgr) {
    mgr->PostInit();
}

// Reconstructed from eboot.elf at 0x36B110.
void UserMgr::PostInit() {
    if (theSaveLoadMgr != nullptr) {
        static Symbol loadEnd;
        if (loadEnd == Symbol()) {
            loadEnd = Symbol("load_end");
        }
        theSaveLoadMgr->AddSink(&mLoadEndSink, this, loadEnd, Symbol());
    }
}

// Reconstructed from eboot.elf at 0x36B1E0.
User* UserMgr::GetUserFromPadNum(int padNum) const {
    return JoypadGetUserFromPadNum(padNum);
}

// Reconstructed from eboot.elf at 0x36B1F0.
eastl::vector<User*> UserMgr::GetUsersFromPlatformUserId(int platformUserId) const {
    eastl::vector<User*> users;
    GetUsers(users);
    eastl::vector<User*> matching;
    for (User* const user : users) {
        if (user->GetPlatformUserId() == platformUserId) {
            matching.push_back(user);
        }
    }
    return matching;
}

// Reconstructed from eboot.elf at 0x36B3C0.
DataNode UserMgr::OnMsg(const UserSignedOutMsg& msg) {
    const eastl::vector<User*> users = GetUsersFromPlatformUserId(msg.GetUser());
    for (User* const user : users) {
        user->SetPlatformUserId(-1);
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x36B490. Errors 0 and 2 count as loaded.
void UserMgr::OnLoadEnd(int error, int padNum) {
    if ((error | 2) == 2) {
        const eastl::vector<User*> users = GetUsersFromPlatformUserId(padNum);
        for (User* const user : users) {
            user->OnProfileLoaded();
        }
    }
}

// Reconstructed from eboot.elf at 0x36B520.
DataNode UserMgr::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    if (type == UserSignedOutMsg::Event()) {
        DataNode result = OnMsg(UserSignedOutMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol getUserFromPadNum;
    if (getUserFromPadNum == Symbol()) {
        getUserFromPadNum = Symbol("get_user_from_pad_num");
    }
    if (type == getUserFromPadNum) {
        DataNode user;
        user.mType = kDataObject;
        user.mValue.object = static_cast<MsgSink*>(GetUserFromPadNum(msg->Int(2)));
        return user;
    }
    static Symbol loadEnd;
    if (loadEnd == Symbol()) {
        loadEnd = Symbol("load_end");
    }
    if (type == loadEnd) {
        OnLoadEnd(msg->Int(2), msg->Int(3));
        return DataNode(0);
    }
    return DataNode();
}
