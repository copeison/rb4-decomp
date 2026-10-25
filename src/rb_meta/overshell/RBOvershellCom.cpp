// rb_meta/RBOvershellCom.o (0xBE6F60 to 0xBF727F). The properties'
// accessors are the lambdas after the identity slots (0xBF65B0-0xBF65F0).
// The compiler's code: the EASTL helpers (0xBF6BA0 for the slot vector's
// resize, 0xBF6EB0 for the online set's copy), the std::functions of the
// dialogs' callbacks (vtables 0x1971B60, 0x1971BB0 and 0x1971C00,
// 0xBF6CA0-0xBF7170; the third is _ShowErrorZeroPlayersDialog's condition,
// 0xBF7020) and the messages' constructors inlined from their headers
// (0xBF6860 GameModeChangedMsg, 0xBF6970 GameMicsChangedMsg, 0xBF6A80
// SystemOverlayStatusChangedMsg, for Handle). The static initializer is at
// 0xBF7180; the shared header statics it sets first (0x1B02988-0x1B029D8)
// belong to headers that are not modelled, apart from the null object id.
// The class's registration and factory are emitted elsewhere (0xB87E00,
// 0xBAB940). The functions that call into classes not modelled yet stay
// declared in the header with their addresses.
#include "rb_meta/overshell/RBOvershellCom.h"

#include <cfloat>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/InstanceCom.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "math/color/Color.h"
#include "math/random/Rand.h"
#include "os/platform/PlatformMgr.h"
#include "persistence/profiles/Profile.h"
#include "rb_game/users/BandUser.h"
#include "rb_game/users/BandUserMgr.h"
#include "rb_game/users/RBBandUser.h"
#include "rb_meta/dialog/DialogMgrCom.h"
#include "rb_meta/input/RBUIInputCom.h"
#include "rb_meta/overshell/RBOvershellSlotCom.h"
#include "rb_meta/overshell/RBOvershellSlotState.h"
#include "rb_meta/profiles/RBProfile.h"
#include "rb_meta/profiles/RBProfileMgr.h"
#include "rb_meta/somp/RBSompSessionMgr.h"
#include "rb_meta/state/RBMetaStateCom.h"
#include "render/scene/RndSceneCom.h"
#include "render/scene/RndSceneResource.h"
#include "ui/manager/UIMgr.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataFile.h"
#include "utl/messages/Message.h"
#include "utl/text/Str.h"
#include "utl/time/TimeMgr.h"

// ---------------------------------------------------------------------------
// Declared for the overshell: classes of other modules that are not
// modelled. Only the members the overshell calls are declared, as
// RBOvershellSlotCom.cpp does.

// The game's data (rb_game/RBGameData.o); the game mode is an RBGameMode.
class RBGameData : public MsgSource {
public:
    int GetGameMode() const;  // 0x965E50
};

extern RBGameData theGameData;  // 0x1AE1818

// The game while a song plays (rb_game/Game.o, the map's theGame). The
// overshell pauses the song through the mutable copy (0x1AEF778) and
// subscribes to its "error_zero_players".
class Game : public MsgSource {
public:
    // The map's Game::OvershellSetPaused(bool); the result is not read.
    bool OvershellSetPaused(bool paused);  // 0xA48300
};

extern Game* theGame;         // 0x1AEF770
extern Game* theMutableGame;  // 0x1AEF778

// The game mode's configuration (rb_game/GameMode.o), the inline
// GameMode::Instance()'s static (0x1AE4538, guard 0x1AE45D0), which each
// user constructs on first use. Only "max_players" is declared.
class GameMode {
public:
    GameMode();   // 0xA4ABE0
    ~GameMode();  // 0x970430

    static GameMode& Instance() {
        static GameMode sInstance;
        return sInstance;
    }

    unsigned char mOpaque[0x90];  // Not modelled.
    // "max_players". Field name not in the reference map.
    int mMaxPlayers;
    unsigned char mOpaque94[4];  // Not modelled.
};

static_assert(sizeof(GameMode) == 0x98);

// The data the game's track and overshell layouts are built from
// (rb_game/RBTrackLayoutData.o), not modelled: just what the overshell
// calls.
class RBTrackLayoutData {
public:
    // Marks the data for a refresh, under the object's lock.
    void SetNeedsRefresh();  // 0x99E690
};

extern RBTrackLayoutData theRBTrackLayoutData;  // 0x1AE5AD8

// The story mode's ("BTM", game mode 8) manager of rb_game (0x92D000 and
// on), not modelled. The overshell stays disabled while one of its events
// is pending (the flag at +0xA48 with an int and a symbol) or runs
// (+0xA58). The class, global and member names are not in the reference
// map; the evidence for them is weak.
class RBBTMMgr {
public:
    bool HasPendingEvent() const;  // 0x932D40

    unsigned char mOpaque[0xA58];  // Not modelled.
    bool mEventActive;
};

extern RBBTMMgr* TheRBBTMMgr;  // 0x1ADFE20

// The character preview scene's manager (rb_meta/RBCharacterPreviewMgrCom.o),
// not modelled: the map's static ShowPreviewScene and HidePreviewScene.
class RBCharacterPreviewMgrCom {
public:
    static void ShowPreviewScene();  // 0xB09F30
    static void HidePreviewScene();  // 0xB09FD0
};

// The pads' light colors by pad (a table at 0x1B4C120). The map has
// RBLightingStateCom::GetJoyPadColor(int); the match rests on the
// signature, so it is weak.
class RBLightingStateCom {
public:
    static const Hmx::Color& GetJoyPadColor(int pad);  // 0xFE2020
};

// The pads' light bars. The map's os/Joypad.o functions; in this build
// they are emitted at 0x8CF700-0x8CF7C0 and the match rests on the
// signatures, so it is weak.
void JoypadSetColorInactive(int pad, bool inactive);  // 0x8CF700
void JoypadSetColor(int pad, const Hmx::Color& color);  // 0x8CF760
void JoypadResetColor(int pad);                         // 0x8CF7C0

namespace {

// The id of no object, at 0x1B02988, which the static initializer sets
// with the shared header's statics. Each object that uses it has its own
// copy. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// RBGameData's story mode ("BTM"). Name not in the reference map.
constexpr int kGameModeBTM = 8;

// The pads whose lights the overshell sets. Name not in the reference map.
constexpr int kNumPads = 8;

// The pending online states end after this long, and not before the
// shorter time unless the slots are ready, in milliseconds. Names not in
// the reference map.
constexpr float kOnlineTransitionTimeoutMs = 11000.0F;
constexpr float kOnlineTransitionMinMs = 1500.0F;

// How long the SOMP layout transition lasts, in milliseconds. Name not in
// the reference map.
constexpr float kSompLayoutTransitionMs = 2500.0F;

// GetSompMinWaitTime's starting minimum. Name not in the reference map.
constexpr float kNoWaitTime = FLT_MAX;

// The milliseconds of the real time, the online transitions' clock.
// Inlined from TimeMgr's timer. Name not in the reference map.
float RealTimeMs() {
    return TheTimeMgr->mRealTime.SplitMs();
}

// Whether the set holds the slot. Name not in the reference map.
bool HasSlot(const eastl::set<unsigned long>& slots, unsigned long slot) {
    return slots.find(slot) != slots.end();
}

// The run-time state's subscriptions, in declaration order. Name not in
// the reference map.
constexpr int kNumSinks = 17;
void GetSinks(RBOvershellCom::RuntimeData& data, MsgSource::EventSinkElem* (&sinks)[kNumSinks]) {
    MsgSource::EventSinkElem* const all[kNumSinks] = {
        &data.mTransitionDoneSink,
        &data.mGameMicsChangedSink,
        &data.mDebugControllerTypesChangedSink,
        &data.mProfileAboutToBeRemovedSink,
        &data.mProfileFailedToSaveLoadSink,
        &data.mPendingProfilesResolvedSink,
        &data.mGameCreatedSink,
        &data.mGameStartedSink,
        &data.mGameEndedSink,
        &data.mGameAboutToBeDestroyedSink,
        &data.mFailScreenPresentedSink,
        &data.mSetSongOptionsRequiredSink,
        &data.mErrorZeroPlayersSink,
        &data.mSystemOverlayStatusChangedSink,
        &data.mGameModeChangedSink,
        &data.mJoypadSink,
        &data.mLostConnectionToServerSink,
    };
    for (int index = 0; index < kNumSinks; ++index) {
        sinks[index] = all[index];
    }
}

}  // namespace

// The object's statics, in the order of its static initializer (0xBF7180).
Symbol RBOvershellCom::sId("RBOvershellCom");
Symbol RBOvershellCom::sClassName("RBOvershellCom");
PropRegistry RBOvershellCom::sPropRegistry;
ComMetaData RBOvershellCom::sMetaData;

RBOvershellCom* theOvershell;

// Reconstructed from eboot.elf at 0xBE6F60.
const char* OvershellOnlineStateToString(OvershellOnlineState state) {
    static const char* const kNames[] = {"Invalid", "Offline", "Pending Offline", "Online", "Pending Online"};
    const unsigned int index = static_cast<unsigned int>(state + 1);
    return index <= 4 ? kNames[index] : "UNKNOWN_STATE";
}

// Reconstructed from eboot.elf at 0xBE6F80. Each label is also the
// description.
eastl::vector<AllowedValue<int>> GetOvershellOnlineStateAllowedValues() {
    eastl::vector<AllowedValue<int>> values;
    values.reserve(5);
    values.push_back(AllowedValue<int>{kOvershellOnlineStateInvalid, String("Invalid"), String("Invalid")});
    values.push_back(AllowedValue<int>{kOvershellOnlineStateOffline, String("Offline"), String("Offline")});
    values.push_back(AllowedValue<int>{
        kOvershellOnlineStatePendingOffline, String("Pending Offline"), String("Pending Offline")});
    values.push_back(AllowedValue<int>{kOvershellOnlineStateOnline, String("Online"), String("Online")});
    values.push_back(AllowedValue<int>{
        kOvershellOnlineStatePendingOnline, String("Pending Online"), String("Pending Online")});
    return values;
}

// Reconstructed from eboot.elf at 0xBE72D0. The slot vector starts with
// four empty slots; the requests and the queued users reserve room for
// ten.
RBOvershellCom::RuntimeData::RuntimeData()
    : mNeedsFindSlots(true),
      mSystemOverlayOpen(false),
      mSystemOverlayClosed(false),
      mSlots(),
      mJoinOrder(),
      mInputCom(nullptr),
      mLostConnectionToServer(false),
      mOverrideFlow(kOverrideFlowNone),
      mOverrideFlowRequests(),
      mActiveStatus(kOvershellInMeta),
      mInVenueMetaMode(false),
      mSongOptionsRequired(false),
      mUseExtendedMicArrows(false),
      mAllowsButtonPulse(true),
      mQueuedJoinUsers(),
      mPartRand(),
      mRandomSeed(0),
      mIdleMasterGuid(),
      mIdleMasterPadNum(-1),
      mIdleMasterProfile(nullptr),
      mIdleMasterSlot(static_cast<unsigned long>(-1)),
      mGameplayBlocking(false),
      mSompLayoutTransitionBlocking(false),
      mOnlineState(kOvershellOnlineStateOffline),
      mOnlineSlots(),
      mOnlineStateChanged(false),
      mReconnectDialogShowing(false),
      mJoinBlocked(false),
      mOnlineTransitionStartMs(-1.0F),
      mSompDropInStartMs(-1.0F),
      mMovingUser(false),
      mSompLayoutTransition(false),
      mSompLayoutTransitionTime(-1.0F),
      mNeedsUpdateAll(false),
      mNeedsDrawReset(false),
      mLightLayout(kLayoutInvalid) {
    MsgSource::EventSinkElem* sinks[kNumSinks];
    GetSinks(*this, sinks);
    for (MsgSource::EventSinkElem* const sink : sinks) {
        sink->mSink = nullptr;
        sink->mSource = nullptr;
    }
    mSlots.resize(4);
    mOverrideFlowRequests.reserve(10);
    mQueuedJoinUsers.reserve(10);
}

// Reconstructed from eboot.elf at 0xBE78E0. Each linked subscription leaves
// its source, the last one first, before the links unlink themselves.
RBOvershellCom::RuntimeData::~RuntimeData() {
    MsgSource::EventSinkElem* sinks[kNumSinks];
    GetSinks(*this, sinks);
    for (int index = kNumSinks - 1; index >= 0; --index) {
        MsgSource::EventSinkElem* const sink = sinks[index];
        LinkedList::Node& link = sink->mLink;
        if (link.mNext != &link && link.mPrev != &link) {
            sink->mSource->RemoveSink(sink);
        }
    }
}

// Reconstructed from eboot.elf at 0xBE7EE0. The prototype publishes itself
// as theOvershell while the registry is built.
void RBOvershellCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RBOvershellCom prototype;
    PropRegistry& ownRegistry = prototype._GetPropRegistry();
    if (&ownRegistry != &registry) {
        registry.AddInheritedRegistry(sClassName, ownRegistry);
    }
    metadata.mDescription = "Total control of the overshell scene.";
    metadata.mAuthor = "Sheila Porter";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(RndSceneResource::Id());

    PropInfo& slots = registry.RegisterPropArray(
        "overshell_slots", offsetof(RBOvershellCom, mOvershellSlots), sizeof(PropArray<GameObjectId>));
    ArrayMetadata& slotsMetadata = static_cast<ArrayMetadata&>(*slots.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(slots, kPropertyGameObjectId, sizeof(GameObjectId));
    const GameObjectId nullId = {0xFFFFFFFFU};
    GameObjectIdMetadata& slot = TypeSpecificMetadata(slotsMetadata.mItemInfo, nullId);
    static_cast<void>(Symbol("item"));
    slot.mRequiredComs = {RBOvershellSlotCom::sId};
    slotsMetadata.mFixedSize = true;

    SymbolMetadata& status =
        TypeSpecificMetadata(registry.RegisterProp("active_status", -1, kPropertySymbol, 0), Symbol());
    static_cast<void>(Symbol("prop"));
    {
        static Symbol inactive;
        if (inactive == Symbol()) {
            inactive = Symbol("kOvershellInactive");
        }
        static Symbol inMeta;
        if (inMeta == Symbol()) {
            inMeta = Symbol("kOvershellInMeta");
        }
        static Symbol inSong;
        if (inSong == Symbol()) {
            inSong = Symbol("kOvershellInSong");
        }
        eastl::vector<AllowedValue<Symbol>> values;
        values.push_back(AllowedValue<Symbol>{
            inactive, String(inactive.Str()), String("Overshell is inactive (loading screens, etc).")});
        values.push_back(AllowedValue<Symbol>{inMeta, String(inMeta.Str()), String("Game is in a meta screen.")});
        values.push_back(AllowedValue<Symbol>{inSong, String(inSong.Str()), String("In gameplay.")});
        status.mAllowedValues = values;
        // At 0xBF65B0 and 0xBF65C0. The setter writes the status without
        // SetActiveStatus.
        status.mGetter = [](const PropAccessorArgs& args) {
            return OvershellActiveStatusToSymbol(static_cast<RBOvershellCom*>(args.mComponent)->mRuntime.mActiveStatus);
        };
        status.mSetter = [](const PropAccessorArgs& args) {
            static_cast<RBOvershellCom*>(args.mComponent)->mRuntime.mActiveStatus =
                SymbolToOvershellActiveStatus(*static_cast<const Symbol*>(args.mValue));
        };
        status.mSaves = false;
        status.mHistory = false;
    }

    BoolMetadata& showing =
        TypeSpecificMetadata(registry.RegisterProp("should_be_showing", -1, kPropertyBool, 0), false);
    static_cast<void>(Symbol("prop"));
    // At 0xBF65F0.
    showing.mGetter = [](const PropAccessorArgs& args) {
        return static_cast<RBOvershellCom*>(args.mComponent)->ShouldBeShowing();
    };
    showing.mReadOnly = true;
    showing.mSaves = false;
}

// Reconstructed from eboot.elf at 0xBE8810. The four slot objects start
// null.
RBOvershellCom::RBOvershellCom() : mRuntime(), mOvershellSlots() {
    theOvershell = this;
    mOvershellSlots.Resize(4);
    for (unsigned int index = 0; index < 4; ++index) {
        mOvershellSlots[index] = gNullObjectId;
    }
}

// Reconstructed from eboot.elf at 0xBE88D0. The deleting destructor is at
// 0xBE8970.
RBOvershellCom::~RBOvershellCom() {
    theOvershell = nullptr;
}

// Reconstructed from eboot.elf at 0xBE8A10.
RBOvershellSlotCom* RBOvershellCom::GetSlotComFromGameSlot(GameSlot slot) const {
    for (RBOvershellSlotCom* const slotCom : mRuntime.mSlots) {
        if (slotCom != nullptr && slotCom->mRuntime.mGameSlot == slot) {
            return slotCom;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0xBE8A80.
RBOvershellSlotCom* RBOvershellCom::GetSlotFromUser(BandUser* user) const {
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (slot != nullptr && slot->GetUser() == user) {
            return slot;
        }
    }
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (slot != nullptr && slot->IsValidUser(user)) {
            return slot;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0xBE8B10.
RBOvershellSlotCom* RBOvershellCom::GetSlot(unsigned long index) const {
    if (index >= mRuntime.mSlots.size()) {
        return nullptr;
    }
    return mRuntime.mSlots[index];
}

// Reconstructed from eboot.elf at 0xBE8B40. A changed status takes every
// slot out of its options, except the users choosing their part in the
// song settings flow; the release build keeps the call of a stripped
// check on the override flow.
void RBOvershellCom::SetActiveStatus(OvershellActiveStatus status, bool venueMetaMode) {
    if (mRuntime.mActiveStatus != status || mRuntime.mInVenueMetaMode != venueMetaMode) {
        mRuntime.mActiveStatus = status;
        mRuntime.mInVenueMetaMode = venueMetaMode;
        static_cast<void>(GetOverrideFlow());
        if (status == kOvershellInMeta) {
            SetSongOptionsRequired(false);
        }
        for (unsigned long index = 0; index < mRuntime.mSlots.size(); ++index) {
            if (mRuntime.mSlots[index]->GetUser() == nullptr) {
                continue;
            }
            if (mRuntime.mSlots[index]->InOverrideFlow(kOverrideFlowSongSettings)) {
                if (!mRuntime.mSlots[index]->mRuntime.mPartChosen) {
                    mRuntime.mSlots[index]->EndOverrideFlow(kOverrideFlowSongSettings, true);
                }
            } else {
                mRuntime.mSlots[index]->LeaveOptions(true);
            }
        }
    }
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        slot->SetActiveStatus(status);
    }
    if (status == kOvershellInSong) {
        _SetInGame(true);
    }
    if (_CanJoinUsers()) {
        _AddQueuedJoinUsers();
    }
    UpdateAll();
}

// Reconstructed from eboot.elf at 0xBE8E20.
void RBOvershellCom::SetSongOptionsRequired(bool required) {
    mRuntime.mSongOptionsRequired = required;
    UpdateAll();
}

// Reconstructed from eboot.elf at 0xBE8E30.
void RBOvershellCom::_SetInGame(bool inGame) {
    if (inGame) {
        SetSongOptionsRequired(true);
        if (_CanJoinUsers()) {
            _StartDropInFlows();
        }
    }
}

// Reconstructed from eboot.elf at 0xBE8ED0.
bool RBOvershellCom::_CanJoinUsers() const {
    return ShouldBeShowing() && _CanUpdateSlots();
}

// Reconstructed from eboot.elf at 0xBE8F40. Users who take part already
// are left out. The release build keeps the calls of a stripped check.
void RBOvershellCom::_AddQueuedJoinUsers() {
    static_cast<void>(_CanJoinUsers());
    for (unsigned long index = 0; index < mRuntime.mQueuedJoinUsers.size(); ++index) {
        BandUser* const user = mRuntime.mQueuedJoinUsers[index];
        if (!user->mParticipating) {
            AttemptToAddUser(user);
        }
    }
    mRuntime.mQueuedJoinUsers.clear();
}

// Reconstructed from eboot.elf at 0xBE8FE0.
void RBOvershellCom::UpdateAll() {
    _RefreshJoinableUsers();
    for (unsigned long index = 0; index < mRuntime.mSlots.size(); ++index) {
        if (mRuntime.mSlots[index] != nullptr) {
            mRuntime.mSlots[index]->UpdateState();
        }
    }
    _ResolveSlotStates();
    for (unsigned long index = 0; index < mRuntime.mSlots.size(); ++index) {
        mRuntime.mSlots[index]->UpdateView();
    }
    _CheckForControllerDisconnects();
}

// Reconstructed from eboot.elf at 0xBE9110.
OvershellActiveStatus RBOvershellCom::GetActiveStatus(bool* inVenueMetaMode) const {
    if (inVenueMetaMode != nullptr) {
        *inVenueMetaMode = mRuntime.mActiveStatus == kOvershellInSong ? mRuntime.mInVenueMetaMode : false;
    }
    return mRuntime.mActiveStatus;
}

// Reconstructed from eboot.elf at 0xBE9130.
void RBOvershellCom::EnterVenueMetaMode() {
    if (!mRuntime.mInVenueMetaMode) {
        SetActiveStatus(kOvershellInMeta, true);
    }
}

// Reconstructed from eboot.elf at 0xBE9150.
void RBOvershellCom::ExitVenueMetaMode() {
    if (mRuntime.mInVenueMetaMode) {
        LeaveOptions();
        _ChooseActiveStatusFromMetaState();
    }
}

// Reconstructed from eboot.elf at 0xBE91F0.
void RBOvershellCom::LeaveOptions() {
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (slot != nullptr && slot->GetUser() != nullptr) {
            slot->LeaveOptions(true);
        }
    }
}

// Reconstructed from eboot.elf at 0xBE9250.
void RBOvershellCom::_ChooseActiveStatusFromMetaState() {
    const RBUILayout layout = theRBMetaState.mCurrentLayout;
    OvershellActiveStatus status = kOvershellInSong;
    if (layout != kLayoutGame) {
        status = layout != kLayoutLoading ? kOvershellInMeta : kOvershellInactive;
    }
    SetActiveStatus(status, false);
}

// Reconstructed from eboot.elf at 0xBE9280.
void RBOvershellCom::SetUseExtendedMicArrows(bool use) {
    mRuntime.mUseExtendedMicArrows = use;
    UpdateAll();
}

// Reconstructed from eboot.elf at 0xBE9290.
void RBOvershellCom::SetAllowsButtonPulse(bool allows) {
    mRuntime.mAllowsButtonPulse = allows;
}

// Reconstructed from eboot.elf at 0xBE92A0.
void RBOvershellCom::SetIdleMasterState(
    const UserGuid& guid,
    int padNum,
    const RBProfile* profile,
    const unsigned long& slot) {
    mRuntime.mIdleMasterGuid = guid;
    mRuntime.mIdleMasterPadNum = padNum;
    mRuntime.mIdleMasterProfile = profile;
    mRuntime.mIdleMasterSlot = slot;
    theRBMetaState.mFocusSlot = slot != static_cast<unsigned long>(-1) ? static_cast<GameSlot>(slot) : kNumGameSlots;
}

// Reconstructed from eboot.elf at 0xBE92E0.
void RBOvershellCom::ClearIdleMaster() {
    mRuntime.mIdleMasterGuid = UserGuid();
    mRuntime.mIdleMasterPadNum = -1;
    mRuntime.mIdleMasterProfile = nullptr;
    mRuntime.mIdleMasterSlot = static_cast<unsigned long>(-1);
    theRBMetaState.mFocusSlot = kNumGameSlots;
}

// Reconstructed from eboot.elf at 0xBE9370.
bool RBOvershellCom::IsAnySlotOriginalIdleMaster() const {
    if (GetOverrideFlow() == kOverrideFlowIdle) {
        for (unsigned long slot = 0; slot < mRuntime.mSlots.size(); ++slot) {
            if (SlotIsOriginalIdleMaster(slot)) {
                return true;
            }
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xBE9430.
bool RBOvershellCom::SlotIsOriginalIdleMaster(const unsigned long& slot) const {
    return GetOverrideFlow() == kOverrideFlowIdle && slot == mRuntime.mIdleMasterSlot && SlotIsPossibleIdleMaster(slot);
}

// Reconstructed from eboot.elf at 0xBE9470.
bool RBOvershellCom::IsAnySlotPossibleIdleMaster() const {
    if (GetOverrideFlow() == kOverrideFlowIdle) {
        for (unsigned long slot = 0; slot < mRuntime.mSlots.size(); ++slot) {
            if (SlotIsPossibleIdleMaster(slot)) {
                return true;
            }
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xBE9510. A user playing another
// controller never is.
bool RBOvershellCom::SlotIsPossibleIdleMaster(const unsigned long& slot) const {
    if (GetOverrideFlow() != kOverrideFlowIdle) {
        return false;
    }
    RBOvershellSlotCom* const slotCom = GetSlot(slot);
    if (slotCom == nullptr) {
        return false;
    }
    BandUser* const user = slotCom->GetUser();
    if (user == nullptr || slotCom->IsUsingOtherController()) {
        return false;
    }
    if (user->GetUserGuid() == mRuntime.mIdleMasterGuid && user->GetPlatformUserId() == mRuntime.mIdleMasterPadNum) {
        return true;
    }
    const RBProfile* const profile = user->GetProfile(false);
    if (profile == nullptr) {
        return false;
    }
    return mRuntime.mIdleMasterProfile != nullptr && profile == mRuntime.mIdleMasterProfile;
}

// Reconstructed from eboot.elf at 0xBE95F0.
void RBOvershellCom::RedetectSlotIdleMaster() {
    if (GetOverrideFlow() == kOverrideFlowIdle) {
        for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
            if (slot != nullptr) {
                slot->ShowState(kState_Idle);
            }
        }
    }
}

// Reconstructed from eboot.elf at 0xBE9640. An online SOMP overshell takes
// no local user (unless a user is being moved), leaving only the release
// build's stripped log; in the SOMP mode the user may have to take the
// session's local profile first. The slot the user joins gets the profile
// before the user.
void RBOvershellCom::AttemptToAddUser(BandUser* user) {
    if (IsFull()) {
        return;
    }
    if (theRBSompSessionMgr.IsSompSession() && !mRuntime.mMovingUser &&
        mRuntime.mOnlineState != kOvershellOnlineStateOffline) {
        static_cast<void>(user->GetPlatformUserId());
        static_cast<void>(user->GetPlatformName());
        return;
    }
    bool switchProfile = false;
    RBProfile* profile = nullptr;
    if (theRBSompSessionMgr.IsInSompMode() && !_CheckSompJoin(user, &switchProfile, &profile)) {
        return;
    }
    if (!_CanJoinUsers()) {
        _QueueUserToJoin(user);
        return;
    }
    RBOvershellSlotCom* const slot = GetSlotFromUser(user);
    if (slot == nullptr) {
        return;
    }
    if (profile != nullptr) {
        theRBProfileMgr->AssignProfileToSlot(profile, slot->mRuntime.mBandSlot);
    }
    if (slot->LookupUserInJoinList(user)) {
        slot->AddUser(user, false);
        if (switchProfile) {
            theRBSompSessionMgr.SetLocalUser(user);
        }
        theRBTrackLayoutData.SetNeedsRefresh();
        UpdateAll();
    }
}

// Reconstructed from eboot.elf at 0xBE9860. A user is queued once. The
// release build keeps the calls of a stripped check.
void RBOvershellCom::_QueueUserToJoin(BandUser* user) {
    static_cast<void>(_CanJoinUsers());
    for (unsigned long index = 0; index < mRuntime.mQueuedJoinUsers.size(); ++index) {
        if (mRuntime.mQueuedJoinUsers[index] == user) {
            return;
        }
    }
    mRuntime.mQueuedJoinUsers.push_back(user);
}

// Reconstructed from eboot.elf at 0xBE99B0. The game mode's
// "max_players" bounds the users; a SOMP session is full while its local
// user must reconnect the controller.
bool RBOvershellCom::IsFull() const {
    if (mRuntime.mJoinBlocked) {
        return true;
    }
    if (GetNumUsers() >= GameMode::Instance().mMaxPlayers) {
        return true;
    }
    if (!theRBSompSessionMgr.IsSompSession()) {
        return false;
    }
    return _NeedsSompControllerReconnect();
}

// Reconstructed from eboot.elf at 0xBE9EE0. The flag is the slot's
// mPartChosen.
void RBOvershellCom::RemoveUsersRequiringSongOptions() {
    int removed = 0;
    for (unsigned long index = 0; index < mRuntime.mSlots.size(); ++index) {
        if (mRuntime.mSlots[index]->GetUser() != nullptr && mRuntime.mSlots[index]->mRuntime.mPartChosen) {
            mRuntime.mSlots[index]->RemoveUser(true);
            ++removed;
        }
    }
    if (removed != 0) {
        UpdateAll();
    }
}

// Reconstructed from eboot.elf at 0xBE9F70. The status and the override
// flow go through theOvershell; the release build keeps the status name
// of a stripped log.
void RBOvershellCom::Reset() {
    if (mRuntime.mActiveStatus != kOvershellInMeta) {
        static_cast<void>(OvershellActiveStatusToSymbol(mRuntime.mActiveStatus));
        theOvershell->SetActiveStatus(kOvershellInMeta, false);
    }
    mRuntime.mOverrideFlowRequests.clear();
    const OvershellOverrideFlow flow = mRuntime.mOverrideFlow;
    if (flow != kOverrideFlowNone) {
        theOvershell->EndOverrideFlow(flow, true);
    }
    LeaveOptions();
    _GoOffline();
    mRuntime.mJoinBlocked = false;
}

// Reconstructed from eboot.elf at 0xBEA150. Another flow's end is queued
// only behind a queued begin; otherwise only the release build's stripped
// log is left.
void RBOvershellCom::EndOverrideFlow(OvershellOverrideFlow flow, bool cancelled) {
    if (mRuntime.mOverrideFlow == flow) {
        if (_CanChangeOverrideFlow(false)) {
            _EndOverrideFlow(flow, cancelled);
            return;
        }
    } else if (!_HasQueuedBeginOverrideFlow()) {
        static_cast<void>(GetOverrideFlow());
        return;
    }
    _QueueEndOverrideFlow(flow, cancelled);
}

// Reconstructed from eboot.elf at 0xBEA1E0.
void RBOvershellCom::GoOffline() {
    _GoOffline();
}

// Reconstructed from eboot.elf at 0xBEA1F0.
void RBOvershellCom::_ProcessOverrideFlowRequests() {
    while (!mRuntime.mOverrideFlowRequests.empty()) {
        const OverrideFlowRequest request = mRuntime.mOverrideFlowRequests.front();
        if (!_CanProcessOverrideFlowRequest(request)) {
            break;
        }
        _ProcessOverrideFlowRequest(request);
        mRuntime.mOverrideFlowRequests.erase(mRuntime.mOverrideFlowRequests.begin());
    }
}

// Reconstructed from eboot.elf at 0xBEA330.
bool RBOvershellCom::_CanProcessOverrideFlowRequest(const OverrideFlowRequest& request) const {
    return _CanChangeOverrideFlow(request.mBegin);
}

// Reconstructed from eboot.elf at 0xBEA340.
void RBOvershellCom::_ProcessOverrideFlowRequest(const OverrideFlowRequest& request) {
    if (request.mBegin) {
        _BeginOverrideFlow(request.mFlow);
    } else {
        _EndOverrideFlow(request.mFlow, request.mCancelled);
    }
}

// Reconstructed from eboot.elf at 0xBEA3D0.
bool RBOvershellCom::_HasQueuedBeginOverrideFlow() const {
    for (const OverrideFlowRequest& request : mRuntime.mOverrideFlowRequests) {
        if (request.mBegin) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xBEA400.
bool RBOvershellCom::_HasQueuedEndOverrideFlow() const {
    for (const OverrideFlowRequest& request : mRuntime.mOverrideFlowRequests) {
        if (!request.mBegin) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xBEA430. A new flow draws the seed the
// part draws use; replacing another flow leaves only the release build's
// stripped log.
void RBOvershellCom::_BeginOverrideFlow(OvershellOverrideFlow flow) {
    if (GetOverrideFlow() != kOverrideFlowNone && GetOverrideFlow() != flow) {
        static_cast<void>(GetOverrideFlow());
    }
    mRuntime.mOverrideFlow = flow;
    mRuntime.mRandomSeed = gRand.Int();
    UpdateAll();
}

// Reconstructed from eboot.elf at 0xBEA4A0. A finished song settings flow
// requires the song options; the idle flow forgets its master.
void RBOvershellCom::_EndOverrideFlow(OvershellOverrideFlow flow, bool cancelled) {
    mRuntime.mOverrideFlow = kOverrideFlowNone;
    if (flow == kOverrideFlowSongSettings && !cancelled) {
        SetSongOptionsRequired(true);
    }
    if (flow == kOverrideFlowIdle) {
        ClearIdleMaster();
    }
    OvershellOverrideEndedMsg msg(flow, cancelled);
    _Export(msg.mData);
    UpdateAll();
}

// Reconstructed from eboot.elf at 0xBEA5A0. While the overshell is
// disabled a flow may still end; otherwise no UI transition may run.
bool RBOvershellCom::_CanChangeOverrideFlow(bool beginning) const {
    const bool blocked = _IsGameplayBlocking() || _IsInSompLayoutTransition();
    if (blocked && !beginning) {
        return true;
    }
    return !theUI->IsInTransition();
}

// Reconstructed from eboot.elf at 0xBEA690.
void RBOvershellCom::BeginOverrideFlow(OvershellOverrideFlow flow) {
    if (_CanChangeOverrideFlow(true)) {
        _BeginOverrideFlow(flow);
    } else {
        _QueueBeginOverrideFlow(flow);
    }
}

// Reconstructed from eboot.elf at 0xBEA720. The release build keeps the
// scan of a stripped check.
void RBOvershellCom::_QueueBeginOverrideFlow(OvershellOverrideFlow flow) {
    static_cast<void>(_HasQueuedBeginOverrideFlow());
    mRuntime.mOverrideFlowRequests.push_back(OverrideFlowRequest{true, flow, false});
}

// Reconstructed from eboot.elf at 0xBEA880.
OvershellOverrideFlow RBOvershellCom::GetOverrideFlow() const {
    if (_IsGameplayBlocking()) {
        return kOverrideFlowDisabled;
    }
    if (_IsInSompLayoutTransition()) {
        return kOverrideFlowSompTransition;
    }
    return mRuntime.mOverrideFlow;
}

// Reconstructed from eboot.elf at 0xBEA950. The release build keeps the
// scan of a stripped check.
void RBOvershellCom::_QueueEndOverrideFlow(OvershellOverrideFlow flow, bool cancelled) {
    static_cast<void>(_HasQueuedBeginOverrideFlow());
    mRuntime.mOverrideFlowRequests.push_back(OverrideFlowRequest{false, flow, cancelled});
}

// Reconstructed from eboot.elf at 0xBEAAB0.
void RBOvershellCom::_Export(DataArray* msg) {
    mObject->mEntity->Export(msg);
}

// Reconstructed from eboot.elf at 0xBEAAD0. Inactive, a story mode event,
// a SOMP session's song in the game layout, and the winning, losing,
// cutscene and exiting gameplay states disable the overshell.
bool RBOvershellCom::_IsGameplayBlocking() const {
    if (mRuntime.mActiveStatus == kOvershellInactive) {
        return true;
    }
    if (theGameData.GetGameMode() == kGameModeBTM &&
        (TheRBBTMMgr->HasPendingEvent() || TheRBBTMMgr->mEventActive)) {
        return true;
    }
    if (theRBSompSessionMgr.IsSompSession() && theRBSompSessionMgr.IsSessionSongActive() &&
        theRBMetaState.mCurrentLayout == kLayoutGame) {
        return true;
    }
    const MetaGameplayState state = theRBMetaState.GetMetaGameplayState();
    return state == kMetaGameplayStateWinning || state == kMetaGameplayStateLosing ||
        state == kMetaGameplayStateWatchingCutscene || state == kMetaGameplayStateExiting;
}

// Reconstructed from eboot.elf at 0xBEAB70.
bool RBOvershellCom::_IsInSompLayoutTransition() const {
    return theRBSompSessionMgr.IsSompSession() && mRuntime.mSompLayoutTransition;
}

// Reconstructed from eboot.elf at 0xBEAB90. The disabled flow outlives the
// blocking states, the SOMP transition flow the layout transition.
void RBOvershellCom::_EndBlockedOverrideFlow() {
    if (mRuntime.mOverrideFlow == kOverrideFlowNone) {
        return;
    }
    const bool blocking = _IsGameplayBlocking();
    const bool transition = _IsInSompLayoutTransition();
    if (!blocking && !transition) {
        return;
    }
    if ((blocking && mRuntime.mOverrideFlow != kOverrideFlowDisabled) ||
        (transition && mRuntime.mOverrideFlow != kOverrideFlowSompTransition)) {
        EndOverrideFlow(mRuntime.mOverrideFlow, true);
    }
}

// Reconstructed from eboot.elf at 0xBEADA0.
void RBOvershellCom::_UpdateGameplayBlocking() {
    const bool blocking = _IsGameplayBlocking();
    const bool transition = _IsInSompLayoutTransition();
    if (blocking != mRuntime.mGameplayBlocking || transition != mRuntime.mSompLayoutTransitionBlocking) {
        mRuntime.mGameplayBlocking = blocking;
        mRuntime.mSompLayoutTransitionBlocking = transition;
        UpdateAll();
    }
}

// Reconstructed from eboot.elf at 0xBEAEB0.
bool RBOvershellCom::ShouldBeShowing() const {
    return mRuntime.mActiveStatus != kOvershellInactive && GetOverrideFlow() != kOverrideFlowDisabled;
}

// Reconstructed from eboot.elf at 0xBEAED0.
bool RBOvershellCom::_CanUpdateSlots() const {
    return !theRBSompSessionMgr.IsSompSession() || _IsOnlineStateSettled();
}

// Reconstructed from eboot.elf at 0xBEAF20.
bool RBOvershellCom::IsUserInIdleFlow(BandUser* user) const {
    RBOvershellSlotCom* const slot = GetSlotFromUser(user);
    return slot != nullptr && slot->GetState()->InIdleFlow();
}

// Reconstructed from eboot.elf at 0xBEAFC0.
bool RBOvershellCom::IsUserSlotAllowingInputToShell(BandUser* user) const {
    RBOvershellSlotCom* const slot = GetSlotFromUser(user);
    return slot != nullptr && slot->GetState()->AllowsInputToShell();
}

// Reconstructed from eboot.elf at 0xBEB060.
bool RBOvershellCom::IsUserAllowingInputToShell(BandUser* user) {
    if (!IsUserInIdleFlow(user)) {
        return true;
    }
    return IsUserSlotAllowingInputToShell(user);
}

// Reconstructed from eboot.elf at 0xBEB1C0. Outside a SOMP session: once
// every slot in the song settings flow is ready to play, their users wait,
// and the flow ends when all of them wait; otherwise the waiting users are
// ready again.
void RBOvershellCom::_ResolveReadyToPlayStates() {
    if (theRBSompSessionMgr.IsSompSession()) {
        return;
    }
    eastl::vector<RBOvershellSlotCom*> slots;
    slots.reserve(mRuntime.mSlots.size());
    for (unsigned long index = 0; index < mRuntime.mSlots.size(); ++index) {
        RBOvershellSlotCom* const slot = mRuntime.mSlots[index];
        if (slot != nullptr && slot->InOverrideFlow(kOverrideFlowSongSettings) && slot->GetUser() != nullptr) {
            slots.push_back(slot);
        }
    }
    if (slots.empty()) {
        return;
    }
    bool allReady = true;
    for (unsigned long index = 0; index < slots.size(); ++index) {
        if (slots[index]->GetState()->GetStateID() != kState_ReadyToPlay &&
            slots[index]->GetState()->GetStateID() != kState_ReadyToPlayWait) {
            allReady = false;
            break;
        }
    }
    if (allReady) {
        for (unsigned long index = 0; index < slots.size(); ++index) {
            slots[index]->GetUser()->SetOvershellSlotState(kState_ReadyToPlayWait);
        }
    } else {
        for (unsigned long index = 0; index < slots.size(); ++index) {
            if (slots[index]->GetUser()->GetOvershellSlotState() == kState_ReadyToPlayWait) {
                slots[index]->GetUser()->SetOvershellSlotState(kState_ReadyToPlay);
            }
        }
    }
    if (allReady) {
        for (unsigned long index = 0; index < slots.size(); ++index) {
            if (slots[index]->GetUser()->GetOvershellSlotState() != kState_ReadyToPlayWait) {
                return;
            }
        }
        if (GetOverrideFlow() == kOverrideFlowSongSettings) {
            EndOverrideFlow(kOverrideFlowSongSettings, false);
        }
    }
}

// Reconstructed from eboot.elf at 0xBEBFB0.
void RBOvershellCom::UpdateCharPreviewSceneState() {
    bool needed = false;
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (slot->NeedsCharPreviewScene()) {
            needed = true;
            break;
        }
    }
    if (needed) {
        RBCharacterPreviewMgrCom::ShowPreviewScene();
    } else {
        RBCharacterPreviewMgrCom::HidePreviewScene();
    }
}

// Reconstructed from eboot.elf at 0xBEC020.
int RBOvershellCom::GetNumUsers() const {
    int users = 0;
    for (unsigned long index = 0; index < mRuntime.mSlots.size(); ++index) {
        if (mRuntime.mSlots[index]->GetUser() != nullptr) {
            ++users;
        }
    }
    return users;
}

// Reconstructed from eboot.elf at 0xBEC1E0.
bool RBOvershellCom::InSong() const {
    return mRuntime.mActiveStatus == kOvershellInSong;
}

// Reconstructed from eboot.elf at 0xBEC1F0.
bool RBOvershellCom::AllSlotsReadyToPlay() const {
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (!slot->GetState()->IsReadyToPlay()) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0xBEC240. A slot that is not ready
// pauses; so does no user, or no user that is ready.
bool RBOvershellCom::ShouldPause() const {
    bool noUsers = true;
    bool anyUserReady = false;
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (slot->GetUser() != nullptr) {
            if (slot->GetState()->IsReadyToPlay()) {
                anyUserReady = true;
            }
            noUsers = false;
        }
        if (!slot->GetState()->IsReadyToPlay()) {
            return true;
        }
    }
    return noUsers || !anyUserReady;
}

// Reconstructed from eboot.elf at 0xBEC2F0.
bool RBOvershellCom::AllSlotsHaveChosenBandMember() const {
    if (GetOverrideFlow() != kOverrideFlowChooseBandMembers) {
        return false;
    }
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (slot->GetState()->InChooseBandMembersFlow() && !slot->GetState()->ChooseBandMembersReady()) {
            return false;
        }
    }
    return HasAnyUsers();
}

// Reconstructed from eboot.elf at 0xBEC3C0.
bool RBOvershellCom::HasAnyUsers() const {
    for (unsigned long index = 0; index < mRuntime.mSlots.size(); ++index) {
        if (mRuntime.mSlots[index]->GetUser() != nullptr) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xBEC410. The release build keeps the
// calls of a stripped check.
void RBOvershellCom::_StartDropInFlows() {
    static_cast<void>(_CanJoinUsers());
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        BandUser* const user = slot->GetUser();
        if (user != nullptr && (user->GetTrackType() == kTrackNone || !user->IsFullyInGame())) {
            slot->ForceDropInFlow();
        }
    }
}

// Reconstructed from eboot.elf at 0xBEC4C0.
bool RBOvershellCom::_IsInGame() const {
    return mRuntime.mActiveStatus == kOvershellInSong;
}

// Reconstructed from eboot.elf at 0xBECA70. A slot's state stands while it
// prevents an override, is in an override flow or its part is chosen; a
// state that needs the online session is left once the overshell is not
// online. The release build keeps the state of a stripped log for an
// empty slot.
void RBOvershellCom::_ResolveSlotStates() {
    _ResolvePartWaitStates();
    _ResolveReadyToPlayStates();
    _ResolveChooseProfileStates();
    for (unsigned long index = 0; index < mRuntime.mSlots.size(); ++index) {
        RBOvershellSlotCom* const slot = mRuntime.mSlots[index];
        BandUser* const user = slot->GetUser();
        if (user == nullptr) {
            static_cast<void>(slot->GetState()->GetStateID());
            continue;
        }
        if (slot->IsReconnectingController()) {
            _ResolveControllerDisconnectedState(slot);
        }
        if (slot->GetState()->PreventsOverride() || _ResolveOverrideFlowState(slot, user) ||
            slot->mRuntime.mPartChosen) {
            continue;
        }
        if (slot->GetState()->RequiresOnlineSession() && !IsOnline()) {
            slot->LeaveOptions(true);
        }
    }
}

// Reconstructed from eboot.elf at 0xBECB80.
void RBOvershellCom::_CheckForControllerDisconnects() {
    if (!_CanUpdateSlots()) {
        return;
    }
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (slot->IsUsingOtherController()) {
            _HandleControllerDisconnect(slot);
        }
    }
}

// Reconstructed from eboot.elf at 0xBECC20.
void RBOvershellCom::QueueUpdateAll() {
    mRuntime.mNeedsUpdateAll = true;
}

// Reconstructed from eboot.elf at 0xBECC30.
int RBOvershellCom::_GetNumJoinedUsers(ControllerType type) const {
    int users = 0;
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        BandUser* const user = slot->GetUser();
        if (user != nullptr && user->GetControllerType() == type) {
            ++users;
        }
    }
    return users;
}

// Reconstructed from eboot.elf at 0xBECC90.
bool RBOvershellCom::IsAnySlotAllowingInputToShell() const {
    for (unsigned long index = 0; index < mRuntime.mSlots.size(); ++index) {
        if (mRuntime.mSlots[index]->GetState()->AllowsInputToShell()) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xBECCF0. Only the slots with a user
// count.
bool RBOvershellCom::IsAnyLocalSlotAllowingInputToShell() const {
    for (unsigned long index = 0; index < mRuntime.mSlots.size(); ++index) {
        if (mRuntime.mSlots[index]->GetUser() != nullptr &&
            mRuntime.mSlots[index]->GetState()->AllowsInputToShell()) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xBECD60. Only the slots with a user
// count.
bool RBOvershellCom::AreAllLocalSlotsAllowingInputToShell() const {
    for (unsigned long index = 0; index < mRuntime.mSlots.size(); ++index) {
        if (mRuntime.mSlots[index]->GetUser() != nullptr &&
            !mRuntime.mSlots[index]->GetState()->AllowsInputToShell()) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0xBECDE0.
bool RBOvershellCom::IsAnySlotJoinable() const {
    for (unsigned long index = 0; index < mRuntime.mSlots.size(); ++index) {
        if (mRuntime.mSlots[index]->GetState()->GetStateID() == kState_Join) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xBECE40. A signed-in user leaves the
// profile choice.
void RBOvershellCom::_ResolveChooseProfileStates() {
    for (unsigned long index = 0; index < mRuntime.mSlots.size(); ++index) {
        RBOvershellSlotCom* const slot = mRuntime.mSlots[index];
        if (slot->GetUser() == nullptr) {
            continue;
        }
        BandUser* const user = slot->GetUser();
        if (slot->GetState()->GetStateID() == kState_ChooseProfile && user->IsSignedIn()) {
            slot->LeaveOptions(true);
        }
    }
}

// Reconstructed from eboot.elf at 0xBECED0. A user whose controller is
// still the wrong one leaves the slot unless the game or the SOMP session
// keeps the user. The release build keeps the calls of a stripped log.
void RBOvershellCom::_ResolveControllerDisconnectedState(RBOvershellSlotCom* slot) {
    static_cast<void>(slot->IsReconnectingController());
    static_cast<void>(slot->GetUser());
    if (slot->IsUsingOtherController()) {
        if (_ShouldKeepSlotUser(slot)) {
            return;
        }
        slot->RemoveUser(true);
    } else {
        _ReconnectSlot(slot);
    }
    UpdateAll();
}

// Reconstructed from eboot.elf at 0xBECFD0. A slot in a song settings flow
// whose song options are required keeps its flow; a slot whose state left
// the flow returns to the state the flow started from.
bool RBOvershellCom::_ResolveOverrideFlowState(RBOvershellSlotCom* slot, BandUser* user) {
    const OvershellSlotStateID state = user->GetOvershellSlotState();
    const OvershellOverrideFlow flow = GetOverrideFlow();
    const OvershellOverrideFlow slotFlow = slot->GetOverrideFlow();
    const bool keepsSongSettings = slotFlow == kOverrideFlowSongSettings && mRuntime.mSongOptionsRequired;
    if (!keepsSongSettings && flow == kOverrideFlowNone && slotFlow != kOverrideFlowNone) {
        slot->EndOverrideFlow(slotFlow, true);
        return true;
    }
    if (_IsSlotOutOfOverrideFlow(slot)) {
        if (slotFlow != kOverrideFlowNone) {
            slot->EndOverrideFlow(slotFlow, false);
        }
        slot->RevertToOverrideFlowReturnState();
        return true;
    }
    _StartSlotOverrideFlow(slot, state);
    return false;
}

// Reconstructed from eboot.elf at 0xBED090.
bool RBOvershellCom::IsOnline() const {
    return static_cast<unsigned int>(mRuntime.mOnlineState - 1) < 2;
}

// Reconstructed from eboot.elf at 0xBED0A0. A user without a profile gets
// the pad's. The release build keeps the calls of a stripped log.
void RBOvershellCom::_ReconnectSlot(RBOvershellSlotCom* slot) {
    static_cast<void>(slot->IsReconnectingController());
    static_cast<void>(slot->GetUser());
    BandUser* const user = slot->GetUser();
    if (user->GetProfile(false) == nullptr) {
        slot->AddProfileToSlot(user->GetPlatformUserId());
    }
    slot->ShowOptions();
}

// Reconstructed from eboot.elf at 0xBED100. In a song or the venue meta
// mode a user fully in the game stays; a SOMP session keeps every user.
bool RBOvershellCom::_ShouldKeepSlotUser(RBOvershellSlotCom* slot) const {
    BandUser* const user = slot->GetUser();
    if (user == nullptr) {
        return false;
    }
    if ((mRuntime.mActiveStatus == kOvershellInSong || mRuntime.mInVenueMetaMode) && user->IsFullyInGame()) {
        return true;
    }
    return theRBSompSessionMgr.IsSompSession();
}

// Reconstructed from eboot.elf at 0xBED160.
void RBOvershellCom::_CheckForSompControllerReconnect() {
    if (!mRuntime.mReconnectDialogShowing && _NeedsSompControllerReconnect()) {
        _ShowReconnectControllerDialog();
    }
}

// Reconstructed from eboot.elf at 0xBED1A0. A modal OK dialog labelled
// "overshell_leave"; it hides once the controller is back. The callbacks
// are the lambdas behind the std::function vtables 0x1971B60 (0xBF6D20)
// and 0x1971BB0 (0xBF6E20).
void RBOvershellCom::_ShowReconnectControllerDialog() {
    static Symbol body;
    if (body == Symbol()) {
        body = Symbol("somp_must_reconnect_original_controller");
    }
    static Symbol leave;
    if (leave == Symbol()) {
        leave = Symbol("overshell_leave");
    }
    DialogMgrCom::DialogArgs args;
    const DialogMgrCom::DialogType type = DialogMgrCom::kDialogTypeOk;
    const DialogMgrCom::DialogModalType modal = DialogMgrCom::kDialogModal;
    const DialogMgrCom::DialogTextLocType bodyLoc = DialogMgrCom::kDialogTextToken;
    const int initialFocus = 0;
    DialogMgrCom::InitForStandardTextPopup(
        args,
        type,
        modal,
        body,
        bodyLoc,
        initialFocus,
        leave,
        [this](DialogMgrCom::Button button, BandUser*) { _OnReconnectControllerDialogHidden(button); },
        [this]() { return !_NeedsSompControllerReconnect(); });
    DialogMgrCom::ShowDialog(args);
    mRuntime.mReconnectDialogShowing = true;
}

// Reconstructed from eboot.elf at 0xBED470. The condition met
// (DialogMgrCom::kButtonConditionMet) means the controller came back.
void RBOvershellCom::_OnReconnectControllerDialogHidden(int button) {
    mRuntime.mReconnectDialogShowing = false;
    if (_NeedsSompControllerReconnect() && button != DialogMgrCom::kButtonConditionMet) {
        static Symbol reason;
        if (reason == Symbol()) {
            reason = Symbol("somp_error_controller_disconnected");
        }
        _DropOutOfSompSession(reason);
    }
}

// Reconstructed from eboot.elf at 0xBED5B0. The session goes to its green
// room.
void RBOvershellCom::_DropOutOfSompSession(Symbol reason) {
    RBSompClient* const client = theRBSompSessionMgr.mClient;
    if (client != nullptr) {
        client->RequestLeaveSong(theRBSompSessionMgr.GetLocalUser());
        theRBSompSessionMgr.GotoLayout(kLayoutSompGreenRoom, reason);
        LeaveOptions();
    }
}

// Reconstructed from eboot.elf at 0xBED650.
bool RBOvershellCom::_IsSlotOutOfOverrideFlow(RBOvershellSlotCom* slot) {
    if (slot->GetState()->InSongSettingsFlow() && !slot->InOverrideFlow(kOverrideFlowSongSettings)) {
        return true;
    }
    if (slot->GetState()->InChooseBandMembersFlow() && !slot->InOverrideFlow(kOverrideFlowChooseBandMembers)) {
        return true;
    }
    if (slot->GetState()->InIdleFlow() && !slot->InOverrideFlow(kOverrideFlowIdle)) {
        return true;
    }
    if (slot->GetState()->InDisabledFlow() && !slot->InOverrideFlow(kOverrideFlowDisabled)) {
        return true;
    }
    if (slot->GetState()->InSompTransitionFlow() && !slot->InOverrideFlow(kOverrideFlowSompTransition)) {
        return true;
    }
    return false;
}

// Reconstructed from eboot.elf at 0xBED750. In a song the song settings
// return to the joined state; the SOMP transition always does.
void RBOvershellCom::_StartSlotOverrideFlow(RBOvershellSlotCom* slot, OvershellSlotStateID returnState) {
    if (slot->InOverrideFlow(kOverrideFlowSongSettings) && !slot->GetState()->InSongSettingsFlow()) {
        slot->SetOverrideFlowReturnState(
            mRuntime.mActiveStatus == kOvershellInSong ? kState_JoinedDefault : returnState);
        slot->BeginOverrideFlow(kOverrideFlowSongSettings, true);
        slot->ShowSongOptions();
    } else if (slot->InOverrideFlow(kOverrideFlowChooseBandMembers) && !slot->GetState()->InChooseBandMembersFlow()) {
        slot->SetOverrideFlowReturnState(returnState);
        slot->BeginOverrideFlow(kOverrideFlowChooseBandMembers, true);
        slot->ShowBandMemberSelect();
    } else if (slot->InOverrideFlow(kOverrideFlowIdle) && !slot->GetState()->InIdleFlow()) {
        slot->SetOverrideFlowReturnState(returnState);
        slot->BeginOverrideFlow(kOverrideFlowIdle, true);
        slot->ShowIdle();
    } else if (slot->InOverrideFlow(kOverrideFlowDisabled) && !slot->GetState()->InDisabledFlow()) {
        slot->SetOverrideFlowReturnState(returnState);
        slot->BeginOverrideFlow(kOverrideFlowDisabled, true);
        slot->ShowDisabled();
    } else if (slot->InOverrideFlow(kOverrideFlowSompTransition) && !slot->GetState()->InSompTransitionFlow()) {
        slot->SetOverrideFlowReturnState(kState_JoinedDefault);
        slot->BeginOverrideFlow(kOverrideFlowSompTransition, true);
        slot->ShowScreenTransition();
    }
}

// Reconstructed from eboot.elf at 0xBEDAF0. Offline or going offline, the
// online slots are forgotten at once.
void RBOvershellCom::_GoOffline() {
    mRuntime.mOnlineStateChanged = true;
    if (static_cast<unsigned int>(mRuntime.mOnlineState) <= kOvershellOnlineStatePendingOffline) {
        mRuntime.mOnlineSlots.clear();
    } else {
        mRuntime.mOnlineState = kOvershellOnlineStatePendingOffline;
        _ResetGoingOfflineSlots();
    }
    UpdateAll();
}

// Reconstructed from eboot.elf at 0xBEDC70.
bool RBOvershellCom::_IsOnlineStateSettled() const {
    const bool pending = mRuntime.mOnlineState == kOvershellOnlineStatePendingOffline ||
        mRuntime.mOnlineState == kOvershellOnlineStatePendingOnline;
    return (!pending || mRuntime.mMovingUser) && !theUI->IsInTransition();
}

// Reconstructed from eboot.elf at 0xBEE7F0.
bool RBOvershellCom::IsPendingOnline() const {
    return mRuntime.mOnlineState == kOvershellOnlineStatePendingOnline;
}

// Reconstructed from eboot.elf at 0xBEE800.
bool RBOvershellCom::IsPendingOffline() const {
    return mRuntime.mOnlineState == kOvershellOnlineStatePendingOffline;
}

// Reconstructed from eboot.elf at 0xBEEBB0.
void RBOvershellCom::PrepareForSompLocalUser() {
    _RemoveUsersOverLimit(GetNumUsers(), 1);
}

// Reconstructed from eboot.elf at 0xBEEC20.
void RBOvershellCom::_RemoveUsersOverLimit(int numUsers, int maxUsers) {
    if (numUsers > maxUsers) {
        for (unsigned long index = 0; index < mRuntime.mSlots.size() && numUsers > maxUsers; ++index) {
            BandUser* const user = mRuntime.mSlots[index]->GetUser();
            if (user != nullptr && !user->mJustDisconnected) {
                mRuntime.mSlots[index]->RemoveUser(true);
                --numUsers;
            }
        }
        for (unsigned long index = 0; numUsers > maxUsers && index < mRuntime.mSlots.size(); ++index) {
            BandUser* const user = mRuntime.mSlots[index]->GetUser();
            if (user != nullptr) {
                user->mJustDisconnected = false;
                mRuntime.mSlots[index]->RemoveUser(true);
                --numUsers;
            }
        }
    }
    for (unsigned long index = 0; index < mRuntime.mSlots.size(); ++index) {
        if (mRuntime.mSlots[index]->GetUser() != nullptr) {
            mRuntime.mSlots[index]->GetUser()->mJustDisconnected = false;
        }
    }
    UpdateAll();
}

// Reconstructed from eboot.elf at 0xBEEDA0. The game mode's
// "max_players" bounds the users.
void RBOvershellCom::_OnGameModeChanged() {
    _RemoveUsersOverLimit(GetNumUsers(), GameMode::Instance().mMaxPlayers);
}

// Reconstructed from eboot.elf at 0xBEEE60.
void RBOvershellCom::_Update(RBOvershellSlotCom* slot) {
    if (slot != nullptr) {
        slot->Update();
    }
}

// Reconstructed from eboot.elf at 0xBEEE70. The users of the profile, or
// of its pad, leave.
void RBOvershellCom::_OnProfileAboutToBeRemoved(RBProfile* profile) {
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        BandUser* const user = slot->GetUser();
        if (user != nullptr &&
            (user->GetProfile(false) == profile || user->GetPlatformUserId() == profile->mPadNum)) {
            slot->AttemptRemoveUser();
        }
    }
    RefreshSlotCharLists();
}

// Reconstructed from eboot.elf at 0xBEEF00.
void RBOvershellCom::RefreshSlotCharLists() {
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        static Symbol refreshCharList;
        if (refreshCharList == Symbol()) {
            refreshCharList = Symbol("refresh_char_list");
        }
        Message<0> msg(refreshCharList);
        slot->Handle(msg.mData, true);
    }
}

// Reconstructed from eboot.elf at 0xBEF050. The users of the profile's pad
// leave.
void RBOvershellCom::_OnProfileFailedToSaveLoad(RBProfile* profile) {
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        BandUser* const user = slot->GetUser();
        if (user != nullptr && user->mPlatformUserId == profile->mPadNum) {
            slot->AttemptRemoveUser();
        }
    }
    RefreshSlotCharLists();
}

// Reconstructed from eboot.elf at 0xBEF0B0.
void RBOvershellCom::_OnPendingProfilesResolved() {
    UpdateAll();
    RefreshSlotCharLists();
    _SetupSlotCharacters();
}

// Reconstructed from eboot.elf at 0xBEF150. The slots that wait for their
// user's profile give the user the profile's character.
void RBOvershellCom::_SetupSlotCharacters() {
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        BandUser* const user = slot->GetUser();
        if (user != nullptr && user->GetProfile(false) != nullptr && slot->mRuntime.mWaitingForProfile) {
            slot->_SetupCurrentCharForUser(user, user->GetProfile(false));
        }
    }
}

// Reconstructed from eboot.elf at 0xBEF460.
void RBOvershellCom::CheckForSompControllerDisconnects() {
    if (theRBSompSessionMgr.IsSompSession()) {
        _DropOutOnSompControllerDisconnect();
    } else {
        _RemoveDisconnectedUsers();
    }
}

// Reconstructed from eboot.elf at 0xBEF4C0.
void RBOvershellCom::_DropOutOnSompControllerDisconnect() {
    bool disconnected = false;
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (IsSlotOnline(slot->mRuntime.mBandSlot) && slot->IsUsingOtherController()) {
            disconnected = true;
        }
    }
    if (disconnected) {
        static Symbol reason;
        if (reason == Symbol()) {
            reason = Symbol("somp_error_controller_disconnected");
        }
        _DropOutOfSompSession(reason);
    }
}

// Reconstructed from eboot.elf at 0xBEF6C0.
void RBOvershellCom::_RemoveDisconnectedUsers() {
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (slot->IsUsingOtherController()) {
            slot->AttemptRemoveUser();
        }
    }
}

// Reconstructed from eboot.elf at 0xBEF710.
bool RBOvershellCom::IsSlotOnline(unsigned long slot) const {
    if (IsOffline()) {
        return true;
    }
    return HasSlot(mRuntime.mOnlineSlots, slot);
}

// Reconstructed from eboot.elf at 0xBEF790.
bool RBOvershellCom::IsOffline() const {
    return mRuntime.mOnlineState == kOvershellOnlineStateOffline ||
        mRuntime.mOnlineState == kOvershellOnlineStatePendingOnline;
}

// Reconstructed from eboot.elf at 0xBEF7B0.
bool RBOvershellCom::_IsOnlineSlot(unsigned long slot) const {
    return HasSlot(mRuntime.mOnlineSlots, slot);
}

// Reconstructed from eboot.elf at 0xBEF820.
bool RBOvershellCom::IsSlotOffline(unsigned long slot) const {
    if (IsOffline()) {
        return false;
    }
    return !HasSlot(mRuntime.mOnlineSlots, slot);
}

// Reconstructed from eboot.elf at 0xBEF8A0.
void RBOvershellCom::MoveOnlineSlot(unsigned long from, unsigned long to) {
    if (HasSlot(mRuntime.mOnlineSlots, from)) {
        mRuntime.mOnlineSlots.erase(from);
    }
    mRuntime.mOnlineSlots.insert(to);
}

// Reconstructed from eboot.elf at 0xBEFA80.
void RBOvershellCom::SetSompStatusTokens(unsigned long slot, Symbol status, Symbol status2, Symbol status3) {
    if (IsOnline() && _IsOnlineSlot(slot)) {
        mRuntime.mSlots[slot]->SetSompStatusTokens(status, status2, status3);
    }
}

// Reconstructed from eboot.elf at 0xBEFB10. Going online again for the same
// slots changes nothing; the release build keeps the comparison of a
// stripped check.
void RBOvershellCom::_GoOnline(const eastl::set<unsigned long>& slots) {
    if (mRuntime.mOnlineState == kOvershellOnlineStateOnline ||
        mRuntime.mOnlineState == kOvershellOnlineStatePendingOnline) {
        static_cast<void>(slots == mRuntime.mOnlineSlots);
    } else {
        mRuntime.mOnlineState = kOvershellOnlineStatePendingOnline;
        mRuntime.mOnlineSlots = slots;
        mRuntime.mOnlineStateChanged = true;
        _ResetOnlineSlots();
    }
    UpdateAll();
}

// Reconstructed from eboot.elf at 0xBEFD70. The slots outside the online
// slots start their transition.
void RBOvershellCom::_ResetOnlineSlots() {
    mRuntime.mOnlineTransitionStartMs = RealTimeMs();
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (IsNotOnlineSlot(slot->mRuntime.mBandSlot)) {
            slot->BeginGoingOnline();
        }
    }
}

// Reconstructed from eboot.elf at 0xBEFE80. The users' band slots.
void RBOvershellCom::GoOnline(const eastl::vector<BandUser*>& users) {
    eastl::set<unsigned long> slots;
    for (BandUser* const user : users) {
        slots.insert(user->GetSlot());
    }
    _GoOnline(slots);
}

// Reconstructed from eboot.elf at 0xBF0010.
void RBOvershellCom::GoOnline(BandUser* user) {
    eastl::set<unsigned long> slots;
    slots.insert(user->GetSlot());
    _GoOnline(slots);
}

// Reconstructed from eboot.elf at 0xBF0150.
void RBOvershellCom::_CancelGoingOnline() {
    mRuntime.mOnlineState = kOvershellOnlineStateOffline;
    mRuntime.mOnlineStateChanged = true;
    mRuntime.mSompDropInStartMs = -1.0F;
    _ResetOfflineSlots();
    UpdateAll();
}

// Reconstructed from eboot.elf at 0xBF0230. The slots outside the online
// slots finish their transition.
void RBOvershellCom::_ResetOfflineSlots() {
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (IsNotOnlineSlot(slot->mRuntime.mBandSlot)) {
            slot->FinishGoingOnline();
        }
    }
}

// Reconstructed from eboot.elf at 0xBF02E0.
void RBOvershellCom::_FinishGoingOnline() {
    mRuntime.mOnlineState = kOvershellOnlineStateOnline;
    mRuntime.mOnlineStateChanged = true;
    _ResetOfflineSlots();
    UpdateAll();
}

// Reconstructed from eboot.elf at 0xBF03B0.
void RBOvershellCom::CancelGoingOnline() {
    if (mRuntime.mOnlineState == kOvershellOnlineStatePendingOnline) {
        _CancelGoingOnline();
    }
}

// Reconstructed from eboot.elf at 0xBF04A0. The slots outside the online
// slots start their transition.
void RBOvershellCom::_ResetGoingOfflineSlots() {
    mRuntime.mOnlineTransitionStartMs = RealTimeMs();
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (IsNotOnlineSlot(slot->mRuntime.mBandSlot)) {
            slot->BeginGoingOffline();
        }
    }
}

// Reconstructed from eboot.elf at 0xBF05B0.
void RBOvershellCom::_FinishGoingOffline() {
    mRuntime.mOnlineState = kOvershellOnlineStateOffline;
    mRuntime.mOnlineStateChanged = true;
    mRuntime.mSompDropInStartMs = -1.0F;
    mRuntime.mOnlineSlots.clear();
    _ResetOfflineSlotsAfterOffline();
    UpdateAll();
}

// Reconstructed from eboot.elf at 0xBF06F0.
void RBOvershellCom::_ResetOfflineSlotsAfterOffline() {
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (IsNotOnlineSlot(slot->mRuntime.mBandSlot)) {
            slot->FinishGoingOffline();
        }
    }
}

// Reconstructed from eboot.elf at 0xBF07A0.
bool RBOvershellCom::IsNotOnlineSlot(unsigned long slot) const {
    return !HasSlot(mRuntime.mOnlineSlots, slot);
}

// Reconstructed from eboot.elf at 0xBF0810. The slots outside the online
// slots must have finished their transition.
bool RBOvershellCom::_AreOnlineSlotsReady() const {
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (IsNotOnlineSlot(slot->mRuntime.mBandSlot) && !slot->IsOnlineTransitionDone()) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0xBF08F0.
bool RBOvershellCom::_AreOnlineSlotUsersReady() const {
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (_IsOnlineSlot(slot->mRuntime.mBandSlot) && !_IsSlotUserReady(slot)) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0xBF09F0. A local user the SOMP session
// marks as its host (RBBandUser::mSompHost).
bool RBOvershellCom::_IsSlotUserReady(RBOvershellSlotCom* slot) const {
    RBBandUser* const user = static_cast<RBBandUser*>(slot->GetUser());
    return user != nullptr && user->IsLocal() && user->mSompHost;
}

// Reconstructed from eboot.elf at 0xBF0A30.
bool RBOvershellCom::_IsNoOnlineSlotUserReady() const {
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (_IsOnlineSlot(slot->mRuntime.mBandSlot) && _IsSlotUserReady(slot)) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0xBF0BA0.
void RBOvershellCom::_UpdatePendingOnlineState() {
    if (mRuntime.mOnlineState == kOvershellOnlineStatePendingOffline) {
        _UpdatePendingOffline();
    } else if (mRuntime.mOnlineState == kOvershellOnlineStatePendingOnline) {
        _UpdatePendingOnline();
    }
}

// Reconstructed from eboot.elf at 0xBF0BC0. Online with the users ready,
// else back offline.
void RBOvershellCom::_UpdatePendingOnline() {
    const bool slotsReady = _AreOnlineSlotsReady();
    const bool usersReady = _AreOnlineSlotUsersReady();
    if (_HasOnlineTransitionTimedOut(slotsReady, usersReady)) {
        if (usersReady) {
            _FinishGoingOnline();
        } else {
            _CancelGoingOnline();
        }
    }
}

// Reconstructed from eboot.elf at 0xBF0F80.
void RBOvershellCom::_UpdatePendingOffline() {
    const bool slotsReady = _AreOnlineSlotsReady();
    const bool usersGone = _IsNoOnlineSlotUserReady();
    if (_HasOnlineTransitionTimedOut(slotsReady, usersGone)) {
        _FinishGoingOffline();
    }
}

// Reconstructed from eboot.elf at 0xBF11F0.
bool RBOvershellCom::_HasOnlineTransitionTimedOut(bool slotsReady, bool usersSettled) const {
    const float elapsed = RealTimeMs() - mRuntime.mOnlineTransitionStartMs;
    return elapsed > kOnlineTransitionTimeoutMs ||
        (usersSettled && (elapsed > kOnlineTransitionMinMs || slotsReady));
}

// Reconstructed from eboot.elf at 0xBF1290. A ready user keeps the
// overshell online while the session and its client last.
void RBOvershellCom::_UpdateOnline() {
    if (mRuntime.mOnlineState != kOvershellOnlineStateOnline) {
        return;
    }
    if (!_IsNoOnlineSlotUserReady() && theRBSompSessionMgr.IsSompSession() &&
        theRBSompSessionMgr.mClient != nullptr) {
        return;
    }
    _GoOffline();
}

// Reconstructed from eboot.elf at 0xBF1540. The transition lasts until
// 2.5 seconds have passed outside UI transitions.
void RBOvershellCom::_PollSompLayoutTransition() {
    if (!mRuntime.mSompLayoutTransition || theUI->IsInTransition()) {
        return;
    }
    const float elapsed = mRuntime.mSompLayoutTransitionTime;
    mRuntime.mSompLayoutTransitionTime += TheTimeMgr->mClock.DeltaSeconds() * 1000.0F;
    if (elapsed >= kSompLayoutTransitionMs) {
        mRuntime.mSompLayoutTransition = false;
        mRuntime.mSompLayoutTransitionTime = -1.0F;
        UpdateAll();
    }
}

// Reconstructed from eboot.elf at 0xBF2610. Only a SOMP session's change to
// another layout starts the transition; a transition under way leaves the
// layout's name of a stripped log.
void RBOvershellCom::OnSompLayoutChanged(RBUILayout layout) {
    if (!theRBSompSessionMgr.IsSompSession()) {
        return;
    }
    if (mRuntime.mSompLayoutTransition) {
        static_cast<void>(RBUILayoutToSym(layout));
    }
    if (theRBMetaState.mCurrentLayout != layout) {
        mRuntime.mSompLayoutTransition = true;
        mRuntime.mSompLayoutTransitionTime = 0.0F;
    }
}

// Reconstructed from eboot.elf at 0xBF2830.
void RBOvershellCom::_PollQueuedUpdateAll() {
    if (mRuntime.mNeedsUpdateAll) {
        UpdateAll();
        mRuntime.mNeedsUpdateAll = false;
    }
}

// Reconstructed from eboot.elf at 0xBF2860. The difficulty select lasts
// "gig_show_difficulty_song_ms" plus "gig_grace_difficulty_song_ms".
bool RBOvershellCom::_OnResourcesLoaded() {
    mRuntime.mInputCom = mObject->GetCom<RBUIInputCom>();
    const DataArrayPtr config = DataReadFile("gig/config.dta", true);
    static Symbol showMs;
    if (showMs == Symbol()) {
        showMs = Symbol("gig_show_difficulty_song_ms");
    }
    static Symbol graceMs;
    if (graceMs == Symbol()) {
        graceMs = Symbol("gig_grace_difficulty_song_ms");
    }
    const int show = config->FindArray(showMs, false)->Int(1);
    mRuntime.mDifficultySelectMs = config->FindArray(graceMs, false)->Int(1) + show;
    return true;
}

// Reconstructed from eboot.elf at 0xBF33D0. Each slot object is the root of
// the entity its "overshell_slots" object instances; the slots join in the
// order guitar, second guitar, vocals, drums.
void RBOvershellCom::_FindSlots() {
    mRuntime.mJoinOrder.clear();
    for (unsigned long index = 0; index < kNumGameSlots; ++index) {
        const GameObjectId id = mOvershellSlots[index];
        if (id.mId == gNullObjectId.mId) {
            continue;
        }
        const InstanceCom* const instance = mObject->mEntity->GetObject(id)->GetBaseCom<InstanceCom>();
        if (instance == nullptr) {
            continue;
        }
        RBOvershellSlotCom* const slot = instance->mRuntime.mEntity->GetRoot()->GetCom<RBOvershellSlotCom>();
        if (slot != nullptr) {
            slot->mRuntime.mGameSlot = static_cast<GameSlot>(index);
            slot->mRuntime.mBandSlot = index;
            slot->SetInputCom(mRuntime.mInputCom);
            slot->mRuntime.mOvershell = this;
            unsigned long order;
            switch (index) {
            case kGameSlotGuitar0:
                slot->AddValidController(kControllerGuitar);
                order = 0;
                break;
            case kGameSlotDrum:
                slot->AddValidController(kControllerDrum);
                order = 3;
                break;
            case kGameSlotVocals:
                slot->AddValidController(kControllerVocals);
                order = 2;
                break;
            case kGameSlotGuitar1:
                slot->AddValidController(kControllerGuitar);
                order = 1;
                break;
            default:
                order = static_cast<unsigned long>(-1);
                break;
            }
            mRuntime.mJoinOrder.push_back(order);
        }
        mRuntime.mSlots[index] = slot;
    }
}

// Reconstructed from eboot.elf at 0xBF36A0. The pads' lights follow the
// layout: the slot colors in the game, the inactive color elsewhere; an
// open system overlay pauses the song.
void RBOvershellCom::_Poll() {
    theRBMetaState.mAnySlotJoinable = IsAnySlotJoinable();
    const RBUILayout layout = theRBMetaState.mCurrentLayout;
    if (mRuntime.mLightLayout != layout) {
        if (layout == kLayoutGame) {
            for (int pad = 0; pad < kNumPads; ++pad) {
                JoypadSetColorInactive(pad, false);
            }
        } else {
            for (int pad = 0; pad < kNumPads; ++pad) {
                JoypadSetColorInactive(pad, true);
                JoypadResetColor(pad);
            }
        }
        mRuntime.mLightLayout = layout;
    }
    if (layout == kLayoutGame) {
        for (int pad = 0; pad < kNumPads; ++pad) {
            JoypadSetColor(pad, RBLightingStateCom::GetJoyPadColor(pad));
        }
    }
    _PollSystemOverlay();
    if (mRuntime.mSystemOverlayOpen && mRuntime.mActiveStatus == kOvershellInSong && theGame != nullptr &&
        !TheTimeMgr->mClock.IsPaused(kTimeUnitsSeconds)) {
        theMutableGame->OvershellSetPaused(true);
    }
    _PollRemoteUsers();
    _EndBlockedOverrideFlow();
    _UpdateGameplayBlocking();
    _CheckForSompControllerReconnect();
    _UpdatePendingOnlineState();
    _UpdateOnline();
    _PollSompDropIn();
    _PollSompLayoutTransition();
    _ProcessOverrideFlowRequests();
    _PollQueuedUpdateAll();
    if (mRuntime.mNeedsDrawReset) {
        mObject->mEntity->QueueForPostPoll(this);
    }
}

// Reconstructed from eboot.elf at 0xBF3C70.
void RBOvershellCom::_EditPoll() {
    if (mRuntime.mNeedsFindSlots) {
        _FindSlots();
        mRuntime.mNeedsFindSlots = false;
    }
}

// Reconstructed from eboot.elf at 0xBF3CA0. The scene is the first
// RndSceneCom of the entity's root, else of the roots of the entities
// that instance it.
void RBOvershellCom::_PostPoll() {
    if (!mRuntime.mNeedsDrawReset) {
        return;
    }
    mRuntime.mNeedsDrawReset = false;
    Entity* entity = mObject->mEntity;
    while (true) {
        RndSceneCom* const scene = entity->GetRoot()->GetCom<RndSceneCom>();
        if (scene != nullptr) {
            scene->ResetDrawRegistrations();
            break;
        }
        GameObject* const parent = entity->mParentObject;
        if (parent == nullptr || parent->mEntity == nullptr) {
            break;
        }
        entity = parent->mEntity;
    }
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        slot->_ClearListData();
    }
}

// Reconstructed from eboot.elf at 0xBF3D90. The new layout's status and
// the end of a SOMP layout transition; _PostPoll resets the draw
// registrations.
void RBOvershellCom::_OnLayoutTransitionDone() {
    theRBTrackLayoutData.SetNeedsRefresh();
    mRuntime.mJoinBlocked = false;
    _ChooseActiveStatusFromMetaState();
    if (mRuntime.mSompLayoutTransition) {
        mRuntime.mSompLayoutTransition = false;
        mRuntime.mSompLayoutTransitionTime = -1.0F;
        UpdateAll();
    }
    mRuntime.mNeedsDrawReset = true;
}

// Reconstructed from eboot.elf at 0xBF3E10.
DataNode RBOvershellCom::OnMsg(const JoypadConnectionMsg& msg) {
    static_cast<void>(msg);
    UpdateAll();
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xBF3E40.
DataNode RBOvershellCom::OnMsg(const GameModeChangedMsg& msg) {
    static_cast<void>(msg);
    _OnGameModeChanged();
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xBF3E70. The vocal slot's user applies
// the microphone volumes, in the game's mode while a game exists, and its
// instrument options show the microphones again.
DataNode RBOvershellCom::OnMsg(const GameMicsChangedMsg& msg) {
    static_cast<void>(msg);
    const bool inGame = theGame != nullptr;
    UpdateAll();
    RBOvershellSlotCom* const slot = GetSlot(kGameSlotVocals);
    BandUser* const user = slot->GetUser();
    if (user != nullptr) {
        static_cast<RBBandUser*>(user)->ApplyVocalistVolumeSettings(inGame);
        slot->RefreshInstrumentOptions();
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xBF3F10. The game's "error_zero_players"
// reaches the overshell (mErrorZeroPlayersSink).
DataNode RBOvershellCom::OnMsg(const GameCreatedMsg& msg) {
    static_cast<void>(msg);
    // Component begins with MsgSink's slots; the model does not derive it.
    theMutableGame->AddSink(
        &mRuntime.mErrorZeroPlayersSink, reinterpret_cast<MsgSink*>(this), Symbol("error_zero_players"), Symbol());
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xBF4020. The vocal slot's user applies
// the microphone volumes of the meta screens; every slot leaves its
// options.
DataNode RBOvershellCom::OnMsg(const GameEndedMsg& msg) {
    static_cast<void>(msg);
    BandUser* const user = mRuntime.mSlots[kGameSlotVocals]->GetUser();
    if (user != nullptr) {
        static_cast<RBBandUser*>(user)->ApplyVocalistVolumeSettings(false);
    }
    LeaveOptions();
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xBF40B0. As OnMsg(GameEndedMsg).
DataNode RBOvershellCom::OnMsg(const GameAboutToBeDestroyedMsg& msg) {
    static_cast<void>(msg);
    BandUser* const user = mRuntime.mSlots[kGameSlotVocals]->GetUser();
    if (user != nullptr) {
        static_cast<RBBandUser*>(user)->ApplyVocalistVolumeSettings(false);
    }
    LeaveOptions();
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xBF4140.
DataNode RBOvershellCom::OnMsg(const SystemOverlayStatusChangedMsg& msg) {
    if (msg.IsShowing()) {
        _OnSystemOverlayOpened();
    } else {
        _OnSystemOverlayClosed();
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xBF4190.
void RBOvershellCom::_OnSystemOverlayOpened() {
    mRuntime.mSystemOverlayOpen = true;
}

// Reconstructed from eboot.elf at 0xBF41A0. _Poll handles the closed
// overlay once the platform reports it gone.
void RBOvershellCom::_OnSystemOverlayClosed() {
    mRuntime.mSystemOverlayOpen = false;
    mRuntime.mSystemOverlayClosed = true;
}

// Reconstructed from eboot.elf at 0xBF41B0. A pad whose profile changed
// moves the slot to the pad's profile; a pad without one takes the
// profile away unless the controller is being reconnected or the game or
// the SOMP session keeps the user.
void RBOvershellCom::_RefreshSlotProfile(RBOvershellSlotCom* slot) {
    BandUser* const user = slot->GetUser();
    RBProfile* const slotProfile = theRBProfileMgr->GetProfileFromSlotIdx(slot->mRuntime.mGameSlot);
    const int padNum = user->mPlatformUserId;
    RBProfile* const padProfile = padNum != -1 ? theRBProfileMgr->CreateProfile<RBProfile>(padNum) : nullptr;
    if (slotProfile == padProfile) {
        return;
    }
    if ((slotProfile == nullptr && padProfile != nullptr) || (slotProfile != nullptr && padProfile != nullptr)) {
        slot->RemoveProfileFromSlot();
        slot->AddProfileToSlot(padNum);
        return;
    }
    if (slot->IsReconnectingController() || _ShouldKeepSlotUser(slot)) {
        return;
    }
    slot->RemoveProfileFromSlot();
}

// Reconstructed from eboot.elf at 0xBF45C0. Only the online slots' users
// count (every slot while offline); -1 without one.
float RBOvershellCom::GetSompMinWaitTime() const {
    float minWait = kNoWaitTime;
    bool any = false;
    for (RBOvershellSlotCom* const slot : mRuntime.mSlots) {
        if (IsSlotOnline(slot->mRuntime.mBandSlot) && slot->GetUser() != nullptr) {
            const float wait = slot->GetSompWaitTime();
            minWait = wait < minWait ? wait : minWait;
            any = true;
        }
    }
    return any ? minWait : -1.0F;
}

// Reconstructed from eboot.elf at 0xBF6370.
Symbol RBOvershellCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0xBF6380.
Symbol RBOvershellCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0xBF6390.
int RBOvershellCom::CurrentRev() const {
    return const_cast<RBOvershellCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0xBF63B0.
bool RBOvershellCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xBF63E0.
Component* RBOvershellCom::AsComponent() {
    return this;
}

// Inlined into _Imprint (0xBF63F0): the properties are copied and the
// run-time state starts afresh.
RBOvershellCom::RBOvershellCom(const RBOvershellCom& other)
    : Component(other), mRuntime(), mOvershellSlots(other.mOvershellSlots) {}

// Reconstructed from eboot.elf at 0xBF63F0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RBOvershellCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RBOvershellCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RBOvershellCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0xBF6580.
PropRegistry& RBOvershellCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0xBF6590.
ComMetaData& RBOvershellCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0xBF6610.
OvershellOverrideEndedMsg::OvershellOverrideEndedMsg(OvershellOverrideFlow flow, bool cancelled)
    : Message<2>(Event()) {
    mData->Node(2) = DataNode(static_cast<int>(flow));
    mData->Node(3) = DataNode(static_cast<int>(cancelled));
}

// Reconstructed from eboot.elf at 0xBAB940. The binary emits the factory
// with the other rb_meta factories.
Component* RBOvershellCom::_Create() {
    return new RBOvershellCom();
}

// Reconstructed from eboot.elf at 0xB87E00. The binary emits the
// registration with the other rb_meta classes'.
void RBOvershellCom::Init() {
    InitComponentClass<RBOvershellCom>();
}
