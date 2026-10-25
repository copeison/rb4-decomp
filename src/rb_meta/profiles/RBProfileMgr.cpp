// rb_meta/RBProfileMgr.o (0xD5B320 to 0xD62D4F). The EASTL helpers of the
// slot map, the critical profiles and the sets (0xD62580-0xD62B40) and the
// std::function of ShowSoloNoServerWarning's callback (vtable 0x197DF40,
// 0xD62A40-0xD62B30) are the compiler's. The static initializer is at
// 0xD62C80; the shared header statics it sets first (0x1B1F790-0x1B1F7B8)
// belong to headers that are not modelled. The template that creates the
// profiles is emitted at 0xBB02E0.
#include "rb_meta/profiles/RBProfileMgr.h"

#include <string.h>
#include <unistd.h>

#include "os/platform/PlatformMgr.h"
#include "os/threading/CritSec.h"
#include "persistence/saveload/SaveLoadMgr.h"
#include "rb_meta/dialog/DialogMgrCom.h"
#include "rb_meta/net/RBNetEntitlementsJob.h"
#include "rb_meta/net/RBNetMgr.h"
#include "rb_meta/profiles/RBProfile.h"
#include "rb_meta/songs/SongMgr.h"
#include "ui/manager/UIMgr.h"
#include "utl/data/DataArray.h"
#include "utl/messages/Message.h"
#include "utl/threading/Thread.h"

// Declared here until rb_game/RBGameData.o is reconstructed. The game's
// data (theGameData, a static object) exports the game's lifetime events;
// the messages are inline in its header, and RBProfileMgr.o emits
// GamePreEndedMsg's constructor (0xD62620).
class RBGameData : public MsgSource {};

extern RBGameData theGameData;  // 0x1AE1818

// A game was created: (target "game_created"). The constructor is at
// 0x971500.
class GameCreatedMsg : public Message<0> {
public:
    explicit GameCreatedMsg(DataArray* msg) : Message<0>(msg) {
        static_cast<void>(msg->Node(1).LiteralSym(msg));
        static_cast<void>(Event());
    }

    // The message type, built on first use (0x1AE4638).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("game_created");
        }
        return t;
    }
};

// The game started: (target "game_started" arg). The constructor is at
// 0x971610.
class GameStartedMsg : public Message<1> {
public:
    explicit GameStartedMsg(DataArray* msg) : Message<1>(msg) {
        static_cast<void>(msg->Node(1).LiteralSym(msg));
        static_cast<void>(Event());
    }

    // The message type, built on first use (0x1AE45E8).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("game_started");
        }
        return t;
    }
};

// The game is about to end: (target "pre_game_ended" arg value). The map's
// build has one argument.
class GamePreEndedMsg : public Message<2> {
public:
    // Reconstructed from eboot.elf at 0xD62620.
    explicit GamePreEndedMsg(DataArray* msg) : Message<2>(msg) {
        static_cast<void>(msg->Node(1).LiteralSym(msg));
        static_cast<void>(Event());
    }

    // The message type, built on first use (0x1AE4528).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("pre_game_ended");
        }
        return t;
    }

    // The value RBProfileMgr records the game with (node 3). Name not in
    // the reference map; the evidence is weak.
    int GetValue() const {
        return mData->Int(3);
    }
};

namespace {

// Pushes the "profile" heap for the scope. Each caller keeps its own
// guarded copy of the heap's index. Name not in the reference map.
class ProfileHeapScope {
public:
    explicit ProfileHeapScope(long heap) {
        MemPushHeap(heap);
    }
    ~ProfileHeapScope() {
        MemPopHeap();
    }

    ProfileHeapScope(const ProfileHeapScope&) = delete;
    ProfileHeapScope& operator=(const ProfileHeapScope&) = delete;
};

// The lock Poll holds and the worker thread takes around its reads of the
// profiles. At 0x1B1F7E0; the static initializer constructs it. Name not
// in the reference map.
CritSec gRBProfileMgrCritSec;

// Whether the worker thread keeps polling; the destructor clears it. At
// 0x1B1F808. Name not in the reference map.
bool gSignedInPollRunning;

// The object node of a profile: its MsgSink, or null. Name not in the
// reference map.
DataNode ProfileNode(RBProfile* profile) {
    DataNode node;
    node.mType = kDataObject;
    node.mValue.object = static_cast<MsgSink*>(profile);
    return node;
}

// Exports a message with the profile as its argument. Name not in the
// reference map.
void ExportProfileMsg(MsgSource& source, Symbol type, RBProfile* profile) {
    Message<1> msg(type);
    msg.mData->Node(2) = ProfileNode(profile);
    source.Export(msg.mData);
}

// The manager's profile of the pad, or null. Name not in the reference
// map.
RBProfile* FindProfile(const ProfileMgr& mgr, int padNum) {
    for (Profile* const profile : mgr.mProfiles) {
        if (profile->mPadNum == padNum) {
            return static_cast<RBProfile*>(profile);
        }
    }
    return nullptr;
}

// The profile a slot holds, or null. Name not in the reference map.
RBProfile* SlotProfile(const eastl::map<unsigned long, RBProfileMgr::ProfileData>& slots, unsigned long slot) {
    auto& lookup = const_cast<eastl::map<unsigned long, RBProfileMgr::ProfileData>&>(slots);
    const auto found = lookup.find(slot);
    return found != lookup.end() ? found->second.mProfile : nullptr;
}

// Shows a modal OK popup with the token. Name not in the reference map.
void ShowOkPopup(const Symbol& body, std::function<void(DialogMgrCom::Button, BandUser*)> onHide) {
    DialogMgrCom::DialogArgs args;
    const DialogMgrCom::DialogType type = DialogMgrCom::kDialogTypeOk;
    const DialogMgrCom::DialogModalType modal = DialogMgrCom::kDialogModal;
    const DialogMgrCom::DialogTextLocType bodyLoc = DialogMgrCom::kDialogTextToken;
    const int initialFocus = 0;
    DialogMgrCom::InitForStandardTextPopup(args, type, modal, body, bodyLoc, initialFocus, onHide, {});
    DialogMgrCom::ShowDialog(args);
}

}  // namespace

RBProfileMgr* theRBProfileMgr;

// Reconstructed from eboot.elf at 0xBB02E0. The template is emitted with
// the meta state's object; it creates the pad's profile unless the pad has
// one, loads it and exports "profile_signed_in".
template <class T>
T* ProfileMgr::CreateProfile(int padNum) {
    if (padNum == -1) {
        return nullptr;
    }
    for (Profile* const profile : mProfiles) {
        if (profile->mPadNum == padNum) {
            if (profile != nullptr) {
                return static_cast<T*>(profile);
            }
            break;
        }
    }
    T* const profile = new T(padNum);
    mProfiles.push_back(profile);
    if (_UsesSaveLoadMgr()) {
        theSaveLoadMgr->Load(profile, true);
    }
    _OnProfileAdded(profile);
    static Symbol profileSignedIn;
    if (profileSignedIn == Symbol()) {
        profileSignedIn = Symbol("profile_signed_in");
    }
    Message<1> msg(profileSignedIn);
    DataNode node;
    node.mType = kDataObject;
    node.mValue.object = static_cast<MsgSink*>(profile);
    msg.mData->Node(2) = node;
    Export(msg.mData);
    return profile;
}

// Reconstructed from eboot.elf at 0xD5B320.
RBProfileMgr::RBProfileMgr()
    : mShowCriticalDropOutDialog(false),
      mShowLoadFailedDialog(false),
      mShowSaveFailedDialog(false),
      mMode(kModeNone),
      mLastLayout(kLayoutInvalid),
      mBootToMainMenuFromGame(false),
      mShowLostConnectionDialog(false),
      mServerConnected(true),
      mShowSoloNoServerWarning(true),
      mSignedInPollThread(nullptr) {
    for (MsgSource::EventSinkElem* sink : {&mGameCreatedSink,
                                           &mGameStartedSink,
                                           &mPreGameEndedSink,
                                           &mSongCacheDirtySink,
                                           &mSystemOptionsDirtySink,
                                           &mConnectionStatusSink,
                                           &mTransitionDoneSink}) {
        sink->mSink = nullptr;
        sink->mSource = nullptr;
    }
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    for (unsigned long slot = 0; slot < 4; ++slot) {
        mProfileData[slot] = ProfileData();
    }
}

// Reconstructed from eboot.elf at 0xD5B6C0. The deleting destructor is at
// 0xD5BAE0. A linked subscription leaves its source before the link
// unlinks itself; the binary does this for each subscription as it is
// destroyed, the later ones first.
RBProfileMgr::~RBProfileMgr() {
    theRBProfileMgr = nullptr;
    gSignedInPollRunning = false;
    if (mSignedInPollThread != nullptr) {
        mSignedInPollThread->mThread._Join();
        if (mSignedInPollThread != nullptr) {
            mSignedInPollThread->mThread._ForceKillThread();
            delete mSignedInPollThread;
        }
        mSignedInPollThread = nullptr;
    }
    for (MsgSource::EventSinkElem* sink : {&mTransitionDoneSink,
                                           &mConnectionStatusSink,
                                           &mSystemOptionsDirtySink,
                                           &mSongCacheDirtySink,
                                           &mPreGameEndedSink,
                                           &mGameStartedSink,
                                           &mGameCreatedSink}) {
        LinkedList::Node& link = sink->mLink;
        if (link.mNext != &link && link.mPrev != &link) {
            sink->mSource->RemoveSink(sink);
        }
    }
}

// Reconstructed from eboot.elf at 0xD5BB00.
RBProfileMgr* RBProfileMgr::Init() {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    theRBProfileMgr = new RBProfileMgr();
    ProfileMgr::Init(theRBProfileMgr);
    theRBProfileMgr->_RegisterForEvents();
    theRBProfileMgr->_StartSignedInPollThread();
    return theRBProfileMgr;
}

// Reconstructed from eboot.elf at 0xD5BBF0.
void RBProfileMgr::_RegisterForEvents() {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    theGameData.AddSink(&mGameCreatedSink, this, GameCreatedMsg::Event(), Symbol());
    theGameData.AddSink(&mGameStartedSink, this, GameStartedMsg::Event(), Symbol());
    theGameData.AddSink(&mPreGameEndedSink, this, GamePreEndedMsg::Event(), Symbol());
    static Symbol songCacheDirty;
    if (songCacheDirty == Symbol()) {
        songCacheDirty = Symbol("song_cache_dirty");
    }
    static Symbol onSongCacheDirty;
    if (onSongCacheDirty == Symbol()) {
        onSongCacheDirty = Symbol("on_song_cache_dirty");
    }
    theSongMgr->AddSink(&mSongCacheDirtySink, this, songCacheDirty, onSongCacheDirty);
    static Symbol systemOptionsDirty;
    if (systemOptionsDirty == Symbol()) {
        systemOptionsDirty = Symbol("system_options_dirty");
    }
    static Symbol onSystemOptionsDirty;
    if (onSystemOptionsDirty == Symbol()) {
        onSystemOptionsDirty = Symbol("on_system_options_dirty");
    }
    theRBProfileMgr->mSystemOptions.AddSink(
        &mSystemOptionsDirtySink, this, systemOptionsDirty, onSystemOptionsDirty);
    ThePlatformMgr->AddSink(&mConnectionStatusSink, this, ConnectionStatusChangedMsg::Event(), Symbol());
}

// Reconstructed from eboot.elf at 0xD5C070.
void RBProfileMgr::_StartSignedInPollThread() {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    gSignedInPollRunning = true;
    NamedThread* const thread = new NamedThread;
    thread->Init("Unknown Thread!");
    mSignedInPollThread = thread;
    thread->Create(
        _SignedInPollThreadProc,
        nullptr,
        "rbprofile_signed_in_poll",
        0,
        static_cast<Thread::ThreadPriority>(Thread::kPriorityDefault + 1),
        0,
        0);
    mSignedInPollThread->mThread.Start();
}

// Reconstructed from eboot.elf at 0xD5C1A0.
int RBProfileMgr::_SignedInPollThreadProc(void* context) {
    static_cast<void>(context);
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    if (gSignedInPollRunning) {
        do {
            _PollSignedInState();
            usleep(500000);
        } while (gSignedInPollRunning);
    }
    return 0;
}

// Reconstructed from eboot.elf at 0xD5C230.
void RBProfileMgr::Poll() {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    ScopedCritSec lock(gRBProfileMgrCritSec);
    ProfileMgr::Poll();
    _CheckForAddedProfileLoads();
    _DestroyQueuedProfiles();
    _CheckForMainMenuDialogs();
    _CheckForModeUpdate();
    _CheckForMainMenuBootFromGame();
    _CheckForLostConnection();
    _CheckForPendingServerJobs();
    _CheckForSongUploads();
}

// Reconstructed from eboot.elf at 0xD5C3C0.
void RBProfileMgr::_CheckForAddedProfileLoads() {
    int* pad = mAddedProfilePads.begin();
    while (pad != mAddedProfilePads.end()) {
        RBProfile* const profile = FindProfile(*this, *pad);
        if (profile == nullptr) {
            mAddedProfilePads.erase(pad);
        } else if (!profile->mReadyToSave) {
            ++pad;
        } else {
            static Symbol profileDroppedIn;
            if (profileDroppedIn == Symbol()) {
                profileDroppedIn = Symbol("profile_dropped_in");
            }
            ExportProfileMsg(*this, profileDroppedIn, profile);
            mAddedProfilePads.erase(pad);
        }
    }
}

// Reconstructed from eboot.elf at 0xD5C670.
void RBProfileMgr::_DestroyQueuedProfiles() {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    while (!mDestructionQueue.empty()) {
        RBProfile* const profile = mDestructionQueue.front();
        static Symbol profileFailedToSaveload;
        if (profileFailedToSaveload == Symbol()) {
            profileFailedToSaveload = Symbol("profile_failed_to_saveload");
        }
        Message<1> msg(profileFailedToSaveload);
        msg.mData->Node(2) = ProfileNode(profile);
        Export(msg.mData);
        mDestructionQueue.erase(mDestructionQueue.begin());
        DestroyProfile(profile);
    }
}

// Reconstructed from eboot.elf at 0xD5C900.
void RBProfileMgr::_CheckForMainMenuDialogs() {
    if (theRBMetaState.mCurrentLayout != kLayoutMainMenu) {
        return;
    }
    if (mShowCriticalDropOutDialog) {
        mShowCriticalDropOutDialog = false;
        _ShowDialog(kDialogCriticalUserDropOut);
    }
    if (mShowLoadFailedDialog) {
        mShowLoadFailedDialog = false;
        _ShowDialog(kDialogLoadFailed);
    }
    if (mShowSaveFailedDialog) {
        mShowSaveFailedDialog = false;
        _ShowDialog(kDialogSaveFailed);
    }
}

// Reconstructed from eboot.elf at 0xD5C970.
void RBProfileMgr::_CheckForModeUpdate() {
    const RBUILayout layout = theRBMetaState.mCurrentLayout;
    if (layout != mLastLayout) {
        if (layout != kLayoutGame) {
            _UpdateMode(kModeMenus, false);
        }
        mLastLayout = layout;
    }
}

// Reconstructed from eboot.elf at 0xD5CA10. The dialog waits for the
// menus (outside the loading and game layouts), the results screen or the
// title.
void RBProfileMgr::_CheckForLostConnection() {
    if (mServerConnected && _AnyParticipatingProfileSignedIn() && !_AnyParticipatingProfileOnline()) {
        mShowLostConnectionDialog = true;
        mServerConnected = false;
    }
    if (!mShowLostConnectionDialog) {
        return;
    }
    const RBUILayout layout = theRBMetaState.mCurrentLayout;
    if ((mMode == kModeMenus && layout != kLayoutLoading && layout != kLayoutGame)
        || theRBMetaState.GetMetaGameplayState() == kMetaGameplayStateOnResultsScreen || layout == kLayoutTitle) {
        static Symbol errorLostConnection;
        if (errorLostConnection == Symbol()) {
            errorLostConnection = Symbol("error_lost_connection");
        }
        ShowOkPopup(errorLostConnection, {});
        mShowLostConnectionDialog = false;
    }
}

// Reconstructed from eboot.elf at 0xD5CCC0. Only the menus fetch. The
// binary tests PlatformMgr_PS4.o's copy of the manager pointer
// (0x19FE980).
void RBProfileMgr::_CheckForPendingServerJobs() {
    static_cast<void>(scePthreadSelf());
    if (ThePlatformMgr == nullptr || mMode != kModeMenus || !mServerConnected
        || !ThePlatformMgr->IsEthernetCableConnected()) {
        return;
    }
    const eastl::vector<RBProfile*> profiles = GetProfiles();
    for (RBProfile* const profile : profiles) {
        _UpdateProfileServerJobs(profile);
    }
}

// Reconstructed from eboot.elf at 0xD5CD90.
void RBProfileMgr::_CheckForSongUploads() {
    static_cast<void>(scePthreadSelf());
    if (mMode != kModeMenus || !mServerConnected || !ThePlatformMgr->IsEthernetCableConnected()) {
        return;
    }
    const eastl::vector<RBProfile*> profiles = GetProfiles();
    for (RBProfile* const profile : profiles) {
        _UpdateSongUpload(profile);
    }
}

// Reconstructed from eboot.elf at 0xD5CEB0.
RBProfile* RBProfileMgr::GetProfileFromSlotIdx(unsigned long slot) const {
    auto& profiles = const_cast<eastl::map<unsigned long, ProfileData>&>(mProfileData);
    const auto it = profiles.find(slot);
    return it != profiles.end() ? it->second.mProfile : nullptr;
}

// Reconstructed from eboot.elf at 0xD5D000.
RBProfile* RBProfileMgr::GetSlotProfileFromPadNum(int padNum) const {
    for (auto& entry : const_cast<eastl::map<unsigned long, ProfileData>&>(mProfileData)) {
        RBProfile* const profile = entry.second.mProfile;
        if (profile != nullptr && profile->mPadNum == padNum) {
            return profile;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0xD5D0E0. The binary reads the slot
// without checking that it exists; a missing slot is treated as empty.
bool RBProfileMgr::DoesSlotOwnProfileSettings(unsigned long slot) const {
    auto& slots = const_cast<eastl::map<unsigned long, ProfileData>&>(mProfileData);
    const auto found = slots.find(slot);
    if (found != slots.end() && found->second.mOwnsSettings) {
        return true;
    }
    RBProfile* const profile = found != slots.end() ? found->second.mProfile : nullptr;
    eastl::vector<unsigned long> sharing;
    sharing.reserve(4);
    for (auto& entry : slots) {
        if (entry.first != slot && entry.second.mProfile == profile) {
            if (entry.second.mOwnsSettings) {
                return false;
            }
            sharing.push_back(entry.first);
        }
    }
    for (const unsigned long other : sharing) {
        if (other < slot) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0xD5D3D0.
unsigned long RBProfileMgr::GetSlotIdxFromProfile(RBProfile* profile) const {
    if (profile != nullptr) {
        for (auto& entry : const_cast<eastl::map<unsigned long, ProfileData>&>(mProfileData)) {
            if (entry.second.mProfile == profile) {
                return entry.first;
            }
        }
    }
    return static_cast<unsigned long>(-1);
}

// Reconstructed from eboot.elf at 0xD5D440.
unsigned long RBProfileMgr::GetSettingsSlotIdxFromProfile(RBProfile* profile) const {
    if (profile != nullptr) {
        for (auto& entry : const_cast<eastl::map<unsigned long, ProfileData>&>(mProfileData)) {
            if (entry.second.mProfile == profile && DoesSlotOwnProfileSettings(entry.first)) {
                return entry.first;
            }
        }
    }
    return static_cast<unsigned long>(-1);
}

// Reconstructed from eboot.elf at 0xD5D4C0.
void RBProfileMgr::AssignProfileToSlot(RBProfile* profile, unsigned long slot) {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    bool ownsSettings = true;
    if (profile != nullptr) {
        for (auto& entry : mProfileData) {
            if (entry.second.mProfile == profile && entry.first != slot) {
                ownsSettings = false;
                break;
            }
        }
    }
    ProfileData& data = mProfileData[slot];
    if (mMode == kModeGame && theRBMetaState.GetMetaGameplayState() != kMetaGameplayStateOnResultsScreen) {
        data.mProfile = nullptr;
        data.mPendingProfile = profile;
        data.mOwnsSettings = ownsSettings;
    } else {
        data.mProfile = profile;
        data.mPendingProfile = nullptr;
        data.mOwnsSettings = ownsSettings;
        if (profile != nullptr) {
            mAddedProfilePads.push_back(profile->mPadNum);
        }
    }
}

// Reconstructed from eboot.elf at 0xD5D740.
void RBProfileMgr::RemoveProfileFromSlot(unsigned long slot) {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    RBProfile* const profile = mProfileData[slot].mProfile;
    if (profile != nullptr) {
        bool onlySlot = true;
        for (auto& entry : mProfileData) {
            if (entry.second.mProfile == profile && entry.first != slot) {
                onlySlot = false;
            }
        }
        if (onlySlot) {
            static Symbol profileAboutToDropOut;
            if (profileAboutToDropOut == Symbol()) {
                profileAboutToDropOut = Symbol("profile_about_to_drop_out");
            }
            ExportProfileMsg(*this, profileAboutToDropOut, profile);
        }
    }
    mProfileData[slot] = ProfileData();
}

// Reconstructed from eboot.elf at 0xD5DAC0.
eastl::vector<RBProfile*> RBProfileMgr::GetProfiles() const {
    eastl::vector<RBProfile*> profiles;
    profiles.reserve(mProfiles.size());
    for (Profile* const profile : mProfiles) {
        profiles.push_back(static_cast<RBProfile*>(profile));
    }
    return profiles;
}

// Reconstructed from eboot.elf at 0xD5DCA0. The binary unrolls the
// lookups of the four slots and reads them without checking that they
// exist.
eastl::vector<RBProfile*> RBProfileMgr::GetAvailableProfiles() const {
    eastl::vector<RBProfile*> profiles;
    profiles.reserve(mProfiles.size());
    for (Profile* const profile : mProfiles) {
        bool inSlot = false;
        for (unsigned long slot = 0; slot < 4 && !inSlot; ++slot) {
            const RBProfile* const slotProfile = SlotProfile(mProfileData, slot);
            inSlot = slotProfile != nullptr && slotProfile->mPadNum == profile->mPadNum;
        }
        if (!inSlot) {
            profiles.push_back(static_cast<RBProfile*>(profile));
        }
    }
    return profiles;
}

// Reconstructed from eboot.elf at 0xD5E050.
eastl::vector<RBProfile*> RBProfileMgr::GetParticipatingProfiles() const {
    eastl::vector<RBProfile*> profiles;
    profiles.reserve(4);
    _GetParticipatingProfiles(profiles);
    return profiles;
}

// Reconstructed from eboot.elf at 0xD5E110.
void RBProfileMgr::_GetParticipatingProfiles(eastl::vector<RBProfile*>& profiles) const {
    for (auto& entry : const_cast<eastl::map<unsigned long, ProfileData>&>(mProfileData)) {
        RBProfile* const profile = entry.second.mProfile;
        if (profile == nullptr) {
            continue;
        }
        RBProfile** found = profiles.begin();
        while (found != profiles.end() && *found != profile) {
            ++found;
        }
        if (found == profiles.end()) {
            profiles.push_back(profile);
        }
    }
}

// Reconstructed from eboot.elf at 0xD5E580.
void RBProfileMgr::AddCriticalProfile(RBProfile* profile) {
    if (profile == nullptr) {
        return;
    }
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    CriticalProfile* entry = nullptr;
    for (CriticalProfile& critical : mCriticalProfiles) {
        if (critical.mProfile == profile) {
            entry = &critical;
            break;
        }
    }
    if (entry == nullptr) {
        mCriticalProfiles.push_back(CriticalProfile());
        entry = &mCriticalProfiles.back();
        entry->mProfile = profile;
    }
    ++entry->mCount;
}

// Reconstructed from eboot.elf at 0xD5E770.
void RBProfileMgr::AddCriticalProfile(int padNum) {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    AddCriticalProfile(FindProfile(*this, padNum));
}

// Reconstructed from eboot.elf at 0xD5E810.
void RBProfileMgr::RemoveCriticalProfile(RBProfile* profile) {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    CriticalProfile* entry = mCriticalProfiles.begin();
    while (entry != mCriticalProfiles.end() && entry->mProfile != profile) {
        ++entry;
    }
    if (entry != mCriticalProfiles.end() && entry->mCount-- == 1) {
        mCriticalProfiles.erase(entry);
    }
}

// Reconstructed from eboot.elf at 0xD5E8D0.
void RBProfileMgr::RemoveCriticalProfile(int padNum) {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    RemoveCriticalProfile(FindProfile(*this, padNum));
}

// Inlined into _OnProfileAboutToBeRemoved (0xD5F780).
bool RBProfileMgr::IsCriticalProfile(RBProfile* profile) const {
    for (const CriticalProfile& critical : mCriticalProfiles) {
        if (critical.mProfile == profile) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xD5E9B0. The exports go through the
// global manager.
void RBProfileMgr::ShowXpWidgets() {
    static Symbol showXpWidgets;
    if (showXpWidgets == Symbol()) {
        showXpWidgets = Symbol("show_xp_widgets");
    }
    Message<0> msg(showXpWidgets);
    theRBProfileMgr->Export(msg.mData);
}

// Reconstructed from eboot.elf at 0xD5EA70.
void RBProfileMgr::HideXpWidgets() {
    static Symbol hideXpWidgets;
    if (hideXpWidgets == Symbol()) {
        hideXpWidgets = Symbol("hide_xp_widgets");
    }
    Message<0> msg(hideXpWidgets);
    theRBProfileMgr->Export(msg.mData);
}

// Reconstructed from eboot.elf at 0xD5EB30.
void RBProfileMgr::ShowXpWidgetForSlot(int slot) {
    static Symbol showXpWidgetSlot;
    if (showXpWidgetSlot == Symbol()) {
        showXpWidgetSlot = Symbol("show_xp_widget_slot");
    }
    Message<1> msg(showXpWidgetSlot);
    msg.mData->Node(2) = DataNode(slot);
    theRBProfileMgr->Export(msg.mData);
}

// Reconstructed from eboot.elf at 0xD5ECB0.
void RBProfileMgr::HideXpWidgetForSlot(int slot) {
    static Symbol hideXpWidgetSlot;
    if (hideXpWidgetSlot == Symbol()) {
        hideXpWidgetSlot = Symbol("hide_xp_widget_slot");
    }
    Message<1> msg(hideXpWidgetSlot);
    msg.mData->Node(2) = DataNode(slot);
    theRBProfileMgr->Export(msg.mData);
}

// Reconstructed from eboot.elf at 0xD5EE30. The callback does nothing.
void RBProfileMgr::ShowSoloNoServerWarning() {
    static Symbol soloNoServerWarning;
    if (soloNoServerWarning == Symbol()) {
        soloNoServerWarning = Symbol("solo_no_server_warning");
    }
    ShowOkPopup(soloNoServerWarning, [](DialogMgrCom::Button, BandUser*) {});
    mShowSoloNoServerWarning = false;
}

// Reconstructed from eboot.elf at 0xD5F070.
void RBProfileMgr::_OnUserLoggedIn(int padNum) {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    CreateProfile<RBProfile>(padNum);
}

// Reconstructed from eboot.elf at 0xD5F0E0.
void RBProfileMgr::_OnProfileAdded(Profile* profile) {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    _UpdateRichPresence(static_cast<RBProfile*>(profile), mMode);
}

// Reconstructed from eboot.elf at 0xD5F780. Only the slots waiting for the
// profile let it go; the slots that hold it keep the pointer.
void RBProfileMgr::_OnProfileAboutToBeRemoved(Profile* profile) {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    if (IsCriticalProfile(static_cast<RBProfile*>(profile))) {
        mCriticalProfiles.clear();
        _BootToMainMenu(mShowCriticalDropOutDialog);
    }
    for (auto& entry : mProfileData) {
        if (entry.second.mPendingProfile == profile) {
            entry.second.mPendingProfile = nullptr;
        }
    }
    RBProfile** queued = mDestructionQueue.begin();
    while (queued != mDestructionQueue.end()) {
        if (*queued == profile) {
            mDestructionQueue.erase(queued);
        } else {
            ++queued;
        }
    }
}

// Reconstructed from eboot.elf at 0xD5F8B0.
void RBProfileMgr::_BootToMainMenu(bool& showDialog) {
    showDialog = true;
    static Symbol transitionDone;
    if (transitionDone == Symbol()) {
        transitionDone = Symbol("transition_done");
    }
    static Symbol onTransitionDone;
    if (onTransitionDone == Symbol()) {
        onTransitionDone = Symbol("on_transition_done");
    }
    theUI->AddSink(&mTransitionDoneSink, this, transitionDone, onTransitionDone);
    theRBMetaState.Goto(kLayoutMainMenu, false);
}

// Inlined into _OnProfileFailedToLoad and _OnProfileFailedToSave.
bool RBProfileMgr::_ShouldIgnoreSaveLoadFailures() const {
    const RBUILayout layout = theRBMetaState.mCurrentLayout;
    return layout == kLayoutInvalid || layout == kLayoutGameStartup;
}

// Inlined into _OnProfileFailedToLoad and _OnProfileFailedToSave.
bool RBProfileMgr::_CanShowSaveLoadFailMsgNow() const {
    return theRBMetaState.mCurrentLayout == kLayoutTitle;
}

// Reconstructed from eboot.elf at 0xD5F9E0.
void RBProfileMgr::_OnProfileFailedToLoad(int padNum) {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    if (!_ShouldIgnoreSaveLoadFailures()) {
        if (_CanShowSaveLoadFailMsgNow()) {
            _ShowDialog(kDialogLoadFailed);
        } else {
            _BootToMainMenu(mShowLoadFailedDialog);
        }
    }
    _AddProfileToDestructionQueue(FindProfile(*this, padNum));
}

// Reconstructed from eboot.elf at 0xD5FAC0.
void RBProfileMgr::_ShowDialog(ProfileMgrDialogType type) {
    Symbol body("");
    switch (type) {
    case kDialogLoadFailed: {
        static Symbol saveDataFailedLoad;
        if (saveDataFailedLoad == Symbol()) {
            saveDataFailedLoad = Symbol("save_data_failed_load");
        }
        body = saveDataFailedLoad;
        break;
    }
    case kDialogSaveFailed: {
        static Symbol saveDataFailedSave;
        if (saveDataFailedSave == Symbol()) {
            saveDataFailedSave = Symbol("save_data_failed_save");
        }
        body = saveDataFailedSave;
        break;
    }
    case kDialogCriticalUserDropOut: {
        static Symbol criticalUserDropOut;
        if (criticalUserDropOut == Symbol()) {
            criticalUserDropOut = Symbol("critical_user_drop_out");
        }
        body = criticalUserDropOut;
        break;
    }
    }
    ShowOkPopup(body, {});
}

// Reconstructed from eboot.elf at 0xD5FE00.
void RBProfileMgr::_AddProfileToDestructionQueue(RBProfile* profile) {
    if (profile == nullptr) {
        return;
    }
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    for (RBProfile* const queued : mDestructionQueue) {
        if (queued == profile) {
            return;
        }
    }
    mDestructionQueue.push_back(profile);
}

// Reconstructed from eboot.elf at 0xD5FF80.
void RBProfileMgr::_OnProfileFailedToSave(int padNum) {
    static_cast<void>(padNum);
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    if (!_ShouldIgnoreSaveLoadFailures()) {
        if (_CanShowSaveLoadFailMsgNow()) {
            _ShowDialog(kDialogSaveFailed);
        } else {
            _BootToMainMenu(mShowSaveFailedDialog);
        }
    }
}

// Reconstructed from eboot.elf at 0xD60010.
void RBProfileMgr::_OnAppResumed() {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    const eastl::vector<RBProfile*> profiles = GetProfiles();
    for (RBProfile* const profile : profiles) {
        profile->SetNeedsEntitlementRefresh(true);
    }
}

// Reconstructed from eboot.elf at 0xD605E0.
bool RBProfileMgr::_AnyParticipatingProfileSignedIn() const {
    eastl::vector<RBProfile*> profiles;
    profiles.reserve(4);
    _GetParticipatingProfiles(profiles);
    for (RBProfile* const profile : profiles) {
        if (profile->mSignInState == 0) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xD60710.
bool RBProfileMgr::_AnyParticipatingProfileOnline() const {
    eastl::vector<RBProfile*> profiles;
    profiles.reserve(4);
    _GetParticipatingProfiles(profiles);
    for (RBProfile* const profile : profiles) {
        if (profile->mSignInState == 0 && profile->mServerReachable) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xD60840. The pads are copied under the
// lock, asked about without it and written back under it.
void RBProfileMgr::_PollSignedInState() {
    static eastl::vector<SignedInState> sStates;
    sStates.reserve(64);
    if (theRBProfileMgr == nullptr) {
        return;
    }
    sStates.clear();
    gRBProfileMgrCritSec.Enter();
    const eastl::vector<RBProfile*> profiles = theRBProfileMgr->GetProfiles();
    for (RBProfile* const profile : profiles) {
        if (profile->mPadNum != -1) {
            SignedInState state;
            state.mPadNum = profile->mPadNum;
            state.mSignedIntoLive = false;
            sStates.push_back(state);
        }
    }
    gRBProfileMgrCritSec.Exit();
    for (SignedInState& state : sStates) {
        state.mSignedIntoLive = ThePlatformMgr->IsSignedIntoLive(state.mPadNum);
    }
    ScopedCritSec lock(gRBProfileMgrCritSec);
    for (const SignedInState& state : sStates) {
        RBProfile* const profile = theRBProfileMgr->GetSlotProfileFromPadNum(state.mPadNum);
        if (profile != nullptr) {
            profile->mSignedIntoLive = state.mSignedIntoLive;
        }
    }
}

// Reconstructed from eboot.elf at 0xD60C00.
void RBProfileMgr::_ResolvePendingProfiles() {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    bool resolved = false;
    for (auto& entry : mProfileData) {
        RBProfile* const pending = entry.second.mPendingProfile;
        if (pending != nullptr) {
            entry.second.mProfile = pending;
            entry.second.mPendingProfile = nullptr;
            mAddedProfilePads.push_back(pending->mPadNum);
            resolved = true;
        }
    }
    if (resolved) {
        static Symbol pendingProfilesResolved;
        if (pendingProfilesResolved == Symbol()) {
            pendingProfilesResolved = Symbol("pending_profiles_resolved");
        }
        Message<1> msg(pendingProfilesResolved);
        Export(msg.mData);
    }
}

// Reconstructed from eboot.elf at 0xD60EA0. The presence goes through the
// global manager's profiles.
void RBProfileMgr::_UpdateMode(ProfileMgrMode mode, bool force) {
    if ((mode == kModeNone || mMode == mode) && !force) {
        return;
    }
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    if (mode == kModeMenus && mMode == kModeGame) {
        _ResolvePendingProfiles();
    }
    mMode = mode;
    for (Profile* const profile : theRBProfileMgr->mProfiles) {
        _UpdateRichPresence(static_cast<RBProfile*>(profile), mMode);
    }
}

// Reconstructed from eboot.elf at 0xD60FA0. Only the menus and the game
// (or the title) react.
DataNode RBProfileMgr::OnMsg(const ConnectionStatusChangedMsg& msg) {
    if ((mMode != kModeMenus && mMode != kModeGame) && theRBMetaState.mCurrentLayout != kLayoutTitle) {
        return DataNode();
    }
    if (msg.IsConnected()) {
        const eastl::vector<RBProfile*> profiles = GetProfiles();
        for (RBProfile* const profile : profiles) {
            profile->SetNeedsEntitlementRefresh(true);
        }
        _ResetSongUploads();
        mServerConnected = true;
    }
    if (!_AnyParticipatingProfileSignedIn()) {
        return DataNode();
    }
    if (!msg.IsConnected() && mServerConnected) {
        mShowLostConnectionDialog = true;
        mServerConnected = false;
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xD61270.
bool RBProfileMgr::AnyParticipatingProfileSubmittingScores() const {
    eastl::vector<RBProfile*> profiles;
    profiles.reserve(4);
    _GetParticipatingProfiles(profiles);
    for (RBProfile* const profile : profiles) {
        if (profile->mMiniLeaderboardResults.mSubmissionsStarted) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xD61130.
void RBProfileMgr::_ResetSongUploads() {
    const eastl::vector<RBProfile*> profiles = GetProfiles();
    for (RBProfile* const profile : profiles) {
        profile->SetSongsUploaded(false);
    }
}

// Reconstructed from eboot.elf at 0xD613A0. A guest drops its
// entitlements. The name lookups are what the release build keeps of its
// logging.
void RBProfileMgr::_UpdateProfileServerJobs(RBProfile* profile) {
    if (!profile->mPlatformEntitlementsJob.mNeeded && !profile->mServerEntitlementsJob.mNeeded) {
        return;
    }
    if (profile->mPadNum == -1) {
        profile->ClearEntitlements();
        profile->SetNeedsEntitlementRefresh(false);
    } else if (profile->mReadyToSave && profile->mSignInState == 0 && !_IsProfileWaitingForServer(profile)
               && !profile->IsEntitlementRetryWaiting()) {
        _StartProfileServerJobs(profile);
    }
}

// Reconstructed from eboot.elf at 0xD61480.
bool RBProfileMgr::_IsProfileWaitingForServer(RBProfile* profile) const {
    return profile->mPlatformEntitlementsJob.mPending || profile->mServerEntitlementsJob.mPending;
}

// Reconstructed from eboot.elf at 0xD614A0. The platform's entitlements
// are no longer requested here; the flag is only cleared.
void RBProfileMgr::_StartProfileServerJobs(RBProfile* profile) {
    if (profile->mPlatformEntitlementsJob.mNeeded) {
        profile->SetNeedsPlatformEntitlements(false);
    }
    if (profile->mServerEntitlementsJob.mNeeded) {
        _StartEntitlementsJob(profile);
    }
}

// Reconstructed from eboot.elf at 0xD61700.
void RBProfileMgr::_StartEntitlementsJob(RBProfile* profile) {
    if (profile->mServerEntitlementsJob.mPending) {
        static_cast<void>(profile->GetPlatformName());
        return;
    }
    RBNetEntitlementsJob* const job = new RBNetEntitlementsJob(profile->mPadNum);
    theRBNetMgr.ManageJob(job->AsDingoJob());
    profile->SetServerEntitlementsPending(true);
}

// Reconstructed from eboot.elf at 0xD61760. A guest counts as uploaded;
// the song job's first flag (SetSongsUploaded) means the songs were sent.
void RBProfileMgr::_UpdateSongUpload(RBProfile* profile) {
    if (profile->mSongUploadJob.mNeeded) {
        return;
    }
    if (profile->mPadNum == -1) {
        profile->SetSongsUploaded(true);
    } else if (profile->mReadyToSave && profile->mSignInState == 0 && !profile->mSongUploadJob.mPending
               && !profile->IsSongUploadRetryWaiting()) {
        _StartSongUploadJob(profile);
    }
}

// Reconstructed from eboot.elf at 0xD617D0.
void RBProfileMgr::_StartSongUploadJob(RBProfile* profile) {
    if (profile->mSongUploadJob.mPending) {
        static_cast<void>(profile->GetPlatformName());
        return;
    }
    const eastl::set<int> songs = theSongMgr->GetPlaybleSongSet();
    RBNetUpdateSongsJob* const job = new RBNetUpdateSongsJob(profile->mPadNum, songs);
    theRBNetMgr.ManageJob(job->AsDingoJob());
    profile->SetSongUploadPending(true);
}

// Reconstructed from eboot.elf at 0xD61C40.
void RBProfileMgr::OnEntitlementsJobDone(int padNum, bool success, const eastl::set<int>& entitlements) {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    static_cast<void>(scePthreadSelf());
    RBProfile* const profile = FindProfile(*this, padNum);
    if (profile == nullptr) {
        return;
    }
    if (!profile->mServerEntitlementsJob.mPending) {
        static_cast<void>(profile->GetPlatformName());
    }
    profile->SetServerEntitlementsPending(false);
    if (success) {
        for (const eastl::rbtree_node_base* node = entitlements.mAnchor.mpNodeLeft; node != entitlements.end();
             node = eastl::RBTreeIncrement(node)) {
            profile->AddEntitlement(static_cast<const eastl::set<int>::node_type*>(node)->mValue);
        }
        profile->SetNeedsServerEntitlements(false);
    } else {
        static_cast<void>(profile->GetPlatformName());
    }
}

// Reconstructed from eboot.elf at 0xD61D50.
void RBProfileMgr::OnSongUploadJobDone(int padNum, bool success) {
    static const long sHeap = MemFindHeap("profile");
    ProfileHeapScope scope(sHeap);
    static_cast<void>(scePthreadSelf());
    RBProfile* const profile = FindProfile(*this, padNum);
    if (profile == nullptr) {
        return;
    }
    if (!profile->mSongUploadJob.mPending) {
        static_cast<void>(profile->GetPlatformName());
    }
    profile->SetSongUploadPending(false);
    if (success) {
        profile->SetSongsUploaded(true);
    } else {
        static_cast<void>(profile->GetPlatformName());
    }
}

// Reconstructed from eboot.elf at 0xD611C0.
DataNode RBProfileMgr::OnMsg(const GameCreatedMsg& msg) {
    static_cast<void>(msg);
    _UpdateMode(kModeGame, true);
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xD61200.
DataNode RBProfileMgr::OnMsg(const GameStartedMsg& msg) {
    static_cast<void>(msg);
    _ResolvePendingProfiles();
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xD61230.
DataNode RBProfileMgr::OnMsg(const GamePreEndedMsg& msg) {
    _UpdateProfileDataFromGameState(msg.GetValue());
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xD61E30.
void RBProfileMgr::_OnSongCacheDirty() {
    for (Profile* const profile : mProfiles) {
        profile->MakeDirty();
    }
    mSystemOptions.MakeDirty();
}

// Reconstructed from eboot.elf at 0xD61E80.
void RBProfileMgr::_OnSystemOptionsDirty() {
    for (Profile* const profile : mProfiles) {
        profile->MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD61EC0. During the game the boot waits
// for _CheckForMainMenuBootFromGame.
void RBProfileMgr::_OnTransitionDone(const Symbol& from, const Symbol& to) {
    static_cast<void>(from);
    static_cast<void>(to);
    const RBUILayout layout = theRBMetaState.mCurrentLayout;
    const bool booting = mShowCriticalDropOutDialog || mShowSaveFailedDialog || mShowLoadFailedDialog;
    if (layout == kLayoutMainMenu || !booting) {
        return;
    }
    if (layout == kLayoutGame) {
        mBootToMainMenuFromGame = true;
    } else {
        theRBMetaState.Goto(kLayoutMainMenu, false);
    }
}

// Reconstructed from eboot.elf at 0xD61F20.
DataNode RBProfileMgr::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    if (type == GameCreatedMsg::Event()) {
        DataNode result = OnMsg(GameCreatedMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    if (type == GameStartedMsg::Event()) {
        DataNode result = OnMsg(GameStartedMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    if (type == GamePreEndedMsg::Event()) {
        DataNode result = OnMsg(GamePreEndedMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    if (type == ConnectionStatusChangedMsg::Event()) {
        DataNode result = OnMsg(ConnectionStatusChangedMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    if (type == PlatformEntitlementsRetrievedMsg::Event()) {
        DataNode result = OnMsg(PlatformEntitlementsRetrievedMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    {
        DataNode result = ProfileMgr::Handle(msg, false);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol onSongCacheDirty;
    if (onSongCacheDirty == Symbol()) {
        onSongCacheDirty = Symbol("on_song_cache_dirty");
    }
    if (type == onSongCacheDirty) {
        _OnSongCacheDirty();
        return DataNode(0);
    }
    static Symbol onSystemOptionsDirty;
    if (onSystemOptionsDirty == Symbol()) {
        onSystemOptionsDirty = Symbol("on_system_options_dirty");
    }
    if (type == onSystemOptionsDirty) {
        _OnSystemOptionsDirty();
        return DataNode(0);
    }
    static Symbol onTransitionDone;
    if (onTransitionDone == Symbol()) {
        onTransitionDone = Symbol("on_transition_done");
    }
    if (type == onTransitionDone) {
        _OnTransitionDone(msg->Sym(2), msg->Sym(3));
        return DataNode(0);
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0xD62560.
SystemOptions* RBProfileMgr::GetSystemOptions() {
    return &mSystemOptions;
}

// Reconstructed from eboot.elf at 0xD62570.
const char* RBProfileMgr::GetSystemSaveFileName() const {
    return "rb_system_settings.bin";
}
