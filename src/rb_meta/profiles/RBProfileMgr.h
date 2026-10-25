#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/profiles/ProfileMgr.h"
#include "rb_meta/profiles/RBSystemOptions.h"
#include "rb_meta/state/RBMetaStateCom.h"
#include "utl/containers/Map.h"
#include "utl/containers/Set.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

class ConnectionStatusChangedMsg;
class DataArray;
class GameCreatedMsg;
class GamePreEndedMsg;
class GameStartedMsg;
class NamedThread;
class PlatformEntitlementsRetrievedMsg;
class Profile;
class RBProfile;

// The game's profile manager (rb_meta/RBProfileMgr.o, 0xD5B320-0xD62D4F
// with the static initializer at 0xD62C80). It creates an RBProfile for
// each signed-in pad, assigns the profiles to the four band slots, queues
// profiles whose saves fail for destruction, and boots to the main menu
// with a dialog when a critical profile drops out or a save fails. It
// tracks whether the band is in a game (the rich presence mode), resolves
// the profiles that joined during a song once the menus return, and keeps
// the console's options. A worker thread ("rbprofile_signed_in_poll")
// records which profiles are signed into the network; the manager's lock
// guards the profiles against it. The vtable at 0x197DEB0 has 16 slots;
// the object is 1144 bytes from the "profile" heap.
class RBProfileMgr : public ProfileMgr {
public:
    // What the band is doing, for the rich presence. Names not in the
    // reference map; the evidence for them is weak.
    enum ProfileMgrMode : int {
        kModeNone = 0,
        kModeMenus = 1,
        kModeGame = 2,
    };

    // The dialogs _ShowDialog shows. Names not in the reference map.
    enum ProfileMgrDialogType : int {
        // "critical_user_drop_out".
        kDialogCriticalUserDropOut = 0,
        // "save_data_failed_save".
        kDialogSaveFailed = 1,
        // "save_data_failed_load".
        kDialogLoadFailed = 2,
    };

    // A profile the current activity needs, with the number of times it
    // was added; the map names the type.
    struct CriticalProfile {
        RBProfile* mProfile;
        int mCount;
    };

    // A band slot's profile; the map names the type. Field names are not
    // in the reference map.
    struct ProfileData {
        ProfileData() : mProfile(nullptr), mPendingProfile(nullptr), mOwnsSettings(true) {}

        // The slot's profile.
        RBProfile* mProfile;
        // A profile assigned during a song, which takes the slot when the
        // band returns to the menus (_ResolvePendingProfiles).
        RBProfile* mPendingProfile;
        // Whether the slot was the first to hold the profile, so that its
        // settings follow the profile.
        bool mOwnsSettings;
    };

    // Whether a pad's profile is signed into the network, as the worker
    // thread reads it. Name and field names not in the reference map.
    struct SignedInState {
        int mPadNum;
        bool mSignedIntoLive;
    };

    // The manager comes from the "profile" heap.
    static void* operator new(unsigned long size) {
        static const long sHeap = MemFindHeap("profile");
        MemPushHeap(sHeap);
        void* const allocation = MemAlloc(size, "RBProfileMgr", 0);
        MemPopHeap();
        return allocation;
    }
    static void operator delete(void* allocation) {
        MemFree(allocation);
    }

    RBProfileMgr();  // 0xD5B320
    // Slots 0-1: 0xD5B6C0, 0xD5BAE0. Stops and deletes the worker thread
    // and leaves the events.
    ~RBProfileMgr() override;

    // Slot 2: the game's, the platform's, the song manager's, the system
    // options' and the UI's events, then ProfileMgr's messages.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xD61F20
    // Slot 7.
    SystemOptions* GetSystemOptions() override;  // 0xD62560
    // Slot 8: "rb_system_settings.bin".
    const char* GetSystemSaveFileName() const override;  // 0xD62570
    // Slot 9: updates the new profile's rich presence.
    void _OnProfileAdded(Profile* profile) override;  // 0xD5F0E0
    // Slot 10: a critical profile's removal boots to the main menu; the
    // profile leaves the pending slots and the destruction queue.
    void _OnProfileAboutToBeRemoved(Profile* profile) override;  // 0xD5F780
    // Slot 11: reports the failure (on the title screen at once, elsewhere
    // after booting to the main menu) and queues the pad's profile for
    // destruction.
    void _OnProfileFailedToLoad(int padNum) override;  // 0xD5F9E0
    // Slot 12: reports the failure like a failed load, without dropping
    // the profile.
    void _OnProfileFailedToSave(int padNum) override;  // 0xD5FF80
    // Slot 14: creates and loads the pad's profile.
    void _OnUserLoggedIn(int padNum) override;  // 0xD5F070
    // Slot 15: resets every profile's server state. Name not in the
    // reference map.
    void _OnAppResumed() override;  // 0xD60010

    // Creates the manager, sets it as the "profile_mgr" global, subscribes
    // to the events and starts the worker thread.
    static RBProfileMgr* Init();  // 0xD5BB00
    // Runs the profiles' updates and the manager's checks under the
    // manager's lock.
    void Poll();  // 0xD5C230

    // The profile of a pad, among the slots. Name not in the reference
    // map.
    RBProfile* GetSlotProfileFromPadNum(int padNum) const;  // 0xD5D000
    // The slot's profile. The binary reads the slot without checking that
    // it exists; a missing slot is treated as empty.
    RBProfile* GetProfileFromSlotIdx(unsigned long slot) const;  // 0xD5CEB0
    // Whether the slot's settings are the profile's: true when the slot
    // owns them, or when no earlier slot holds the profile and no other
    // slot owns them.
    bool DoesSlotOwnProfileSettings(unsigned long slot) const;  // 0xD5D0E0
    // The first slot that holds the profile, or -1.
    unsigned long GetSlotIdxFromProfile(RBProfile* profile) const;  // 0xD5D3D0
    // The first slot that holds the profile and owns its settings, or -1.
    // Name not in the reference map.
    unsigned long GetSettingsSlotIdxFromProfile(RBProfile* profile) const;  // 0xD5D440
    // Puts the profile in the slot. During a song the profile waits as the
    // slot's pending profile; otherwise its pad is checked for a finished
    // load ("profile_dropped_in").
    void AssignProfileToSlot(RBProfile* profile, unsigned long slot);  // 0xD5D4C0
    // Empties the slot, exporting "profile_about_to_drop_out" when no other
    // slot holds its profile.
    void RemoveProfileFromSlot(unsigned long slot);  // 0xD5D740
    // A copy of the manager's profiles. Name not in the reference map.
    eastl::vector<RBProfile*> GetProfiles() const;  // 0xD5DAC0
    // The profiles no slot holds.
    eastl::vector<RBProfile*> GetAvailableProfiles() const;  // 0xD5DCA0
    // The slots' profiles, each once.
    eastl::vector<RBProfile*> GetParticipatingProfiles() const;  // 0xD5E050
    // Adds the slots' profiles that the vector lacks.
    void _GetParticipatingProfiles(eastl::vector<RBProfile*>& profiles) const;  // 0xD5E110
    // The campaign's profile, then the slots' profiles. Not reconstructed:
    // it reads the campaign manager.
    eastl::vector<RBProfile*> GetCampaignParticipatingProfiles() const;  // 0xD5E270
    // The current session's profile, then the slots' profiles. Not
    // reconstructed: it reads the session manager. Name not in the
    // reference map.
    eastl::vector<RBProfile*> GetSessionParticipatingProfiles() const;  // 0xD5E400
    // The profile whose platform user id, as text, is the symbol. Not
    // reconstructed: the id's formatter (0x11AE630) is not modelled. Name
    // not in the reference map.
    RBProfile* GetProfileFromOnlineId(const Symbol& onlineId) const;  // 0xD5CF30

    // Counts one more need for the profile.
    void AddCriticalProfile(RBProfile* profile);  // 0xD5E580
    // The pad's profile.
    void AddCriticalProfile(int padNum);  // 0xD5E770
    // Counts one need less; the last one drops the entry.
    void RemoveCriticalProfile(RBProfile* profile);  // 0xD5E810
    // The pad's profile.
    void RemoveCriticalProfile(int padNum);  // 0xD5E8D0
    // Inlined into _OnProfileAboutToBeRemoved.
    bool IsCriticalProfile(RBProfile* profile) const;

    // Export "show_xp_widgets" and "hide_xp_widgets". Names not in the
    // reference map.
    void ShowXpWidgets();  // 0xD5E9B0
    void HideXpWidgets();  // 0xD5EA70
    // Export "show_xp_widget_slot" and "hide_xp_widget_slot" with the slot.
    // Names not in the reference map.
    void ShowXpWidgetForSlot(int slot);  // 0xD5EB30
    void HideXpWidgetForSlot(int slot);  // 0xD5ECB0
    // Shows "solo_no_server_warning" once. Name not in the reference map.
    void ShowSoloNoServerWarning();  // 0xD5EE30
    // Whether a participating profile started its mini leaderboard
    // submissions. Name not in the reference map; the evidence is weak.
    bool AnyParticipatingProfileSubmittingScores() const;  // 0xD61270

    // Subscribes to the game's, the song manager's, the system options'
    // and the platform's events. Name not in the reference map.
    void _RegisterForEvents();  // 0xD5BBF0
    // Starts the worker thread that polls the network sign-in. Name not in
    // the reference map.
    void _StartSignedInPollThread();  // 0xD5C070
    // The worker thread: polls the sign-in every half second until the
    // manager is destroyed. Name not in the reference map.
    static int _SignedInPollThreadProc(void* context);  // 0xD5C1A0
    // Asks the platform which profiles are signed into the network, with
    // the manager's lock released while it asks. Name not in the reference
    // map.
    static void _PollSignedInState();  // 0xD60840

    // Exports "profile_dropped_in" for each added pad whose profile
    // finished loading; drops pads without a profile.
    void _CheckForAddedProfileLoads();  // 0xD5C3C0
    // Exports "profile_failed_to_saveload" for each queued profile and
    // destroys it.
    void _DestroyQueuedProfiles();  // 0xD5C670
    // On the main menu, shows the dialogs booting there postponed. Inlined
    // into Poll; the binary keeps an uncalled copy.
    void _CheckForMainMenuDialogs();  // 0xD5C900
    // Leaving the game layout returns the mode to the menus. Inlined into
    // Poll; the binary keeps an uncalled copy.
    void _CheckForModeUpdate();  // 0xD5C970
    // Ends the game and goes to the main menu once a boot was requested
    // during the game. Inlined into Poll; the binary keeps an uncalled
    // copy. Not reconstructed: it reads the game's state.
    void _CheckForMainMenuBootFromGame();  // 0xD5C9B0
    // Shows "error_lost_connection" once the band is back in the menus
    // after losing the server. Name not in the reference map.
    void _CheckForLostConnection();  // 0xD5CA10
    // In the menus, with the server and the network up, starts the
    // entitlement jobs of the profiles that need them. Name not in the
    // reference map.
    void _CheckForPendingServerJobs();  // 0xD5CCC0
    // Likewise sends the playable songs of the profiles that have not sent
    // them. Name not in the reference map.
    void _CheckForSongUploads();  // 0xD5CD90
    // A guest drops its entitlements; a loaded, signed-in profile that is
    // not waiting starts its jobs. Name not in the reference map.
    void _UpdateProfileServerJobs(RBProfile* profile);  // 0xD613A0
    // Whether one of the profile's entitlement requests is out. Inlined
    // into _UpdateProfileServerJobs; the binary keeps an uncalled copy.
    // Name not in the reference map.
    bool _IsProfileWaitingForServer(RBProfile* profile) const;  // 0xD61480
    // Inlined into _UpdateProfileServerJobs; the binary keeps an uncalled
    // copy. Name not in the reference map.
    void _StartProfileServerJobs(RBProfile* profile);  // 0xD614A0
    // Asks the platform for the profile's entitlements (PlatformMgr vtable
    // +464). Uncalled in this build; not reconstructed: the platform slot
    // and the entitlement table are not modelled. Name not in the
    // reference map.
    void _RequestPlatformEntitlements(RBProfile* profile);  // 0xD61520
    // Queues an RBNetEntitlementsJob. Inlined; the binary keeps an uncalled
    // copy. Name not in the reference map.
    void _StartEntitlementsJob(RBProfile* profile);  // 0xD61700
    // _CheckForSongUploads' per-profile step. Inlined; the binary keeps an
    // uncalled copy. Name not in the reference map.
    void _UpdateSongUpload(RBProfile* profile);  // 0xD61760
    // Queues an RBNetUpdateSongsJob with the playable songs. Name not in
    // the reference map.
    void _StartSongUploadJob(RBProfile* profile);  // 0xD617D0
    // Marks every profile's songs as not sent. Inlined into the connection
    // handler; the binary keeps an uncalled copy. Name not in the
    // reference map.
    void _ResetSongUploads();  // 0xD61130
    // The entitlement job finished: stores the entitlements on success.
    // Name not in the reference map.
    void OnEntitlementsJobDone(int padNum, bool success, const eastl::set<int>& entitlements);  // 0xD61C40
    // The song job finished: marks the songs sent on success. Name not in
    // the reference map.
    void OnSongUploadJobDone(int padNum, bool success);  // 0xD61D50

    // Updates the platform's rich presence of the profile for the mode. Not
    // reconstructed: the platform's presence slot and the game data are
    // not modelled.
    void _UpdateRichPresence(RBProfile* profile, ProfileMgrMode mode);  // 0xD5F150
    // Requests a return to the main menu with a postponed dialog: sets the
    // flag, waits for the transition and goes to the main menu.
    void _BootToMainMenu(bool& showDialog);  // 0xD5F8B0
    // Inlined into the failure hooks.
    bool _CanShowSaveLoadFailMsgNow() const;
    // Shows the dialog as a modal OK popup.
    void _ShowDialog(ProfileMgrDialogType type);  // 0xD5FAC0
    // Inlined into the failure hooks.
    bool _ShouldIgnoreSaveLoadFailures() const;
    // Queues the profile for _DestroyQueuedProfiles, once.
    void _AddProfileToDestructionQueue(RBProfile* profile);  // 0xD5FE00
    // Records the band's performance in the profiles at the end of a
    // song. Not reconstructed: it reads the game's players and songs.
    void _UpdateProfileDataFromGameState(int songId);  // 0xD60180
    // Whether a participating profile is signed in. Name not in the
    // reference map.
    bool _AnyParticipatingProfileSignedIn() const;  // 0xD605E0
    // Whether a participating profile is signed in and reached the
    // server. Name not in the reference map.
    bool _AnyParticipatingProfileOnline() const;  // 0xD60710
    // Gives each slot its pending profile and exports
    // "pending_profiles_resolved" when one did.
    void _ResolvePendingProfiles();  // 0xD60C00
    // Changes the mode (or forces the update) and updates every profile's
    // rich presence.
    void _UpdateMode(ProfileMgrMode mode, bool force);  // 0xD60EA0

    // Inlined into Handle; the binary keeps uncalled copies.
    DataNode OnMsg(const GameCreatedMsg& msg);    // 0xD611C0
    DataNode OnMsg(const GameStartedMsg& msg);    // 0xD61200
    DataNode OnMsg(const GamePreEndedMsg& msg);   // 0xD61230
    // A reconnection resets the profiles' server state; a lost connection
    // shows a dialog later. Newer than the map.
    DataNode OnMsg(const ConnectionStatusChangedMsg& msg);  // 0xD60FA0
    // Gives the pad's profile its entitlements. Newer than the map; not
    // reconstructed: the entitlement table is not modelled.
    DataNode OnMsg(const PlatformEntitlementsRetrievedMsg& msg);  // 0xD618A0
    // The song cache and the system options are saved together; a dirty
    // cache marks the profiles and the options dirty. Inlined into Handle;
    // the binary keeps uncalled copies.
    void _OnSongCacheDirty();      // 0xD61E30
    void _OnSystemOptionsDirty();  // 0xD61E80
    // Goes to the main menu after a postponed boot, unless a game is on.
    // Inlined into Handle; the binary keeps an uncalled copy.
    void _OnTransitionDone(const Symbol& from, const Symbol& to);  // 0xD61EC0

    // The console's options. Name not in the reference map.
    RBSystemOptions mSystemOptions;
    // Field names below are not in the reference map.
    // The four band slots.
    eastl::map<unsigned long, ProfileData> mProfileData;
    eastl::vector<CriticalProfile> mCriticalProfiles;
    // Dialogs booting to the main menu postponed, shown on the main menu.
    bool mShowCriticalDropOutDialog;
    bool mShowLoadFailedDialog;
    bool mShowSaveFailedDialog;
    // The pads of profiles assigned to slots while they may still load.
    eastl::vector<int> mAddedProfilePads;
    eastl::vector<RBProfile*> mDestructionQueue;
    ProfileMgrMode mMode;
    // The layout _CheckForModeUpdate saw last.
    RBUILayout mLastLayout;
    // Set when a boot to the main menu finished while the game was on.
    bool mBootToMainMenuFromGame;
    bool mShowLostConnectionDialog;
    // Whether the server was reached; starts true.
    bool mServerConnected;
    // Whether ShowSoloNoServerWarning still shows its dialog.
    bool mShowSoloNoServerWarning;
    NamedThread* mSignedInPollThread;
    MsgSource::EventSinkElem mGameCreatedSink;
    MsgSource::EventSinkElem mGameStartedSink;
    MsgSource::EventSinkElem mPreGameEndedSink;
    MsgSource::EventSinkElem mSongCacheDirtySink;
    MsgSource::EventSinkElem mSystemOptionsDirtySink;
    MsgSource::EventSinkElem mConnectionStatusSink;
    MsgSource::EventSinkElem mTransitionDoneSink;
};

static_assert(sizeof(RBProfileMgr::CriticalProfile) == 16);
static_assert(offsetof(RBProfileMgr::ProfileData, mPendingProfile) == 8);
static_assert(offsetof(RBProfileMgr::ProfileData, mOwnsSettings) == 16);
static_assert(sizeof(RBProfileMgr::SignedInState) == 8);
static_assert(offsetof(RBProfileMgr, mSystemOptions) == 344);
static_assert(offsetof(RBProfileMgr, mProfileData) == 680);
static_assert(offsetof(RBProfileMgr, mCriticalProfiles) == 736);
static_assert(offsetof(RBProfileMgr, mShowCriticalDropOutDialog) == 768);
static_assert(offsetof(RBProfileMgr, mShowSaveFailedDialog) == 770);
static_assert(offsetof(RBProfileMgr, mAddedProfilePads) == 776);
static_assert(offsetof(RBProfileMgr, mDestructionQueue) == 808);
static_assert(offsetof(RBProfileMgr, mMode) == 840);
static_assert(offsetof(RBProfileMgr, mLastLayout) == 844);
static_assert(offsetof(RBProfileMgr, mBootToMainMenuFromGame) == 848);
static_assert(offsetof(RBProfileMgr, mShowSoloNoServerWarning) == 851);
static_assert(offsetof(RBProfileMgr, mSignedInPollThread) == 856);
static_assert(offsetof(RBProfileMgr, mGameCreatedSink) == 864);
static_assert(offsetof(RBProfileMgr, mConnectionStatusSink) == 1064);
static_assert(offsetof(RBProfileMgr, mTransitionDoneSink) == 1104);
static_assert(sizeof(RBProfileMgr) == 1144);

extern RBProfileMgr* theRBProfileMgr;  // 0x1B1F7F0
