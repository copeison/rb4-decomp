#include "persistence/profiles/ProfileMgr.h"

#include <string.h>

#include "os/platform/PlatformMgr.h"
#include "os/threading/CritSec.h"
#include "persistence/options/SystemOptions.h"
#include "persistence/profiles/Profile.h"
#include "persistence/saveload/SaveLoadMgr.h"
#include "utl/data/DataArray.h"
#include "utl/messages/Message.h"
#include "utl/text/Symbol.h"

namespace {

// The lock the manager's subscriptions and exports take. At 0x1A6E2E0; the
// object's static initializer (0x3B4400) constructs it. Name not in the
// reference map.
CritSec gProfileMgrCritSec;

// The object node of a profile: its MsgSink, or null. Name not in the
// reference map.
inline DataNode ProfileNode(Profile* profile) {
    DataNode node;
    node.mType = kDataObject;
    node.mValue.object = static_cast<MsgSink*>(profile);
    return node;
}

}  // namespace

// Reconstructed from eboot.elf at 0x3B28F0.
void ProfileMgr::Init(ProfileMgr* mgr) {
    DataNode self;
    self.mType = kDataObject;
    self.mValue.object = static_cast<MsgSink*>(mgr);
    DataSetGlobal(Symbol("profile_mgr"), self);
}

// Reconstructed from eboot.elf at 0x3B29A0.
void ProfileMgr::PostInit(ProfileMgr* mgr) {
    mgr->PostInit();
}

// Reconstructed from eboot.elf at 0x3B29B0.
void ProfileMgr::PostInit() {
    if (_UsesSaveLoadMgr()) {
        static Symbol loadEnd;
        if (loadEnd == Symbol()) {
            loadEnd = Symbol("load_end");
        }
        static Symbol saveEnd;
        if (saveEnd == Symbol()) {
            saveEnd = Symbol("save_end");
        }
        theSaveLoadMgr->AddSink(&mLoadEndSink, this, loadEnd, Symbol());
        theSaveLoadMgr->AddSink(&mSaveEndSink, this, saveEnd, Symbol());
        static Symbol saveoptEnd;
        if (saveoptEnd == Symbol()) {
            saveoptEnd = Symbol("saveopt_end");
        }
        static Symbol onSystemOptionsLoaded;
        if (onSystemOptionsLoaded == Symbol()) {
            onSystemOptionsLoaded = Symbol("on_system_options_loaded");
        }
        theSaveLoadMgr->AddSink(&mSaveOptEndSink, this, saveoptEnd, onSystemOptionsLoaded);
    }
}

// Reconstructed from eboot.elf at 0x3B2BD0.
ProfileMgr::ProfileMgr() : SyncMsgSource(&gProfileMgrCritSec) {
    for (MsgSource::EventSinkElem* sink : {&mUserSignedInSink,
                                           &mUserSignedOutSink,
                                           &mAppSuspendSink,
                                           &mPlatformIdentitySink,
                                           &mLoadEndSink,
                                           &mSaveEndSink,
                                           &mSaveOptEndSink}) {
        sink->mSink = nullptr;
        sink->mSource = nullptr;
    }
    ThePlatformMgr->AddSink(&mUserSignedInSink, this, UserSignedInMsg::Event(), Symbol());
    ThePlatformMgr->AddSink(&mUserSignedOutSink, this, UserSignedOutMsg::Event(), Symbol());
    ThePlatformMgr->AddSink(&mAppSuspendSink, this, AppSuspendStateChangedMsg::Event(), Symbol());
    ThePlatformMgr->AddSink(&mPlatformIdentitySink, this, PlatformIdentityChangedMsg::Event(), Symbol());
    mProfiles.reserve(64);
}

// Reconstructed from eboot.elf at 0x3B3020. A linked subscription leaves its
// source before the link unlinks itself; the binary does this for each
// subscription as it is destroyed, the later ones first.
ProfileMgr::~ProfileMgr() {
    for (Profile* const profile : mProfiles) {
        delete profile;
    }
    mProfiles.clear();
    for (MsgSource::EventSinkElem* sink : {&mSaveOptEndSink,
                                           &mSaveEndSink,
                                           &mLoadEndSink,
                                           &mPlatformIdentitySink,
                                           &mAppSuspendSink,
                                           &mUserSignedOutSink,
                                           &mUserSignedInSink}) {
        LinkedList::Node& link = sink->mLink;
        if (link.mNext != &link && link.mPrev != &link) {
            sink->mSource->RemoveSink(sink);
        }
    }
}

// Reconstructed from eboot.elf at 0x3B32D0.
void ProfileMgr::Poll() {
    for (Profile* const profile : mProfiles) {
        profile->Poll();
    }
}

// Reconstructed from eboot.elf at 0x3B3310. The profile is erased from
// where it was found, or the last one is dropped when it is not the
// manager's.
void ProfileMgr::DestroyProfile(Profile* profile) {
    Profile** found = mProfiles.begin();
    while (found != mProfiles.end() && *found != profile) {
        ++found;
    }
    Profile** const end = mProfiles.end();
    if (found + 1 < end) {
        memmove(found, found + 1, reinterpret_cast<char*>(end) - reinterpret_cast<char*>(found + 1));
    }
    mProfiles.pop_back();
    _OnProfileAboutToBeRemoved(profile);
    static Symbol profileAboutToBeRemoved;
    if (profileAboutToBeRemoved == Symbol()) {
        profileAboutToBeRemoved = Symbol("profile_about_to_be_removed");
    }
    Message<1> msg(profileAboutToBeRemoved);
    msg.mData->Node(2) = ProfileNode(profile);
    Export(msg.mData);
    delete profile;
}

// Reconstructed from eboot.elf at 0x3B3520.
void ProfileMgr::ResetAll() {
    for (Profile* const profile : mProfiles) {
        delete profile;
    }
    mProfiles.clear();
}

// Reconstructed from eboot.elf at 0x3B3570.
int ProfileMgr::GetBestStorageDevice() {
    return -1;
}

// Reconstructed from eboot.elf at 0x3B3580.
void ProfileMgr::_OnLoadEnd(int error, int padNum) {
    if (error == SaveLoad::kUnknownError || error == SaveLoad::kDeviceFull) {
        _OnProfileFailedToLoad(padNum);
    }
    static Symbol profileLoaded;
    if (profileLoaded == Symbol()) {
        profileLoaded = Symbol("profile_loaded");
    }
    Message<0> msg(profileLoaded);
    Export(msg.mData);
}

// Inlined into Handle (0x3B3B70).
void ProfileMgr::_OnSaveEnd(int error, int padNum) {
    if (error == SaveLoad::kUnknownError) {
        _OnProfileFailedToSave(padNum);
    }
}

// Inlined into Handle (0x3B3B70).
DataNode ProfileMgr::OnMsg(const UserSignedInMsg& msg) {
    _OnUserLoggedIn(msg.GetUser());
    return DataNode(0);
}

// Inlined into Handle (0x3B3B70).
DataNode ProfileMgr::OnMsg(const UserSignedOutMsg& msg) {
    const int user = msg.GetUser();
    for (Profile* const profile : mProfiles) {
        if (profile->mPadNum == user) {
            if (profile != nullptr) {
                DestroyProfile(profile);
            }
            break;
        }
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x3B3780.
DataNode ProfileMgr::OnMsg(const AppSuspendStateChangedMsg& msg) {
    if (!msg.IsSuspended()) {
        eastl::vector<Profile*> signedOut;
        for (Profile* const profile : mProfiles) {
            if (!ThePlatformMgr->IsSignedIn(profile->mPadNum)) {
                signedOut.push_back(profile);
            }
        }
        for (Profile* const profile : signedOut) {
            DestroyProfile(profile);
        }
        ThePlatformMgr->RefreshSignedInUsers(this);
        _OnAppResumed();
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x3B39B0.
DataNode ProfileMgr::OnMsg(const PlatformIdentityChangedMsg& msg) {
    static_cast<void>(msg);
    eastl::vector<Profile*> profiles;
    for (Profile* const profile : mProfiles) {
        profiles.push_back(profile);
    }
    for (Profile* const profile : profiles) {
        DestroyProfile(profile);
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x3B3B70.
DataNode ProfileMgr::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    if (type == UserSignedInMsg::Event()) {
        DataNode result = OnMsg(UserSignedInMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    if (type == UserSignedOutMsg::Event()) {
        DataNode result = OnMsg(UserSignedOutMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    if (type == AppSuspendStateChangedMsg::Event()) {
        DataNode result = OnMsg(AppSuspendStateChangedMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    if (type == PlatformIdentityChangedMsg::Event()) {
        DataNode result = OnMsg(PlatformIdentityChangedMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol loadEnd;
    if (loadEnd == Symbol()) {
        loadEnd = Symbol("load_end");
    }
    if (type == loadEnd) {
        _OnLoadEnd(msg->Int(2), msg->Int(3));
        return DataNode(0);
    }
    static Symbol saveEnd;
    if (saveEnd == Symbol()) {
        saveEnd = Symbol("save_end");
    }
    if (type == saveEnd) {
        _OnSaveEnd(msg->Int(2), msg->Int(3));
        return DataNode(0);
    }
    static Symbol onSystemOptionsLoaded;
    if (onSystemOptionsLoaded == Symbol()) {
        onSystemOptionsLoaded = Symbol("on_system_options_loaded");
    }
    if (type == onSystemOptionsLoaded) {
        DataNode result = GetSystemOptions()->Handle(msg, true);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    return DataNode();
}
