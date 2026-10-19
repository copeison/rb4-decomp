#pragma once

#include <cstddef>

#include "utl/containers/Vector.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"

class AppSuspendStateChangedMsg;
class DataArray;
class PlatformIdentityChangedMsg;
class Profile;
class SystemOptions;
class UserSignedInMsg;
class UserSignedOutMsg;

// Owns the signed-in users' profiles (persistence/ProfileMgr.o,
// 0x3B28F0-0x3B44A0). It drops the profiles of users who sign out or whose
// identity changes, forwards the save manager's load and save results to
// the game's hooks, and passes "on_system_options_loaded" to the system
// options. The game derives its manager from it (rb_meta/RBProfileMgr.o)
// and implements the profile creation and the system options. Its exports
// go through a file-static lock. The vtable at 0x18FD270 has 16 slots; the
// object is 344 bytes.
class ProfileMgr : public SyncMsgSource {
public:
    ProfileMgr();  // 0x3B2BD0
    // Slots 0-1: 0x3B3020, 0x3B32B0. Deletes the profiles.
    ~ProfileMgr() override;
    // Slot 2: the platform's sign-in, suspend and identity messages, the
    // save manager's "load_end" and "save_end", and
    // "on_system_options_loaded".
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x3B3B70
    // Slot 7: the system options the game keeps.
    virtual SystemOptions* GetSystemOptions() = 0;
    // Slot 8: the system options' save file.
    virtual const char* GetSystemSaveFileName() const = 0;
    // Slots 9-12: game hooks, empty here. Inline in the map's build; the
    // binary emits them at 0x3B4150-0x3B4180.
    virtual void _OnProfileAdded(Profile* profile) {
        static_cast<void>(profile);
    }
    virtual void _OnProfileAboutToBeRemoved(Profile* profile) {
        static_cast<void>(profile);
    }
    virtual void _OnProfileFailedToLoad(int padNum) {
        static_cast<void>(padNum);
    }
    virtual void _OnProfileFailedToSave(int padNum) {
        static_cast<void>(padNum);
    }
    // Slot 13: whether PostInit subscribes to the save manager's results.
    // Inline, emitted at 0x3B4190. Name not in the reference map; the
    // evidence is weak.
    virtual bool _UsesSaveLoadMgr() {
        return true;
    }
    // Slot 14: a user signed in; the game creates and loads a profile for
    // the pad. The name is the game override's in the map
    // (RBProfileMgr::_OnUserLoggedIn(int)); the evidence is weak.
    virtual void _OnUserLoggedIn(int padNum) = 0;
    // Slot 15: the application resumed and the profiles of signed-out users
    // were dropped. Inline and empty, emitted at 0x3B41A0. Name not in the
    // reference map; the evidence is weak.
    virtual void _OnAppResumed() {}

    // Sets the "profile_mgr" global.
    static void Init(ProfileMgr* mgr);  // 0x3B28F0
    // Calls the manager's PostInit. Its body is a tail call.
    static void PostInit(ProfileMgr* mgr);  // 0x3B29A0
    // Subscribes to the save manager's "load_end", "save_end" and
    // "saveopt_end", the last as "on_system_options_loaded". Name not in
    // the reference map, which has only the static overload.
    void PostInit();  // 0x3B29B0
    // Polls each profile.
    void Poll();  // 0x3B32D0
    // Removes the profile, tells the game and the subscribers
    // ("profile_about_to_be_removed") and deletes it. The profile must be
    // the manager's.
    void DestroyProfile(Profile* profile);  // 0x3B3310
    // Deletes every profile.
    void ResetAll();  // 0x3B3520
    // Always kAnyDevice; the PS4 has one device.
    int GetBestStorageDevice();  // 0x3B3570
    // A profile load finished: reports the pad's failure to the game and
    // exports "profile_loaded".
    void _OnLoadEnd(int error, int padNum);  // 0x3B3580
    // Reports the pad's failure to the game. Inlined into Handle.
    void _OnSaveEnd(int error, int padNum);

    // Inlined into Handle.
    DataNode OnMsg(const UserSignedInMsg& msg);
    // Destroys the user's profile. Inlined into Handle.
    DataNode OnMsg(const UserSignedOutMsg& msg);
    // On resuming, destroys the profiles of users who are no longer signed
    // in.
    DataNode OnMsg(const AppSuspendStateChangedMsg& msg);  // 0x3B3780
    // Destroys every profile. Newer than the map.
    DataNode OnMsg(const PlatformIdentityChangedMsg& msg);  // 0x3B39B0

    // Field names are not in the reference map.
    MsgSource::EventSinkElem mUserSignedInSink;
    MsgSource::EventSinkElem mUserSignedOutSink;
    MsgSource::EventSinkElem mAppSuspendSink;
    MsgSource::EventSinkElem mPlatformIdentitySink;
    MsgSource::EventSinkElem mLoadEndSink;
    MsgSource::EventSinkElem mSaveEndSink;
    MsgSource::EventSinkElem mSaveOptEndSink;
    eastl::vector<Profile*> mProfiles;
};

static_assert(offsetof(ProfileMgr, mUserSignedInSink) == 32);
static_assert(offsetof(ProfileMgr, mLoadEndSink) == 192);
static_assert(offsetof(ProfileMgr, mSaveOptEndSink) == 272);
static_assert(offsetof(ProfileMgr, mProfiles) == 312);
static_assert(sizeof(ProfileMgr) == 344);
