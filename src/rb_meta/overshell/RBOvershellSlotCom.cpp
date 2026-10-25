// rb_meta/RBOvershellSlotCom.o (0xC05910 to 0xC24AFF). The properties'
// accessors are the lambdas after the identity slots (0xC24260-0xC246C0).
// The compiler's code: the std::function of _SelectDifficulty's dialog
// callback (vtable 0x19723F0, 0xC246D0-0xC24810) and the message queue's
// DoReallocPtrArray (0xC24820, eastl::deque). The static initializer is at
// 0xC24990; the shared header statics it sets first (0x1B03E48-0x1B03EA0)
// belong to headers that are not modelled. The class's registration and
// factory are emitted elsewhere (0xB88970, 0xBABA30). The functions that
// call into classes not modelled yet stay declared in the header with
// their addresses.
#include "rb_meta/overshell/RBOvershellSlotCom.h"

#include <new>

#include "anim/props/PropertyBlenderCom.h"
#include "entity/core/Entity.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropUtl.h"
#include "mic/core/MicChannelMgr.h"
#include "os/joypads/Joypad.h"
#include "os/locale/Locale.h"
#include "persistence/profiles/Profile.h"
#include "rb_game/mics/GameMic.h"
#include "rb_game/mics/GameMicManager.h"
#include "rb_game/options/GameplayOptions.h"
#include "rb_game/users/BandUser.h"
#include "rb_game/users/BandUserMgr.h"
#include "rb_game/users/RBBandUser.h"
#include "rb_game/users/RBBandUserMgr.h"
#include "rb_game/users/RemoteBandUser.h"
#include "rb_meta/character/RBCharacterMenuMgr.h"
#include "rb_meta/input/RBUIInputCom.h"
#include "rb_meta/overshell/RBOvershellCom.h"
#include "rb_meta/overshell/RBOvershellListProviderCom.h"
#include "rb_meta/overshell/RBOvershellSlotState.h"
#include "rb_meta/persistentdata/RBCharacterPersistentData.h"
#include "rb_meta/profiles/RBProfile.h"
#include "rb_meta/profiles/RBProfileMgr.h"
#include "rb_meta/profiles/RBSystemOptions.h"
#include "rb_meta/somp/RBSompSessionMgr.h"
#include "rb_meta/songs/SongMetadata.h"
#include "rb_meta/songs/SongMgr.h"
#include "rb_meta/state/RBMetaStateCom.h"
#include "render/fonts/RndTextCom.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndEntityResource.h"
#include "ui/input/UIInputReceiverCom.h"
#include "ui/input/UIMsg.h"
#include "ui/lists/UIListCom.h"
#include "ui/manager/UIMgr.h"
#include "utl/data/DataArray.h"
#include "utl/messages/Message.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"
#include "utl/time/TimeMgr.h"

// The game's data (rb_game/RBGameData.o), which is not modelled; declared
// here as RBAchPersistentData_PS4.cpp does. The game mode is an RBGameMode.
// Only the members the slot calls are declared.
class RBGameData : public MsgSource {
public:
    // The songs the game is about to load. The map's
    // GetSongsToLoadMetaSafe(eastl::vector<Symbol>&) const; this build
    // returns the list.
    eastl::vector<Symbol> GetSongsToLoadMetaSafe() const;  // 0x966360
    int GetGameMode() const;  // 0x965E50
    // The in-song menu's quit and skip tokens for the game mode
    // ("get_current_overshell_quit_token" and "..._skip_token"). Names not
    // in the reference map.
    Symbol GetCurrentOvershellQuitToken() const;  // 0x968C10
    Symbol GetCurrentOvershellSkipToken() const;  // 0x968F50
};

extern RBGameData theGameData;  // 0x1AE1818

// The game while a song plays (the map's theGame), not modelled.
class Game;
extern Game* theGame;  // 0x1AEF770

// A polymorphic singleton of the game (vtable 0x1960D78) that the trophies
// and the in-song menu ask about the show; state 8 lets a user leave
// without confirmation. Not modelled; the class, global and field names are
// not in the reference map and the evidence for them is weak.
class RBGameSession {
public:
    unsigned char mOpaque[0x68];  // Not modelled.
    int mState;
};

extern RBGameSession* theRBGameSession;  // 0x1AE4938

// The data the game's track and overshell layouts are built from
// (rb_game/RBTrackLayoutData.o), not modelled: just what the slot calls.
class RBTrackLayoutData {
public:
    // Marks the data for a refresh, under the object's lock.
    void SetNeedsRefresh();  // 0x99E690
};

extern RBTrackLayoutData theRBTrackLayoutData;  // 0x1AE5AD8

namespace {

// The slot's exported events, built by the static initializer before the
// class statics. Names not in the reference map.
Symbol gOnCancel("on_cancel");
Symbol gOnStart("on_start");
Symbol gOnSelect("on_select");
Symbol gOnSelectDisabled("on_select_disabled");
Symbol gOnWidgetScroll("on_widget_scroll");

// The track types "display_track_type" offers beyond TrackType's, which
// _StartDropInFlow marks a joining user's part with: a part still to be
// chosen, and a vocal part still to be chosen. Names not in the reference
// map; the labels are the evidence.
constexpr TrackType kTrackPending = static_cast<TrackType>(7);
constexpr TrackType kTrackPendingVocals = static_cast<TrackType>(8);

// The states OvershellSlotStateToString leaves unnamed: a user waits to join
// a full overshell (2, the "join_blocked" view), the text slider of the
// instrument options (22) and the track texture menu (58). Names not in the
// reference map.
constexpr OvershellSlotStateID kStateJoinBlocked = static_cast<OvershellSlotStateID>(2);
constexpr OvershellSlotStateID kStateOptionsTextSlider = static_cast<OvershellSlotStateID>(22);
constexpr OvershellSlotStateID kStateTrackTextureMenu = static_cast<OvershellSlotStateID>(58);

// The pad "locked_to_pad_num" names when the list's receiver takes no
// pad's input in particular. Name not in the reference map; the evidence
// is weak.
constexpr int kNoLockedPad = 8;

// How long a state waits for the server before the session gives up, and
// how long a queued message shows, in milliseconds. Names not in the
// reference map.
constexpr float kServerWaitTimeoutMs = 15000.0F;
constexpr float kMessageShowMs = 4000.0F;

// The joypad actions the slot turns into its own events (JoypadAction).
// Names not in the reference map.
constexpr int kActionCancel = 2;
constexpr int kActionStart = 4;
constexpr int kActionViewModify = 5;

// RBGameData's game modes the overshell tests. Names not in the reference
// map; 7 is practice (no trophies), the others are weak: 8 is the mode
// without "allow_continue_after_difficulty_change" (BTM), 1 and 6 the modes
// with the venue meta mode.
constexpr int kGameModeQuickplay = 1;
constexpr int kGameModeVenueMeta = 6;
constexpr int kGameModePractice = 7;
constexpr int kGameModeBTM = 8;

// theRBGameSession's state in which a user may leave a song at once. Name
// not in the reference map; the evidence is weak.
constexpr int kGameSessionStateEnded = 8;

// The character types _GetCharTypeFromSymbols gives: one of the user's
// profile's characters, a prefab, another user's character. Names not in
// the reference map (RBBandUser::CharType names only the none value).
constexpr RBBandUser::CharType kCharTypePlayer = static_cast<RBBandUser::CharType>(0);
constexpr RBBandUser::CharType kCharTypePrefab = static_cast<RBBandUser::CharType>(1);
constexpr RBBandUser::CharType kCharTypeOtherUser = static_cast<RBBandUser::CharType>(2);

// The slot's default height bound ("max_height"). Name not in the
// reference map.
constexpr float kDefaultMaxHeight = 246.0F;

// The milliseconds of the real time, the slot's input and message clock.
// Inlined from TimeMgr's timer. Name not in the reference map.
float RealTimeMs() {
    return TheTimeMgr->mRealTime.SplitMs();
}

// A script node holding an object pointer, such as the slot as a message
// argument. Name not in the reference map.
DataNode ObjectNode(void* object) {
    DataNode node;
    node.mType = kDataObject;
    node.mValue.object = object;
    return node;
}

// Whether the state is one of the instrument options' states, which
// RefreshInstrumentOptions shows again. Ids 59 and 60 have no name either.
// Name not in the reference map.
bool IsInstrumentOptionsState(OvershellSlotStateID state) {
    switch (static_cast<int>(state)) {
    case kState_OptionsDrumMessage:
    case kState_OptionsDrum:
    case kState_OptionsVocal:
    case kState_OptionsSlider:
    case kStateOptionsTextSlider:
    case kState_OptionsGuitar:
    case kState_ToggleCymbalConfirm:
    case 59:
    case 60:
        return true;
    default:
        return false;
    }
}

// Sends the slot to the list provider with the message type. Inlined into
// the list setups. Name not in the reference map.
void SendToListProvider(RBOvershellSlotCom* slot, Symbol type) {
    Message<1> msg(type);
    msg.mData->Node(2) = ObjectNode(slot);
    static_cast<void>(slot->mRuntime.mListProvider->Handle(msg.mData, true));
}

// Sends the message to the state. Inlined into UpdateState. Name not in the
// reference map.
void SendToState(RBOvershellSlotState* state, Symbol type) {
    const Message<0> msg(type);
    static_cast<void>(state->HandleMsg(msg.mData));
}

// Sends the options to the SOMP session's client, if there is one. Inlined
// into the option changes. Name not in the reference map.
void SendSompOptions(BandUser* user, IGameplayOptions* options) {
    RBSompClient* const client = theRBSompSessionMgr.mClient;
    if (client != nullptr) {
        client->SendGameplayOptions(
            user, options, &theRBProfileMgr->mSystemOptions, theRBSompSessionMgr.IsSessionNoFail());
    }
}

// The views the slot's states show ("current_view"), each named by its own
// symbol. Name not in the reference map.
eastl::vector<AllowedValue<Symbol>> GetViewAllowedValues() {
    eastl::vector<AllowedValue<Symbol>> values;
    auto add = [&values](const Symbol& view) {
        values.push_back(AllowedValue<Symbol>{view, String(view.Str()), String("")});
    };
    static Symbol noInstrument;
    if (noInstrument == Symbol()) {
        noInstrument = Symbol("no_instrument");
    }
    add(noInstrument);
    static Symbol join;
    if (join == Symbol()) {
        join = Symbol("join");
    }
    add(join);
    static Symbol joinBlocked;
    if (joinBlocked == Symbol()) {
        joinBlocked = Symbol("join_blocked");
    }
    add(joinBlocked);
    static Symbol joinedDefault;
    if (joinedDefault == Symbol()) {
        joinedDefault = Symbol("joined_default");
    }
    add(joinedDefault);
    static Symbol listShort;
    if (listShort == Symbol()) {
        listShort = Symbol("list_short");
    }
    add(listShort);
    static Symbol listMedium;
    if (listMedium == Symbol()) {
        listMedium = Symbol("list_medium");
    }
    add(listMedium);
    static Symbol listLong;
    if (listLong == Symbol()) {
        listLong = Symbol("list_long");
    }
    add(listLong);
    static Symbol readyToPlay;
    if (readyToPlay == Symbol()) {
        readyToPlay = Symbol("ready_to_play");
    }
    add(readyToPlay);
    static Symbol confirmAction;
    if (confirmAction == Symbol()) {
        confirmAction = Symbol("confirm_action");
    }
    add(confirmAction);
    static Symbol confirmActionLong;
    if (confirmActionLong == Symbol()) {
        confirmActionLong = Symbol("confirm_action_long");
    }
    add(confirmActionLong);
    static Symbol reconnectController;
    if (reconnectController == Symbol()) {
        reconnectController = Symbol("reconnect_controller");
    }
    add(reconnectController);
    static Symbol master;
    if (master == Symbol()) {
        master = Symbol("master");
    }
    add(master);
    static Symbol notMaster;
    if (notMaster == Symbol()) {
        notMaster = Symbol("not_master");
    }
    add(notMaster);
    static Symbol sompInactive;
    if (sompInactive == Symbol()) {
        sompInactive = Symbol("somp_inactive");
    }
    add(sompInactive);
    static Symbol sompHighLatency;
    if (sompHighLatency == Symbol()) {
        sompHighLatency = Symbol("somp_high_latency");
    }
    add(sompHighLatency);
    static Symbol sompDisconnecting;
    if (sompDisconnecting == Symbol()) {
        sompDisconnecting = Symbol("somp_disconnecting");
    }
    add(sompDisconnecting);
    static Symbol sompDisconnectedMessage;
    if (sompDisconnectedMessage == Symbol()) {
        sompDisconnectedMessage = Symbol("somp_disconnected_message");
    }
    add(sompDisconnectedMessage);
    static Symbol sompPendingOnline;
    if (sompPendingOnline == Symbol()) {
        sompPendingOnline = Symbol("somp_pending_online");
    }
    add(sompPendingOnline);
    static Symbol sompPendingOffline;
    if (sompPendingOffline == Symbol()) {
        sompPendingOffline = Symbol("somp_pending_offline");
    }
    add(sompPendingOffline);
    return values;
}

// The slot of the property accessors' component. Name not in the
// reference map.
RBOvershellSlotCom* AccessedSlot(const PropAccessorArgs& args) {
    return static_cast<RBOvershellSlotCom*>(args.mComponent);
}

// Sets the read-only, unsaved flags of the slot's run-time views. Name not
// in the reference map.
void SetRuntimeView(PropMetadata& metadata) {
    metadata.mHistory = false;
    metadata.mReadOnly = true;
    metadata.mSaves = false;
}

}  // namespace

// The object's statics, in the order of its static initializer (0xC24990).
Symbol RBOvershellSlotCom::sId("RBOvershellSlotCom");
Symbol RBOvershellSlotCom::sClassName("RBOvershellSlotCom");
PropRegistry RBOvershellSlotCom::sPropRegistry;
ComMetaData RBOvershellSlotCom::sMetaData;

// Reconstructed from eboot.elf at 0xC05910. The queued users and the
// controllers reserve room for ten; the states are created here and read by
// _OnResourcesLoaded.
RBOvershellSlotCom::RuntimeData::RuntimeData()
    : mGameSlot(kGameSlotGuitar0),
      mInputCom(nullptr),
      mOvershell(nullptr),
      mConfirmActionTextCom(nullptr),
      mDrawNode(nullptr),
      mStateBlenderComs(),
      mListProvider(nullptr),
      mUIList(nullptr),
      mInputReceiver(nullptr),
      mState(nullptr),
      mPreviousState(nullptr),
      mStateMgr(new RBOvershellSlotStateMgr()),
      mOverrideFlowReturnState(kState_JoinedDefault),
      mCurrentView(),
      mActiveStatus(kOvershellInMeta),
      mBandSlot(static_cast<unsigned long>(-1)),
      mPartChosen(false),
      mSlotName(),
      mSlotLabel(),
      mSompStatusTokens(),
      mValidControllers(),
      mPotentialUsers(),
      mCharManagementSession(false),
      mManagedCharName(),
      mManagedCharType(),
      mManagedCharOwner(),
      mOverrideFlow(kOverrideFlowNone),
      mPreviewTrackType(kTrackNone),
      mNumHarmonyMicsToDisplay(0),
      mVocalFreestyleIconOverride(false),
      mProDrumsIconOverride(false),
      mTooltipShowing(false),
      mTooltipButton(static_cast<JoypadAction>(0)),
      mTooltipToken(),
      mMicMonitorsShowing(false),
      mNeedsCharPreviewScene(false),
      mWaitingForProfile(false),
      mInputDisabled(false),
      mAccountPickerUser(nullptr),
      mCymbalTogglePending(false),
      mCymbalToggleLane(-1),
      mSompUpdatePending(false),
      mLastSompUpdateMs(0.0F),
      mKickTargetSet(false),
      mKickTargetName(),
      mKickTargetId(),
      mKickSucceeded(false),
      mLastInputMs(-1.0F),
      mShowingMessage(false),
      mMessageEndMs(-1.0F),
      mMessageToken(),
      mMessageChanged(false),
      mMessageQueue(),
      mEmoteCategory(-1),
      mEmote(-1),
      mConnectionState(kSlotConnectionOffline),
      mOnlineReturnState(kState_JoinedDefault),
      mOnlineSettingsReturnState(kState_JoinedDefault),
      mExitSubmenuReturnState(kState_JoinedDefault),
      mSompTransitionDone(false),
      mServerWaitStartMs(-1.0F),
      mPendingFocus(false),
      mPendingFocusReceiver(nullptr),
      mSliderReturnState(kState_JoinedDefault) {
    mListFocusSink.mSink = nullptr;
    mListFocusSink.mSource = nullptr;
    mAccountPickerSink.mSink = nullptr;
    mAccountPickerSink.mSource = nullptr;
    mValidControllers.reserve(10);
    mPotentialUsers.reserve(10);
    for (bool& down : mButtonDown) {
        down = false;
    }
    for (float& energy : mMicEnergy) {
        energy = 0.0F;
    }
}

// Reconstructed from eboot.elf at 0xC05DE0. The subscriptions leave their
// sources, the last one first, before the links unlink themselves; the
// states are deleted by the component's destructor.
RBOvershellSlotCom::RuntimeData::~RuntimeData() {
    MsgSource::EventSinkElem* const sinks[] = {&mAccountPickerSink, &mListFocusSink};
    for (MsgSource::EventSinkElem* const sink : sinks) {
        LinkedList::Node& link = sink->mLink;
        if (link.mNext != &link && link.mPrev != &link) {
            sink->mSource->RemoveSink(sink);
        }
    }
}

// Reconstructed from eboot.elf at 0xC05FD0.
void RBOvershellSlotCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RBOvershellSlotCom prototype;
    PropRegistry& ownRegistry = prototype._GetPropRegistry();
    if (&ownRegistry != &registry) {
        registry.AddInheritedRegistry(sClassName, ownRegistry);
    }
    metadata.mDescription = "Holds state for an overshell slot.";
    metadata.mAuthor = "Sheila Porter";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(RndEntityResource::Id());
    metadata.AddExportedEvent(
        gOnCancel, String("The user has backed out (by pressing the B button or its equivalent)."));
    metadata.AddExportedEvent(gOnStart, String("The user has pressed the start button."));
    metadata.AddExportedEvent(gOnSelect, String("The user has selected an option."));
    metadata.AddExportedEvent(gOnSelectDisabled, String("The user has attempted to select a disabled option."));
    metadata.AddExportedEvent(gOnWidgetScroll, String("The user has changed a value in a slider or combo box."));
    const eastl::vector<AllowedValue<Symbol>> views = GetViewAllowedValues();

    GameObjectIdMetadata& list = TypeSpecificMetadata(
        registry.RegisterProp("list", offsetof(RBOvershellSlotCom, mList), kPropertyGameObjectId, sizeof(GameObjectId)),
        prototype.mList);
    list.mDefault = prototype.mList;
    static_cast<void>(Symbol("prop"));
    list.mHelp = String("RBOvershellListProviderCom to configure with view states.");
    {
        eastl::vector<Symbol> required;
        required.push_back(RBOvershellListProviderCom::sId);
        required.push_back(UIListCom::sId);
        list.mRequiredComs = required;
    }

    GameObjectIdMetadata& confirmActionText = TypeSpecificMetadata(
        registry.RegisterProp(
            "confirm_action_text",
            offsetof(RBOvershellSlotCom, mConfirmActionText),
            kPropertyGameObjectId,
            sizeof(GameObjectId)),
        prototype.mConfirmActionText);
    confirmActionText.mDefault = prototype.mConfirmActionText;
    static_cast<void>(Symbol("prop"));
    confirmActionText.mHelp = String("RndTextCom to set error/confirmation text to.");
    {
        eastl::vector<Symbol> required;
        required.push_back(RndTextCom::sId);
        confirmActionText.mRequiredComs = required;
    }

    IntMetadata<int>& slot = TypeSpecificMetadata(registry.RegisterProp("slot", -1, kPropertyInt, 0), 0);
    static_cast<void>(Symbol("prop"));
    {
        eastl::vector<AllowedValue<int>> values;
        values.push_back(AllowedValue<int>{kGameSlotGuitar0, String("kGameSlotGuitar0"), String("")});
        values.push_back(AllowedValue<int>{kGameSlotDrum, String("kGameSlotDrum"), String("")});
        values.push_back(AllowedValue<int>{kGameSlotVocals, String("kGameSlotVocals"), String("")});
        values.push_back(AllowedValue<int>{kGameSlotGuitar1, String("kGameSlotGuitar1"), String("")});
        slot.mAllowedValues = values;
        // At 0xC24260 and 0xC24270. Setting the slot also points the slot
        // at the first band slot.
        slot.mGetter = [](const PropAccessorArgs& args) {
            return static_cast<int>(AccessedSlot(args)->mRuntime.mGameSlot);
        };
        slot.mSetter = [](const PropAccessorArgs& args) {
            RBOvershellSlotCom* const com = AccessedSlot(args);
            com->mRuntime.mGameSlot = static_cast<GameSlot>(*static_cast<const int*>(args.mValue));
            com->mRuntime.mBandSlot = 0;
        };
        slot.mHistory = false;
    }

    IntMetadata<int>& displayTrackType =
        TypeSpecificMetadata(registry.RegisterProp("display_track_type", -1, kPropertyInt, 0), 0);
    static_cast<void>(Symbol("prop"));
    displayTrackType.mHelp = String(
        "The track type to use for the icon. Will use the user's track type, except when in the 'choose part' "
        "state, during which it will use the track type the user is currently focused on.");
    {
        eastl::vector<AllowedValue<int>> values;
        values.push_back(AllowedValue<int>{kTrackGuitar, String("kTrackGuitar"), String("")});
        values.push_back(AllowedValue<int>{kTrackDrum, String("kTrackDrum"), String("")});
        values.push_back(AllowedValue<int>{kTrackBass, String("kTrackBass"), String("")});
        values.push_back(AllowedValue<int>{kTrackVocals, String("kTrackVocals"), String("")});
        values.push_back(AllowedValue<int>{kTrackNone, String("kTrackNone"), String("")});
        values.push_back(AllowedValue<int>{kTrackPending, String("kTrackPending"), String("")});
        values.push_back(AllowedValue<int>{kTrackPendingVocals, String("kTrackPendingVocals"), String("")});
        displayTrackType.mAllowedValues = values;
        // At 0xC24290 (a tail call of GetDisplayTrackType) and 0xC242A0
        // (_SetPreviewTrackType, inlined).
        displayTrackType.mGetter = [](const PropAccessorArgs& args) {
            return static_cast<int>(AccessedSlot(args)->GetDisplayTrackType());
        };
        displayTrackType.mSetter = [](const PropAccessorArgs& args) {
            AccessedSlot(args)->_SetPreviewTrackType(static_cast<TrackType>(*static_cast<const int*>(args.mValue)));
        };
        displayTrackType.mHistory = false;
    }

    IntMetadata<unsigned int>& numHarmonyMics = TypeSpecificMetadata(
        registry.RegisterProp("num_harmony_mics_to_display", -1, kPropertyUInt, 0), 0U);
    static_cast<void>(Symbol("prop"));
    numHarmonyMics.mHelp = String(
        "The number of harmony parts in the upcoming song (0..3; 0 if not on vocal harmony or no song found). To "
        "be used to figure out which harmony icon to display.");
    numHarmonyMics.mDefault = 0U;
    numHarmonyMics.mMin = 0U;
    numHarmonyMics.mMax = 3U;
    // At 0xC242B0 and 0xC242F0.
    numHarmonyMics.mGetter = [](const PropAccessorArgs& args) {
        return AccessedSlot(args)->GetNumHarmonyMicsToDisplay();
    };
    numHarmonyMics.mSetter = [](const PropAccessorArgs& args) {
        AccessedSlot(args)->mRuntime.mNumHarmonyMicsToDisplay = *static_cast<const int*>(args.mValue);
    };

    BoolMetadata& vocalFreestyle =
        TypeSpecificMetadata(registry.RegisterProp("vocal_freestyle_icon_override", -1, kPropertyBool, 0), false);
    static_cast<void>(Symbol("prop"));
    vocalFreestyle.mHelp = String("True if we should use the vocal freestyle icon.");
    vocalFreestyle.mDefault = false;
    // At 0xC24300 and 0xC24340.
    vocalFreestyle.mGetter = [](const PropAccessorArgs& args) {
        return AccessedSlot(args)->GetVocalFreestyleIconOverride();
    };
    vocalFreestyle.mSetter = [](const PropAccessorArgs& args) {
        AccessedSlot(args)->mRuntime.mVocalFreestyleIconOverride = *static_cast<const bool*>(args.mValue);
    };

    BoolMetadata& proDrums =
        TypeSpecificMetadata(registry.RegisterProp("pro_drums_icon_override", -1, kPropertyBool, 0), false);
    static_cast<void>(Symbol("prop"));
    proDrums.mHelp = String("True if we should use the pro drums icon.");
    proDrums.mDefault = false;
    // At 0xC24350 and 0xC24390.
    proDrums.mGetter = [](const PropAccessorArgs& args) {
        return AccessedSlot(args)->GetProDrumsIconOverride();
    };
    proDrums.mSetter = [](const PropAccessorArgs& args) {
        AccessedSlot(args)->mRuntime.mProDrumsIconOverride = *static_cast<const bool*>(args.mValue);
    };

    SymbolMetadata& currentView =
        TypeSpecificMetadata(registry.RegisterProp("current_view", -1, kPropertySymbol, 0), Symbol());
    static_cast<void>(Symbol("prop"));
    currentView.mAllowedValues = views;
    // At 0xC243A0 and 0xC243B0.
    currentView.mGetter = [](const PropAccessorArgs& args) {
        return AccessedSlot(args)->mRuntime.mCurrentView;
    };
    currentView.mSetter = [](const PropAccessorArgs& args) {
        AccessedSlot(args)->mRuntime.mCurrentView = *static_cast<const Symbol*>(args.mValue);
    };
    currentView.mHistory = false;

    SymbolMetadata& activeStatus =
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
            inactive, String(inactive.Str()), String("Overshell is inactive (loading screens, etc)")});
        values.push_back(AllowedValue<Symbol>{inMeta, String(inMeta.Str()), String("Game is in a meta screen.")});
        values.push_back(AllowedValue<Symbol>{inSong, String(inSong.Str()), String("In gameplay.")});
        activeStatus.mAllowedValues = values;
        // At 0xC243D0 and 0xC243E0.
        activeStatus.mGetter = [](const PropAccessorArgs& args) {
            return OvershellActiveStatusToSymbol(AccessedSlot(args)->mRuntime.mActiveStatus);
        };
        activeStatus.mSetter = [](const PropAccessorArgs& args) {
            AccessedSlot(args)->mRuntime.mActiveStatus =
                SymbolToOvershellActiveStatus(*static_cast<const Symbol*>(args.mValue));
        };
        activeStatus.mSaves = false;
        activeStatus.mHistory = false;
    }

    BoolMetadata& isSomp = TypeSpecificMetadata(registry.RegisterProp("is_somp", -1, kPropertyBool, 0), false);
    static_cast<void>(Symbol("prop"));
    isSomp.mHelp = String("True if we are in a SOMP screen or SOMP gameplay (i.e. we're connectd to a server).");
    SetRuntimeView(isSomp);
    // At 0xC24410.
    isSomp.mGetter = [](const PropAccessorArgs& args) {
        static_cast<void>(args);
        return IsSomp();
    };

    BoolMetadata& fullOnline =
        TypeSpecificMetadata(registry.RegisterProp("full_overshell_is_online", -1, kPropertyBool, 0), false);
    static_cast<void>(Symbol("prop"));
    fullOnline.mHelp = String(
        "True if we are viewing the 'online' overshell right now.  Will always be false when not in SOMP.");
    SetRuntimeView(fullOnline);
    // At 0xC24420.
    fullOnline.mGetter = [](const PropAccessorArgs& args) {
        return AccessedSlot(args)->IsFullOvershellOnline();
    };

    IntMetadata<int>& onlineState =
        TypeSpecificMetadata(registry.RegisterProp("overshell_online_state", -1, kPropertyInt, 0), 0);
    static_cast<void>(Symbol("prop"));
    onlineState.mHelp = String(
        "The detailed online/offline state of the overshell, including whether a transition is pending.");
    SetRuntimeView(onlineState);
    onlineState.mAllowedValues = GetOvershellOnlineStateAllowedValues();
    // At 0xC24480.
    onlineState.mGetter = [](const PropAccessorArgs& args) {
        return static_cast<int>(AccessedSlot(args)->GetOvershellOnlineState());
    };

    BoolMetadata& isRemote =
        TypeSpecificMetadata(registry.RegisterProp("this_overshell_slot_is_remote", -1, kPropertyBool, 0), false);
    static_cast<void>(Symbol("prop"));
    isRemote.mHelp = String(
        "When the overshell is online, whether this slot represents a remote user.  This will always be false when "
        "the overshell is offline or when not in SOMP.");
    SetRuntimeView(isRemote);
    // At 0xC244E0.
    isRemote.mGetter = [](const PropAccessorArgs& args) {
        const RBOvershellSlotCom* const com = AccessedSlot(args);
        return com->mRuntime.mOvershell != nullptr && com->mRuntime.mOvershell->IsSlotOffline(com->mRuntime.mBandSlot);
    };

    BoolMetadata& isLeader =
        TypeSpecificMetadata(registry.RegisterProp("slot_is_somp_leader", -1, kPropertyBool, 0), false);
    static_cast<void>(Symbol("prop"));
    isLeader.mHelp = String("True if this slot's user is the SOMP session leader.");
    SetRuntimeView(isLeader);
    // At 0xC24500.
    isLeader.mGetter = [](const PropAccessorArgs& args) {
        if (!IsSomp()) {
            return false;
        }
        const RBOvershellSlotCom* const com = AccessedSlot(args);
        if (com->GetUser() == nullptr) {
            return false;
        }
        return static_cast<const RBBandUser*>(com->GetUser())->mSompLeader;
    };

    SymbolMetadata& slotName = TypeSpecificMetadata(
        registry.RegisterProp(
            "slot_name",
            offsetof(RBOvershellSlotCom, mRuntime) + offsetof(RuntimeData, mSlotName),
            kPropertySymbol,
            sizeof(Symbol)),
        prototype.mRuntime.mSlotName);
    slotName.mDefault = prototype.mRuntime.mSlotName;
    static_cast<void>(Symbol("prop"));
    slotName.mHelp = String("The name of this slot's user.");
    SetRuntimeView(slotName);

    SymbolMetadata& slotLabel = TypeSpecificMetadata(
        registry.RegisterProp(
            "slot_label",
            offsetof(RBOvershellSlotCom, mRuntime) + offsetof(RuntimeData, mSlotLabel),
            kPropertySymbol,
            sizeof(Symbol)),
        prototype.mRuntime.mSlotLabel);
    slotLabel.mDefault = prototype.mRuntime.mSlotLabel;
    static_cast<void>(Symbol("prop"));
    slotLabel.mHelp = String("The already-localized 'label' text for this slot.");
    SetRuntimeView(slotLabel);

    SymbolMetadata& joinToken =
        TypeSpecificMetadata(registry.RegisterProp("slot_join_token", -1, kPropertySymbol, 0), Symbol());
    static_cast<void>(Symbol("prop"));
    joinToken.mHelp = String("The token to use in this slot to prompt a user to press start to join.");
    SetRuntimeView(joinToken);
    // At 0xC24580, a tail call: the token does not depend on the slot.
    joinToken.mGetter = [](const PropAccessorArgs& args) {
        static_cast<void>(args);
        return _GetJoinToken();
    };

    SymbolMetadata& emptyToken =
        TypeSpecificMetadata(registry.RegisterProp("slot_empty_token", -1, kPropertySymbol, 0), Symbol());
    static_cast<void>(Symbol("prop"));
    emptyToken.mHelp = String("The token to use in this slot when no user is present.");
    SetRuntimeView(emptyToken);
    // At 0xC24590.
    emptyToken.mGetter = [](const PropAccessorArgs& args) {
        return AccessedSlot(args)->_GetEmptyToken();
    };

    SymbolMetadata& messageToken =
        TypeSpecificMetadata(registry.RegisterProp("slot_message_token", -1, kPropertySymbol, 0), Symbol());
    static_cast<void>(Symbol("prop"));
    messageToken.mHelp =
        String("If non-empty, this token is a temporary message that should be displayed to the user.");
    SetRuntimeView(messageToken);
    // At 0xC245A0.
    messageToken.mGetter = [](const PropAccessorArgs& args) {
        return AccessedSlot(args)->GetMessageToken();
    };

    SymbolMetadata& forcedView =
        TypeSpecificMetadata(registry.RegisterProp("slot_forced_view", -1, kPropertySymbol, 0), Symbol());
    static_cast<void>(Symbol("prop"));
    forcedView.mHelp = String(
        "If non-empty, the slot's view is being forced to this value.  You should normally just refer to the "
        "'current_view' property in Forge.");
    SetRuntimeView(forcedView);
    // At 0xC24610.
    forcedView.mGetter = [](const PropAccessorArgs& args) {
        return AccessedSlot(args)->GetForcedView();
    };

    BoolMetadata& spinner =
        TypeSpecificMetadata(registry.RegisterProp("show_somp_connection_spinner", -1, kPropertyBool, 0), false);
    static_cast<void>(Symbol("prop"));
    spinner.mHelp = String(
        "If true, the SOMP 'connection spinner' graphic should be active, to indicate we're communicating with the "
        "server or waiting for a response.");
    SetRuntimeView(spinner);
    // At 0xC24620.
    spinner.mGetter = [](const PropAccessorArgs& args) {
        RBOvershellSlotState* const state = AccessedSlot(args)->mRuntime.mState;
        return state != nullptr && state->ShowOnlineSpinner();
    };

    IntMetadata<int>& connectionState = TypeSpecificMetadata(
        registry.RegisterProp("this_overshell_slot_connection_state", -1, kPropertyInt, 0), 0);
    static_cast<void>(Symbol("prop"));
    connectionState.mHelp = String("The SOMP connection status of the user in this overshell slot.");
    SetRuntimeView(connectionState);
    {
        eastl::vector<AllowedValue<int>> values;
        values.push_back(AllowedValue<int>{kSlotConnectionConnected, String("Connected"), String("Connected")});
        values.push_back(AllowedValue<int>{kSlotConnectionIdle, String("Idle"), String("Idle")});
        values.push_back(
            AllowedValue<int>{kSlotConnectionHighLatency, String("High Latency"), String("High Latency")});
        values.push_back(
            AllowedValue<int>{kSlotConnectionDisconnecting, String("Disconnecting"), String("Disconnecting")});
        values.push_back(AllowedValue<int>{kSlotConnectionOffline, String("Offline"), String("Offline")});
        connectionState.mAllowedValues = values;
    }
    // At 0xC24640.
    connectionState.mGetter = [](const PropAccessorArgs& args) {
        return static_cast<int>(AccessedSlot(args)->GetConnectionState());
    };

    BoolMetadata& transitionDone = TypeSpecificMetadata(
        registry.RegisterProp(
            "somp_offline_or_online_transition_done",
            offsetof(RBOvershellSlotCom, mRuntime) + offsetof(RuntimeData, mSompTransitionDone),
            kPropertyBool,
            sizeof(bool)),
        prototype.mRuntime.mSompTransitionDone);
    transitionDone.mDefault = prototype.mRuntime.mSompTransitionDone;
    static_cast<void>(Symbol("prop"));
    transitionDone.mHelp = String(
        "After a transition starts, set this to true to indicate that the animation for this slot is complete.  "
        "When all slots set this the overshell online/offline state will update on the next frame and the "
        "transition will end.");
    transitionDone.mSaves = false;
    transitionDone.mHistory = false;

    PropRegistry& height =
        static_cast<GroupMetadata&>(*registry.RegisterProp("height", 0, kPropertyStruct, 0).mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct"));
    FloatMetadata& baseHeight = TypeSpecificMetadata(
        height.RegisterProp("base_height", offsetof(RBOvershellSlotCom, mBaseHeight), kPropertyFloat, sizeof(float)),
        prototype.mBaseHeight);
    baseHeight.mDefault = prototype.mBaseHeight;
    static_cast<void>(Symbol("prop"));
    baseHeight.mHelp = String("Constant height value - will be added to the dynamic height calculation.");
    baseHeight.mDefault = 0.0F;
    FloatMetadata& maxHeight = TypeSpecificMetadata(
        height.RegisterProp("max_height", offsetof(RBOvershellSlotCom, mMaxHeight), kPropertyFloat, sizeof(float)),
        prototype.mMaxHeight);
    maxHeight.mDefault = prototype.mMaxHeight;
    static_cast<void>(Symbol("prop"));
    maxHeight.mHelp = String("The dynamic height will never exceed this value.");
    maxHeight.mMin = 0.0F;
    maxHeight.mDefault = kDefaultMaxHeight;
    FloatMetadata& dynamicHeight =
        TypeSpecificMetadata(height.RegisterProp("dynamic_height", -1, kPropertyFloat, 0), 0.0F);
    static_cast<void>(Symbol("prop"));
    dynamicHeight.mHelp = String(
        "Dynamic height of the overshell - calculated based on UIListCom properties and the base height.");
    dynamicHeight.mMin = 0.0F;
    dynamicHeight.mReadOnly = true;
    // At 0xC24650: the base height plus a row per showing item, bounded by
    // the maximum; the base height alone without a list.
    dynamicHeight.mGetter = [](const PropAccessorArgs& args) {
        const RBOvershellSlotCom* const com = AccessedSlot(args);
        const UIListCom* const list = com->mRuntime.mUIList;
        if (list == nullptr) {
            return com->mBaseHeight;
        }
        const float rowHeight = list->mSpacing.z;
        const float height = static_cast<float>(list->GetNumShowing()) * rowHeight + com->mBaseHeight;
        return height < com->mMaxHeight ? height : com->mMaxHeight;
    };

    PropInfo& stateBlenders = registry.RegisterPropArray(
        "state_blenders", offsetof(RBOvershellSlotCom, mStateBlenders), sizeof(PropArray<GameObjectId>));
    ArrayMetadata& stateBlendersMetadata = static_cast<ArrayMetadata&>(*stateBlenders.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    stateBlendersMetadata.mHelp = String("PropertyBlenderComs that we're using to transition between states.");
    {
        eastl::vector<Symbol> required;
        required.push_back(PropertyBlenderCom::sId);
        PropRegistry::RegisterPropArrayItem(stateBlenders, kPropertyGameObjectId, sizeof(GameObjectId));
        const GameObjectId nullId = {0xFFFFFFFFU};
        GameObjectIdMetadata& blender = TypeSpecificMetadata(stateBlendersMetadata.mItemInfo, nullId);
        static_cast<void>(Symbol("item"));
        blender.mRequiredComs = required;
        blender.mAllowsDuplicates = false;
        blender.mAllowsNull = false;
    }

    PropRegistry& tooltips =
        static_cast<GroupMetadata&>(*registry.RegisterProp("tooltips", 0, kPropertyStruct, 0).mMetadata.Get())
            .mRegistry;
    static_cast<void>(Symbol("struct"));
    BoolMetadata& tooltipShowing = TypeSpecificMetadata(
        tooltips.RegisterProp(
            "showing",
            offsetof(RBOvershellSlotCom, mRuntime) + offsetof(RuntimeData, mTooltipShowing),
            kPropertyBool,
            sizeof(bool)),
        prototype.mRuntime.mTooltipShowing);
    tooltipShowing.mDefault = prototype.mRuntime.mTooltipShowing;
    IntMetadata<int>& tooltipButton = static_cast<IntMetadata<int>&>(
        *tooltips
             .RegisterProp(
                 "button",
                 offsetof(RBOvershellSlotCom, mRuntime) + offsetof(RuntimeData, mTooltipButton),
                 kPropertyInt,
                 sizeof(JoypadAction))
             .mMetadata.Get());
    tooltipButton.mDefault = prototype.mRuntime.mTooltipButton;
    static_cast<void>(Symbol("prop"));
    tooltipButton.mAllowedValues = GetJoypadActionsAllowedValues();
    SymbolMetadata& tooltipToken = TypeSpecificMetadata(
        tooltips.RegisterProp(
            "token",
            offsetof(RBOvershellSlotCom, mRuntime) + offsetof(RuntimeData, mTooltipToken),
            kPropertySymbol,
            sizeof(Symbol)),
        prototype.mRuntime.mTooltipToken);
    tooltipToken.mDefault = prototype.mRuntime.mTooltipToken;

    PropRegistry& micMonitors =
        static_cast<GroupMetadata&>(*registry.RegisterProp("mic_monitors", 0, kPropertyStruct, 0).mMetadata.Get())
            .mRegistry;
    static_cast<void>(Symbol("struct"));
    BoolMetadata& micShowing = TypeSpecificMetadata(
        micMonitors.RegisterProp(
            "showing",
            offsetof(RBOvershellSlotCom, mRuntime) + offsetof(RuntimeData, mMicMonitorsShowing),
            kPropertyBool,
            sizeof(bool)),
        prototype.mRuntime.mMicMonitorsShowing);
    micShowing.mDefault = prototype.mRuntime.mMicMonitorsShowing;
    const char* const energyNames[] = {"mic_1_energy", "mic_2_energy", "mic_3_energy"};
    for (int mic = 0; mic < 3; ++mic) {
        FloatMetadata& energy = TypeSpecificMetadata(
            micMonitors.RegisterProp(
                energyNames[mic],
                static_cast<int>(
                    offsetof(RBOvershellSlotCom, mRuntime) + offsetof(RuntimeData, mMicEnergy) + mic * sizeof(float)),
                kPropertyFloat,
                sizeof(float)),
            prototype.mRuntime.mMicEnergy[mic]);
        energy.mDefault = prototype.mRuntime.mMicEnergy[mic];
        static_cast<void>(Symbol("prop"));
        energy.mDefault = 0.0F;
        energy.mMin = 0.0F;
        energy.mMax = 1.0F;
    }

    // The held buttons the overshell's art shows: the joypad mapping's
    // green, red, yellow, blue and orange buttons.
    struct ButtonProp {
        const char* mName;
        int mIndex;
    };
    const ButtonProp buttons[] = {
        {"green_button_down", 1},
        {"red_button_down", 2},
        {"yellow_button_down", 3},
        {"blue_button_down", 14},
        {"orange_button_down", 11},
    };
    for (const ButtonProp& button : buttons) {
        BoolMetadata& down = TypeSpecificMetadata(
            registry.RegisterProp(
                button.mName,
                static_cast<int>(
                    offsetof(RBOvershellSlotCom, mRuntime) + offsetof(RuntimeData, mButtonDown) + button.mIndex),
                kPropertyBool,
                sizeof(bool)),
            prototype.mRuntime.mButtonDown[button.mIndex]);
        down.mDefault = prototype.mRuntime.mButtonDown[button.mIndex];
        static_cast<void>(Symbol("prop"));
        SetRuntimeView(down);
    }

    const bool nonStatic = AreNonStaticTagFunctionsAllowed();
    SetNonStaticTagFunctionsAllowed(false);
    metadata.AddExportedEvent(
        Symbol("button_pulse"),
        String("The overshell should flash because someone is trying to join but hitting the wrong button."));
    SetNonStaticTagFunctionsAllowed(nonStatic);
}

// Reconstructed from eboot.elf at 0xC0A530.
RBOvershellSlotCom::RBOvershellSlotCom()
    : mRuntime(),
      mList({0xFFFFFFFFU}),
      mConfirmActionText({0xFFFFFFFFU}),
      mBaseHeight(0.0F),
      mMaxHeight(0.0F),
      mStateBlenders() {}

// Reconstructed from eboot.elf at 0xC0A5C0. The deleting destructor is at
// 0xC0A810. A slot that still holds a user gives the band slot and the
// profile up and resets the user.
RBOvershellSlotCom::~RBOvershellSlotCom() {
    if (mRuntime.mBandSlot != static_cast<unsigned long>(-1) &&
        theBandUserMgr->GetUserFromSlot(mRuntime.mBandSlot) != nullptr) {
        BandUser* const user = GetUser();
        mRuntime.mPartChosen = false;
        theRBProfileMgr->RemoveProfileFromSlot(mRuntime.mBandSlot);
        theBandUserMgr->ClearSlot(user);
        user->mParticipating = false;
        _SetOverrideType(kOverrideFlowNone, false);
        user->Reset();
    }
    delete mRuntime.mStateMgr;
    mRuntime.mStateMgr = nullptr;
}

// Reconstructed from eboot.elf at 0xC0A750.
BandUser* RBOvershellSlotCom::GetUser() const {
    const unsigned long slot = mRuntime.mBandSlot;
    return theBandUserMgr->GetUserFromSlot(slot != static_cast<unsigned long>(-1) ? slot : 0);
}

// Reconstructed from eboot.elf at 0xC0A770. The release build keeps the
// call of a stripped check on the user.
void RBOvershellSlotCom::_SetOverrideType(OvershellOverrideFlow flow, bool update) {
    mRuntime.mOverrideFlow = flow;
    BandUser* const user = GetUser();
    if (user != nullptr && mRuntime.mOverrideFlow != kOverrideFlowNone) {
        static_cast<void>(static_cast<RBBandUser*>(user)->IsLocal());
    }
    _DeactivateCharPreviewScene();
    if (update) {
        if (mRuntime.mOvershell != nullptr) {
            mRuntime.mOvershell->QueueUpdateAll();
        }
        UpdateState();
        UpdateView();
        _QueueSompUpdate();
    }
}

// Reconstructed from eboot.elf at 0xC0A830.
void RBOvershellSlotCom::SetInputCom(RBUIInputCom* input) {
    mRuntime.mInputCom = input;
    if (input != nullptr) {
        input->RegisterCom(this);
    }
}

// Reconstructed from eboot.elf at 0xC0A850.
void RBOvershellSlotCom::SetActiveStatus(OvershellActiveStatus status) {
    mRuntime.mActiveStatus = status;
    if (mRuntime.mListProvider != nullptr) {
        mRuntime.mListProvider->mActiveStatus = status;
    }
}

// Reconstructed from eboot.elf at 0xC0A870.
BandUser* RBOvershellSlotCom::_GetUser() const {
    const unsigned long slot = mRuntime.mBandSlot;
    return theBandUserMgr->GetUserFromSlot(slot != static_cast<unsigned long>(-1) ? slot : 0);
}

// Reconstructed from eboot.elf at 0xC0A890.
void RBOvershellSlotCom::_DropOutUser() {
    if (IsSomp()) {
        _ShowSompDropOutConfirm(false);
    } else {
        AttemptRemoveUser();
    }
}

// Reconstructed from eboot.elf at 0xC0A8B0.
bool RBOvershellSlotCom::IsSomp() {
    return theRBSompSessionMgr.IsSompSession();
}

// Reconstructed from eboot.elf at 0xC0A8C0.
void RBOvershellSlotCom::_ShowSompDropOutConfirm(bool nonLeader) {
    if (GetUser() == nullptr) {
        ShowState(kState_OnlineError);
        return;
    }
    if (!nonLeader && static_cast<RBBandUser*>(GetUser())->mSompLeader) {
        mRuntime.mOnlineReturnState = mRuntime.mState->GetStateID();
        ShowState(kState_OnlineLeaderDropOutConfirm);
    } else {
        mRuntime.mOnlineReturnState = mRuntime.mState->GetStateID();
        ShowState(kState_OnlineNonLeaderDropOutConfirm);
    }
}

// Reconstructed from eboot.elf at 0xC0AA60. A local user already waiting
// to join leaves only the release build's stripped log.
void RBOvershellSlotCom::AddUser(BandUser* user, bool force) {
    if (!force && static_cast<RBBandUser*>(user)->IsLocal() && LookupUserInJoinList(user)) {
        static_cast<void>(user->GetPlatformUserId());
        return;
    }
    _JoinUser(user);
    if (mRuntime.mOvershell != nullptr) {
        mRuntime.mOvershell->QueueUpdateAll();
    }
    UpdateState();
    UpdateView();
    _QueueSompUpdate();
}

// Reconstructed from eboot.elf at 0xC0AB00.
bool RBOvershellSlotCom::LookupUserInJoinList(const BandUser* user) const {
    if (user == nullptr) {
        return false;
    }
    for (BandUser* const potential : mRuntime.mPotentialUsers) {
        if (potential == user) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xC0B2B0.
void RBOvershellSlotCom::_RefreshOvershell(bool now) {
    if (mRuntime.mOvershell != nullptr) {
        if (now) {
            mRuntime.mOvershell->UpdateAll();
            _QueueSompUpdate();
            return;
        }
        mRuntime.mOvershell->QueueUpdateAll();
    }
    UpdateState();
    UpdateView();
    _QueueSompUpdate();
}

// Reconstructed from eboot.elf at 0xC0B910.
void RBOvershellSlotCom::_DeactivateCharPreviewScene() {
    _HideCharPreview();
    mRuntime.mNeedsCharPreviewScene = false;
    mRuntime.mOvershell->UpdateCharPreviewSceneState();
}

// Reconstructed from eboot.elf at 0xC0B940. A remote user's slot mirrors the
// session and sends nothing.
void RBOvershellSlotCom::_QueueSompUpdate() {
    if (!IsFullOvershellOnline() || !mRuntime.mOvershell->IsSlotOnline(mRuntime.mBandSlot)) {
        return;
    }
    if (mRuntime.mState->GetStateID() == kState_MirrorRemoteUser) {
        return;
    }
    BandUser* const user = GetUser();
    if (user != nullptr && !static_cast<RBBandUser*>(user)->IsLocal()) {
        return;
    }
    if (!mRuntime.mSompUpdatePending) {
        mRuntime.mSompUpdatePending = true;
    }
}

// Reconstructed from eboot.elf at 0xC0BA00.
void RBOvershellSlotCom::AddPotentialUser(BandUser* user) {
    mRuntime.mPotentialUsers.push_back(user);
}

// Reconstructed from eboot.elf at 0xC0BAF0.
unsigned long RBOvershellSlotCom::NumPotentialUsers() const {
    return mRuntime.mPotentialUsers.size();
}

// Reconstructed from eboot.elf at 0xC0BB10.
bool RBOvershellSlotCom::IsValidUser(BandUser* user) const {
    if (user == nullptr) {
        return false;
    }
    if (GetUser() == user) {
        return true;
    }
    if (GetUser() != nullptr) {
        return false;
    }
    return LookupUserInJoinList(user);
}

// Reconstructed from eboot.elf at 0xC0BB90.
void RBOvershellSlotCom::ClearPotentialUsers() {
    mRuntime.mPotentialUsers.clear();
}

// Reconstructed from eboot.elf at 0xC0C180.
void RBOvershellSlotCom::_UpdateSlotName() {
    Symbol name;
    RBOvershellSlotState* const state = mRuntime.mState;
    if (state != nullptr && state->HasPlayerName()) {
        name = state->GetPlayerName();
    } else if (BandUser* const user = GetUser()) {
        name = Symbol(user->GetPlatformName());
    } else {
        static Symbol joinIn;
        if (joinIn == Symbol()) {
            joinIn = Symbol("matchmaker_join_in");
        }
        static Symbol micsChanged;
        if (micsChanged == Symbol()) {
            micsChanged = Symbol("mics_changed_header");
        }
        name = Symbol();
        if (mRuntime.mState != nullptr) {
            const OvershellSlotStateID id = mRuntime.mState->GetStateID();
            if (id == kState_ConnectMic) {
                name = Symbol(Localize(micsChanged, nullptr, 0, TheLocale));
            } else if (id == kState_Join) {
                name = Symbol(Localize(joinIn, nullptr, 0, TheLocale));
            }
        }
    }
    mRuntime.mSlotName = name;
}

// Reconstructed from eboot.elf at 0xC0C360.
void RBOvershellSlotCom::_UpdateSlotLabel() {
    RBOvershellSlotState* const state = mRuntime.mState;
    mRuntime.mSlotLabel = state != nullptr && state->HasCharacter() ? state->GetCharacter() : Symbol();
}

// Reconstructed from eboot.elf at 0xC0C3B0.
void RBOvershellSlotCom::AddValidController(ControllerType type) {
    mRuntime.mValidControllers.push_back(type);
}

// Reconstructed from eboot.elf at 0xC0C4A0.
bool RBOvershellSlotCom::IsValidControllerType(ControllerType type) const {
    for (const ControllerType valid : mRuntime.mValidControllers) {
        if (valid == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xC0C4E0.
bool RBOvershellSlotCom::IsUsingOtherController() const {
    BandUser* const user = GetUser();
    return user != nullptr && static_cast<RBBandUser*>(user)->IsUsingOtherController();
}

// Reconstructed from eboot.elf at 0xC0C520.
bool RBOvershellSlotCom::IsReconnectingController() const {
    return mRuntime.mState->GetStateID() == kState_ReconnectController;
}

// Reconstructed from eboot.elf at 0xC0C540.
RBOvershellSlotState* RBOvershellSlotCom::GetState() const {
    return mRuntime.mState;
}

// Reconstructed from eboot.elf at 0xC0C550.
void RBOvershellSlotCom::ShowReconnectController() {
    BandUser* const user = GetUser();
    if (user == nullptr) {
        return;
    }
    ShowState(kState_ReconnectController);
    theRBSompSessionMgr.OnReconnectControllerShown(user);
}

// Reconstructed from eboot.elf at 0xC0C5B0. The release build keeps the
// state's name of a stripped log for a remote user; the SOMP update is
// queued twice.
void RBOvershellSlotCom::ShowState(OvershellSlotStateID state) {
    BandUser* const user = GetUser();
    if (user == nullptr) {
        return;
    }
    const bool local = static_cast<RBBandUser*>(user)->IsLocal();
    if (state != kState_MirrorRemoteUser && !local) {
        static_cast<void>(OvershellSlotStateToString(state));
    }
    user->SetOvershellSlotState(state);
    if (mRuntime.mOvershell != nullptr) {
        mRuntime.mOvershell->QueueUpdateAll();
    }
    UpdateState();
    UpdateView();
    _QueueSompUpdate();
    _QueueSompUpdate();
}

// Reconstructed from eboot.elf at 0xC0C660.
void RBOvershellSlotCom::ShowOptions() {
    BandUser* const user = GetUser();
    if (user == nullptr || mRuntime.mOvershell == nullptr) {
        return;
    }
    if (user->GetPlatformUserId() == -1) {
        user->SetOvershellSlotState(kState_ChooseProfile);
    } else if (mRuntime.mOvershell->mRuntime.mInVenueMetaMode) {
        user->SetOvershellSlotState(kState_JoinedDefault);
    } else {
        user->SetOvershellSlotState(
            mRuntime.mActiveStatus == kOvershellInSong ? kState_InSongMenu : kState_Options);
    }
    if (mRuntime.mOvershell != nullptr) {
        mRuntime.mOvershell->QueueUpdateAll();
    }
    UpdateState();
    UpdateView();
    _QueueSompUpdate();
    theRBSompSessionMgr.OnOptionsShown(user);
}

// Reconstructed from eboot.elf at 0xC0C740.
bool RBOvershellSlotCom::InOverrideFlow(OvershellOverrideFlow flow) const {
    BandUser* const user = GetUser();
    if (user == nullptr) {
        return false;
    }
    if (user->ConnectedControllerType() != user->GetControllerType() ||
        !static_cast<RBBandUser*>(user)->IsLocal()) {
        return false;
    }
    OvershellOverrideFlow current = mRuntime.mOvershell->GetOverrideFlow();
    if (current == kOverrideFlowNone) {
        current = mRuntime.mOverrideFlow;
    }
    return current == flow;
}

// Reconstructed from eboot.elf at 0xC0C7E0.
OvershellOverrideFlow RBOvershellSlotCom::GetOverrideFlow() const {
    return mRuntime.mOverrideFlow;
}

// Reconstructed from eboot.elf at 0xC0C7F0.
bool RBOvershellSlotCom::IsInVenueMetaMode() const {
    return mRuntime.mOvershell->mRuntime.mInVenueMetaMode;
}

// Reconstructed from eboot.elf at 0xC0C800.
bool RBOvershellSlotCom::IsShowingSharedProfileWarning() const {
    BandUser* const user = GetUser();
    return user != nullptr && user->GetOvershellSlotState() == kState_SharedProfileScoresDontCountWarn;
}

// Reconstructed from eboot.elf at 0xC0C850.
void RBOvershellSlotCom::ForceDropInFlow() {
    if (GetUser()->mPlayer == nullptr) {
        _StartDropInFlow();
    }
}

// Reconstructed from eboot.elf at 0xC0C8A0.
void RBOvershellSlotCom::_StartDropInFlow() {
    BandUser* const user = GetUser();
    user->SetTrackType(user->GetControllerType() == kControllerVocals ? kTrackPendingVocals : kTrackPending);
    mRuntime.mPartChosen = true;
    BeginOverrideFlow(kOverrideFlowSongSettings, true);
}

// Reconstructed from eboot.elf at 0xC0CE40. The drum part's score type is
// read for a stripped log.
void RBOvershellSlotCom::_ShowChoosePartWait() {
    BandUser* const user = GetUser();
    if (user->GetTrackType() == kTrackDrum) {
        static_cast<void>(user->GetPreferredScoreType());
    }
    user->SetOvershellSlotState(kState_ChoosePartWait);
    static Symbol disableUnselectedParts;
    if (disableUnselectedParts == Symbol()) {
        disableUnselectedParts = Symbol("disable_unselected_parts");
    }
    Message<1> msg(disableUnselectedParts);
    msg.mData->Node(2) = ObjectNode(this);
    static_cast<void>(mRuntime.mListProvider->Handle(msg.mData, true));
    if (mRuntime.mOvershell != nullptr) {
        mRuntime.mOvershell->QueueUpdateAll();
    }
    UpdateState();
    UpdateView();
    _QueueSompUpdate();
}

// Reconstructed from eboot.elf at 0xC0D0A0.
void RBOvershellSlotCom::LeaveOptions(bool unpause) {
    _ClearOverrideFlow();
    ShowState(kState_JoinedDefault);
    if (unpause && mRuntime.mActiveStatus == kOvershellInSong && TheTimeMgr->mClock.IsPaused(kTimeUnitsSeconds)) {
        mRuntime.mOvershell->CheckAndAutoUnpauseGame();
    }
}

// Reconstructed from eboot.elf at 0xC0D120.
void RBOvershellSlotCom::_ClearOverrideFlow() {
    if (mRuntime.mOverrideFlow != kOverrideFlowNone) {
        mRuntime.mOverrideFlow = kOverrideFlowNone;
        mRuntime.mOverrideFlowReturnState = kState_JoinedDefault;
    }
}

// Reconstructed from eboot.elf at 0xC0D550.
void RBOvershellSlotCom::ShowInSongMenu() {
    ShowState(kState_InSongMenu);
    if (mRuntime.mPreviousState->GetStateID() == kState_JoinedDefault &&
        !mRuntime.mOvershell->CheckAndAutoPauseGame()) {
        ShowState(kState_JoinedDefault);
    }
}

// Reconstructed from eboot.elf at 0xC0D5A0.
void RBOvershellSlotCom::ShowBandMemberSelect() {
    ShowState(kState_ChooseBandMember);
}

// Reconstructed from eboot.elf at 0xC0D5B0.
void RBOvershellSlotCom::ShowIdle() {
    ShowState(kState_Idle);
}

// Reconstructed from eboot.elf at 0xC0D5C0.
void RBOvershellSlotCom::ShowDisabled() {
    ShowState(kState_Disabled);
}

// Reconstructed from eboot.elf at 0xC0D5D0.
void RBOvershellSlotCom::ShowScreenTransition() {
    ShowState(kState_OnlineScreenTransition);
}

// Reconstructed from eboot.elf at 0xC0D5E0.
void RBOvershellSlotCom::RefreshInstrumentOptions() {
    static Symbol refreshInstrumentOptions;
    if (refreshInstrumentOptions == Symbol()) {
        refreshInstrumentOptions = Symbol("refresh_instrument_options");
    }
    Message<1> msg(refreshInstrumentOptions);
    msg.mData->Node(2) = ObjectNode(this);
    static_cast<void>(mRuntime.mListProvider->Handle(msg.mData, true));
    if (IsInstrumentOptionsState(mRuntime.mState->GetStateID())) {
        _ShowInstrumentOptions();
    }
}

// Reconstructed from eboot.elf at 0xC0D800. The guitar slots share the
// guitar options.
void RBOvershellSlotCom::_ShowInstrumentOptions() {
    switch (mRuntime.mBandSlot) {
    case kGameSlotGuitar0:
    case kGameSlotGuitar1:
        ShowState(kState_OptionsGuitar);
        break;
    case kGameSlotDrum:
        ShowState(kState_OptionsDrum);
        break;
    case kGameSlotVocals:
        ShowState(kState_OptionsVocal);
        break;
    default:
        LeaveOptions(true);
        break;
    }
}

// Reconstructed from eboot.elf at 0xC0D8B0. Changing from another flow
// leaves only the release build's error name.
void RBOvershellSlotCom::BeginOverrideFlow(OvershellOverrideFlow flow, bool update) {
    if (mRuntime.mOverrideFlow == flow) {
        return;
    }
    if (mRuntime.mOverrideFlow != kOverrideFlowNone) {
        static_cast<void>(MakeErrorName());
        _ClearOverrideFlow();
    }
    _SetOverrideType(flow, update);
}

// Reconstructed from eboot.elf at 0xC0D920. Any flow ends; ending the song
// settings flow forgets the chosen part.
void RBOvershellSlotCom::EndOverrideFlow(OvershellOverrideFlow flow, bool update) {
    if (mRuntime.mOverrideFlow == kOverrideFlowNone) {
        return;
    }
    if (flow == kOverrideFlowSongSettings) {
        mRuntime.mPartChosen = false;
    }
    _SetOverrideType(kOverrideFlowNone, update);
}

// Reconstructed from eboot.elf at 0xC0D940.
void RBOvershellSlotCom::SetOverrideFlowReturnState(OvershellSlotStateID state) {
    mRuntime.mOverrideFlowReturnState = state;
}

// Reconstructed from eboot.elf at 0xC0D950.
void RBOvershellSlotCom::RevertToOverrideFlowReturnState() {
    GetUser()->SetOvershellSlotState(mRuntime.mOverrideFlowReturnState);
    mRuntime.mOverrideFlowReturnState = kState_JoinedDefault;
    if (mRuntime.mOvershell != nullptr) {
        mRuntime.mOvershell->QueueUpdateAll();
    }
    UpdateState();
    UpdateView();
    _QueueSompUpdate();
}

// Reconstructed from eboot.elf at 0xC0D9D0. The release build keeps the
// call of a stripped check.
void RBOvershellSlotCom::_SetupCustomStateList() {
    RBOvershellSlotState* const state = mRuntime.mState;
    static_cast<void>(state->HasListData());
    state->UpdateText(mRuntime.mConfirmActionTextCom);
    static Symbol setup;
    if (setup == Symbol()) {
        setup = Symbol("setup_custom_state_list");
    }
    SendToListProvider(this, setup);
}

// Reconstructed from eboot.elf at 0xC0DBE0.
void RBOvershellSlotCom::_SetupPartSelectList() {
    static Symbol setup;
    if (setup == Symbol()) {
        setup = Symbol("setup_part_select_list");
    }
    SendToListProvider(this, setup);
}

// Reconstructed from eboot.elf at 0xC0DDD0. In a SOMP session's song
// select, green room and song search, a slot without a chosen part shows
// its slot's alternate part.
TrackType RBOvershellSlotCom::GetDisplayTrackType() const {
    RBOvershellSlotState* const state = mRuntime.mState;
    if (state != nullptr && state->HasTrackType()) {
        return state->GetTrackType();
    }
    if (mRuntime.mPreviewTrackType != kTrackNone) {
        return mRuntime.mPreviewTrackType;
    }
    if (IsSomp()) {
        const RBUILayout layout = theRBMetaState.mCurrentLayout;
        if (layout == kLayoutSongSelect || layout == kLayoutSompGreenRoom || layout == kLayoutSongSelectSearch) {
            return GameSlotToTrackType(static_cast<GameSlot>(mRuntime.mBandSlot), true);
        }
    }
    BandUser* const user = GetUser();
    if (user == nullptr || user->GetTrackType() == kTrackNone ||
        static_cast<int>(user->GetTrackType()) == kTrackPending ||
        static_cast<int>(user->GetTrackType()) == kTrackPendingVocals) {
        return GameSlotToTrackType(static_cast<GameSlot>(mRuntime.mBandSlot), false);
    }
    return user->GetTrackType();
}

// Reconstructed from eboot.elf at 0xC0DEC0.
unsigned int RBOvershellSlotCom::GetNumHarmonyMicsToDisplay() const {
    RBOvershellSlotState* const state = mRuntime.mState;
    if (state != nullptr && state->HasNumHarmonyMics()) {
        return static_cast<unsigned int>(state->GetNumHarmonyMics());
    }
    return static_cast<unsigned int>(mRuntime.mNumHarmonyMicsToDisplay);
}

// Reconstructed from eboot.elf at 0xC0DF00.
bool RBOvershellSlotCom::GetVocalFreestyleIconOverride() const {
    RBOvershellSlotState* const state = mRuntime.mState;
    if (state != nullptr && state->HasVocalFreestyle()) {
        return state->GetVocalFreestyle();
    }
    return mRuntime.mVocalFreestyleIconOverride;
}

// Reconstructed from eboot.elf at 0xC0DF40.
bool RBOvershellSlotCom::GetProDrumsIconOverride() const {
    RBOvershellSlotState* const state = mRuntime.mState;
    if (state != nullptr && state->HasProDrums()) {
        return state->GetProDrums();
    }
    return mRuntime.mProDrumsIconOverride;
}

// Reconstructed from eboot.elf at 0xC0DF80.
Symbol RBOvershellSlotCom::_GetJoinToken() {
    static Symbol sompSwitch;
    if (sompSwitch == Symbol()) {
        sompSwitch = Symbol("somp_switch");
    }
    static Symbol joinIn;
    if (joinIn == Symbol()) {
        joinIn = Symbol("matchmaker_join_in");
    }
    return theRBSompSessionMgr.IsInSompMode() ? sompSwitch : joinIn;
}

// Reconstructed from eboot.elf at 0xC0E090. A SOMP session's message
// replaces the token; the online overshell waits for a remote player in the
// player stats, the green room and the invitations.
Symbol RBOvershellSlotCom::_GetEmptyToken() const {
    if (IsSomp() && mRuntime.mShowingMessage) {
        return mRuntime.mMessageToken;
    }
    static Symbol waiting;
    if (waiting == Symbol()) {
        waiting = Symbol("overshell_somp_connection_waiting");
    }
    static Symbol noPlayer;
    if (noPlayer == Symbol()) {
        noPlayer = Symbol("overshell_somp_connection_no_player");
    }
    static Symbol connectController;
    if (connectController == Symbol()) {
        connectController = Symbol("connect_controller");
    }
    if (!IsFullOvershellOnline()) {
        return connectController;
    }
    const RBUILayout layout = theRBMetaState.mCurrentLayout;
    if (layout == kLayoutPlayerStats || layout == kLayoutSompGreenRoom || layout == kLayoutSompInviteFriends) {
        return waiting;
    }
    return noPlayer;
}

// Reconstructed from eboot.elf at 0xC0E280.
bool RBOvershellSlotCom::IsFullOvershellOnline() const {
    const RBOvershellCom* const overshell = mRuntime.mOvershell;
    if (overshell == nullptr || !IsSomp() || overshell->IsPendingOnline()) {
        return false;
    }
    return overshell->IsPendingOffline() || overshell->IsOnline();
}

// Reconstructed from eboot.elf at 0xC0E2E0.
Symbol RBOvershellSlotCom::GetMessageToken() const {
    if (IsFullOvershellOnline() && mRuntime.mShowingMessage) {
        return mRuntime.mMessageToken;
    }
    return Symbol();
}

// Reconstructed from eboot.elf at 0xC0E350.
Symbol RBOvershellSlotCom::_GetListView() const {
    static Symbol listShort;
    if (listShort == Symbol()) {
        listShort = Symbol("list_short");
    }
    static Symbol listMedium;
    if (listMedium == Symbol()) {
        listMedium = Symbol("list_medium");
    }
    static Symbol listLong;
    if (listLong == Symbol()) {
        listLong = Symbol("list_long");
    }
    if (mRuntime.mUIList == nullptr) {
        return listLong;
    }
    const unsigned long rows = mRuntime.mUIList->_GetDataRows();
    if (rows > 5) {
        return listLong;
    }
    return rows > 3 ? listMedium : listShort;
}

// Reconstructed from eboot.elf at 0xC0E4E0.
Symbol RBOvershellSlotCom::_GetConfirmActionView() const {
    static Symbol confirmAction;
    if (confirmAction == Symbol()) {
        confirmAction = Symbol("confirm_action");
    }
    static Symbol confirmActionLong;
    if (confirmActionLong == Symbol()) {
        confirmActionLong = Symbol("confirm_action_long");
    }
    if (mRuntime.mUIList == nullptr) {
        return confirmActionLong;
    }
    return mRuntime.mUIList->_GetDataRows() > 3 ? confirmActionLong : confirmAction;
}

// Reconstructed from eboot.elf at 0xC0E600.
bool RBOvershellSlotCom::_IsListView(const Symbol& view) const {
    static Symbol listShort;
    if (listShort == Symbol()) {
        listShort = Symbol("list_short");
    }
    static Symbol listMedium;
    if (listMedium == Symbol()) {
        listMedium = Symbol("list_medium");
    }
    static Symbol listLong;
    if (listLong == Symbol()) {
        listLong = Symbol("list_long");
    }
    static Symbol confirmAction;
    if (confirmAction == Symbol()) {
        confirmAction = Symbol("confirm_action");
    }
    static Symbol confirmActionLong;
    if (confirmActionLong == Symbol()) {
        confirmActionLong = Symbol("confirm_action_long");
    }
    static Symbol disconnectedMessage;
    if (disconnectedMessage == Symbol()) {
        disconnectedMessage = Symbol("somp_disconnected_message");
    }
    return view == listShort || view == listMedium || view == listLong || view == confirmAction ||
        view == confirmActionLong || view == disconnectedMessage;
}

// Reconstructed from eboot.elf at 0xC0E890.
bool RBOvershellSlotCom::_IsConfirmActionView(const Symbol& view) const {
    static Symbol confirmAction;
    if (confirmAction == Symbol()) {
        confirmAction = Symbol("confirm_action");
    }
    static Symbol confirmActionLong;
    if (confirmActionLong == Symbol()) {
        confirmActionLong = Symbol("confirm_action_long");
    }
    static Symbol disconnectedMessage;
    if (disconnectedMessage == Symbol()) {
        disconnectedMessage = Symbol("somp_disconnected_message");
    }
    return view == confirmAction || view == confirmActionLong || view == disconnectedMessage;
}

// Reconstructed from eboot.elf at 0xC0EA00. A user on the profile swap
// confirmation whose swap partner signed out or lost the controller goes
// back to the profile choice; outside the SOMP part choice the preview part
// is forgotten.
void RBOvershellSlotCom::UpdateState() {
    BandUser* const user = GetUser();
    RBOvershellSlotState* const state = _GenerateCurrentState();
    if (mRuntime.mState == nullptr || state->GetStateID() != mRuntime.mState->GetStateID()) {
        if (mRuntime.mState != nullptr) {
            static Symbol exit;
            if (exit == Symbol()) {
                exit = Symbol("exit");
            }
            SendToState(mRuntime.mState, exit);
        }
        mRuntime.mPreviousState = mRuntime.mState;
        mRuntime.mState = state;
        static Symbol enter;
        if (enter == Symbol()) {
            enter = Symbol("enter");
        }
        SendToState(mRuntime.mState, enter);
        _QueueSompUpdate();
    }
    if (user == nullptr) {
        return;
    }
    if (mRuntime.mState->GetStateID() == kState_ChooseProfileConfirm) {
        static Symbol swapUser;
        if (swapUser == Symbol()) {
            swapUser = Symbol("swap_user");
        }
        const User* const swap =
            static_cast<User*>(static_cast<MsgSink*>(mRuntime.mState->GetProperty(swapUser).LiteralSink(nullptr)));
        if (!swap->IsSignedIn() || !swap->IsJoypadConnected()) {
            ShowState(kState_ChooseProfile);
        }
    }
    if (!IsSomp() ||
        (mRuntime.mState->GetStateID() != kState_ChoosePart && mRuntime.mState->GetStateID() != kState_ChoosePartWait)) {
        mRuntime.mPreviewTrackType = kTrackNone;
    }
    _UpdateListInput();
}

// Reconstructed from eboot.elf at 0xC0EE30.
RBOvershellSlotState* RBOvershellSlotCom::_GenerateCurrentState() const {
    OvershellSlotStateID id;
    BandUser* const user = GetUser();
    if (user != nullptr) {
        id = user->GetOvershellSlotState();
    } else if (mRuntime.mOvershell != nullptr && mRuntime.mOvershell->IsSlotOffline(mRuntime.mBandSlot)) {
        id = kState_MirrorRemoteUser;
    } else if (!mRuntime.mPotentialUsers.empty()) {
        id = mRuntime.mOvershell != nullptr && mRuntime.mOvershell->IsFull() ? kStateJoinBlocked : kState_Join;
    } else {
        id = kState_NoInstrument;
    }
    return mRuntime.mStateMgr->GetSlotState(id);
}

// Reconstructed from eboot.elf at 0xC0EED0.
void RBOvershellSlotCom::_SetPreviewTrackType(TrackType type) {
    mRuntime.mPreviewTrackType = type;
}

// Reconstructed from eboot.elf at 0xC0EEE0.
void RBOvershellSlotCom::_UpdateListInput() {
    if (mRuntime.mInputReceiver != nullptr) {
        _UpdateInputReceiverState();
    }
    if (mRuntime.mUIList != nullptr) {
        _UpdateListScrolling();
        _UpdateListSelection();
    }
}

// Reconstructed from eboot.elf at 0xC0EF90. The release build keeps the
// calls of stripped checks.
void RBOvershellSlotCom::_UpdateInputReceiverState() {
    static_cast<void>(GetUser());
    RBOvershellSlotState* const state = mRuntime.mState;
    if (state->AllowsUIListInput() || state->ForceUIListFocus()) {
        static_cast<void>(state->ForceUIListFocus());
        mRuntime.mInputReceiver->QueueState(kUIStateFocused, false);
        if (mRuntime.mInputCom != nullptr && !state->RetractedPosition()) {
            _QueueFocus(mRuntime.mInputReceiver);
        } else {
            _QueueFocus(nullptr);
        }
    } else {
        mRuntime.mInputReceiver->QueueState(kUIStateDisabled, false);
        if (mRuntime.mInputCom != nullptr && !state->RetractedPosition()) {
            _QueueFocus(nullptr);
        }
    }
}

// Reconstructed from eboot.elf at 0xC0F060. The release build keeps the
// call of a stripped check.
void RBOvershellSlotCom::_UpdateListScrolling() {
    static_cast<void>(GetUser());
    mRuntime.mUIList->mRuntime.mOnlyAppScrolling = !mRuntime.mState->AllowsUIListScrolling();
}

// Reconstructed from eboot.elf at 0xC0F0B0. The release build keeps the
// call of a stripped check.
void RBOvershellSlotCom::_UpdateListSelection() {
    static_cast<void>(GetUser());
    mRuntime.mUIList->SetCanSelectOverride(true, mRuntime.mState->AllowsUIListSelection());
}

// Reconstructed from eboot.elf at 0xC0F100.
void RBOvershellSlotCom::_QueueFocus(UIInputReceiverCom* receiver) {
    mRuntime.mPendingFocus = true;
    mRuntime.mPendingFocusReceiver = receiver;
}

// Reconstructed from eboot.elf at 0xC0F110. A new view sets the blenders'
// transition time from the old view; the slot's name, label (the state's
// character, when it gives one) and microphone monitors follow. The
// release build keeps the calls of stripped controller checks.
void RBOvershellSlotCom::UpdateView() {
    const Symbol view = mRuntime.mState != nullptr ? mRuntime.mState->GetView() : Symbol();
    Symbol newView = view;
    BandUser* const user = GetUser();
    if (newView != Symbol() && newView != mRuntime.mCurrentView) {
        const float time = mRuntime.mStateMgr->GetTransitionTime(mRuntime.mCurrentView, newView);
        for (unsigned int index = 0; index < mRuntime.mStateBlenderComs.size(); ++index) {
            mRuntime.mStateBlenderComs[index]->mDuration = time;
        }
        _SetView(newView);
    }
    if (!mRuntime.mValidControllers.empty() && mRuntime.mValidControllers[0] != kControllerNone) {
        if (user != nullptr && user->mParticipating) {
            static_cast<void>(user->GetControllerType());
        } else if (!mRuntime.mPotentialUsers.empty()) {
            static_cast<void>(mRuntime.mPotentialUsers[0]->ConnectedControllerType());
        }
    }
    mRuntime.mMicMonitorsShowing = user != nullptr && mRuntime.mActiveStatus == kOvershellInMeta &&
        user->GetSlot() == kGameSlotVocals && mRuntime.mState->ShowsExtendedMicArrows();
    if (mRuntime.mState != nullptr && newView == view) {
        mRuntime.mState->UpdateView();
    }
    _UpdateSlotName();
    RBOvershellSlotState* const state = mRuntime.mState;
    if (state != nullptr && state->HasCharacter()) {
        mRuntime.mSlotLabel = state->GetCharacter();
    } else {
        mRuntime.mSlotLabel = Symbol();
    }
    _QueueSompUpdate();
}

// Reconstructed from eboot.elf at 0xC0F380.
void RBOvershellSlotCom::_SetView(Symbol view) {
    mRuntime.mCurrentView = view;
    if (GetUser() != nullptr && mRuntime.mInputCom != nullptr) {
        RBOvershellSlotState* const state = mRuntime.mState;
        _QueueFocus(
            state != nullptr && !state->RetractedPosition() && state->AllowsUIListInput() ? mRuntime.mInputReceiver
                                                                                          : nullptr);
    }
}

// Reconstructed from eboot.elf at 0xC0F400.
void RBOvershellSlotCom::Update() {
    UpdateState();
    UpdateView();
}

// Reconstructed from eboot.elf at 0xC0F420.
bool RBOvershellSlotCom::_OnResourcesLoaded() {
    mRuntime.mStateMgr->Init(this);
    return true;
}

// Reconstructed from eboot.elf at 0xC0F440. The list object's components
// are found once; its input receiver takes no pad in particular. A failed
// property set leaves only the release build's error name and path.
void RBOvershellSlotCom::_Enter() {
    static Symbol listFocus;
    if (listFocus == Symbol()) {
        listFocus = Symbol("list_focus");
    }
    // Component begins with MsgSink's slots; the model does not derive it.
    mObject->mEntity->AddSink(&mRuntime.mListFocusSink, reinterpret_cast<MsgSink*>(this), listFocus, Symbol());
    Entity* const entity = mObject->mEntity;
    if (mRuntime.mListProvider == nullptr) {
        const GameObject* const list = entity->GetObject(mList);
        mRuntime.mUIList = list->GetCom<UIListCom>();
        mRuntime.mUIList->RegisterForSelectionHandling(this);
        mRuntime.mListProvider = list->GetCom<RBOvershellListProviderCom>();
        mRuntime.mListProvider->mSlotCom = this;
        mRuntime.mInputReceiver = list->GetBaseCom<UIInputReceiverCom>();
        static Symbol lockedToPadNum;
        if (lockedToPadNum == Symbol()) {
            lockedToPadNum = Symbol("locked_to_pad_num");
        }
        UIInputReceiverCom* const receiver = mRuntime.mInputReceiver;
        PropPath path;
        PropPath::Node node;
        path.PushNode(node = lockedToPadNum);
        int value = kNoLockedPad;
        void* storage = receiver;
        const PropInfo* const info = receiver->_GetPropRegistry().FindProp(path, &storage);
        if (info == nullptr) {
            static_cast<void>(receiver->MakeErrorName());
            char text[512] = {};
            path.ToString(text, sizeof(text));
        } else if (!_SetProp(*receiver, path, *info, storage, &value, kPropertyInt, false)) {
            static_cast<void>(receiver->MakeErrorName());
            char text[512] = {};
            path.ToString(text, sizeof(text));
        }
    }
    if (mRuntime.mConfirmActionTextCom == nullptr) {
        mRuntime.mConfirmActionTextCom = entity->GetObject(mConfirmActionText)->GetCom<RndTextCom>();
    }
    mRuntime.mDrawNode = mObject->GetCom<RndDrawNodeCom>();
    if (mRuntime.mStateBlenderComs.empty()) {
        mRuntime.mStateBlenderComs.reserve(mStateBlenders.mSize);
        for (unsigned int index = 0; index < mStateBlenders.mSize; ++index) {
            mRuntime.mStateBlenderComs.push_back(entity->GetObject(mStateBlenders[index])->GetCom<PropertyBlenderCom>());
        }
    }
}

// Reconstructed from eboot.elf at 0xC0FC30. In the meta screens the art
// shows a local user's held actions; the list hides with the slot's draw
// node or a retracted state; the microphone monitors show the energy of
// the three microphones. The release build keeps the call of a stripped
// microphone check.
void RBOvershellSlotCom::_Poll() {
    if (mRuntime.mActiveStatus == kOvershellInMeta) {
        BandUser* const user = GetUser();
        if (user != nullptr) {
            if (static_cast<RBBandUser*>(user)->IsLocal()) {
                const JoypadData* const pad = JoypadGetPadData(user->GetPadNum());
                for (unsigned int button = 0; button < 26; ++button) {
                    const JoypadAction action = ButtonToAction(static_cast<JoypadButton>(button), pad->mControllerType);
                    mRuntime.mButtonDown[action] = (pad->mButtons >> button) & 1;
                }
            } else {
                mRuntime.mButtonDown[1] = false;
                mRuntime.mButtonDown[2] = false;
                mRuntime.mButtonDown[3] = false;
                mRuntime.mButtonDown[14] = false;
                mRuntime.mButtonDown[11] = false;
            }
        }
    }
    const bool drawHidden = (mRuntime.mDrawNode->mRuntime.mWorldShowHideFlags & 1) != 0;
    const bool showing = mRuntime.mState != nullptr && !mRuntime.mState->RetractedPosition();
    mRuntime.mUIList->SetHidden(drawHidden || !showing);
    if (mRuntime.mMicMonitorsShowing) {
        for (int index = 0; index < 3; ++index) {
            const MicClientID client = {index, -1};
            GameMic* const mic = TheGameMicManager->GetMic(client);
            if (mic != nullptr) {
                mic->GetMyMic()->Start();
                mic->Update();
                const float energy = mic->mEnergy;
                mRuntime.mMicEnergy[index] = energy > 1.0F ? 1.0F : (energy > 0.0F ? energy : 0.0F);
            }
        }
    }
    _PollDropIn();
    _PollRemoteUserState();
    RBOvershellSlotState* const state = mRuntime.mState;
    if (state != nullptr && state->IsRecordNew()) {
        UpdateState();
        UpdateView();
        mRuntime.mState->ClearRecordNew();
        _QueueSompUpdate();
    }
    _PollConnectionState();
    _PollMessages();
    _PollServerWait();
    _PollSompUpdate();
    _QueuePostPoll();
}

// Reconstructed from eboot.elf at 0xC0FF60.
void RBOvershellSlotCom::_PollDropIn() {
    BandUser* const user = GetUser();
    if (user == nullptr || !static_cast<RBBandUser*>(user)->IsLocal()) {
        return;
    }
    if (mRuntime.mActiveStatus != kOvershellInSong || !mRuntime.mOvershell->mRuntime.mSongOptionsRequired) {
        return;
    }
    if (mRuntime.mOverrideFlow != kOverrideFlowNone || !mRuntime.mOvershell->_CanJoinUsers()) {
        return;
    }
    if (user->GetTrackType() == kTrackNone || !user->IsFullyInGame()) {
        _StartDropInFlow();
    }
}

// Reconstructed from eboot.elf at 0xC10000. The release build keeps the
// call of a stripped log.
void RBOvershellSlotCom::_PollRemoteUserState() {
    BandUser* const user = GetUser();
    if (user == nullptr) {
        return;
    }
    const OvershellSlotStateID state = mRuntime.mState->GetStateID();
    if (static_cast<RBBandUser*>(user)->IsLocal()) {
        if (state == kState_MirrorRemoteUser) {
            LeaveOptions(true);
        }
        return;
    }
    if (mRuntime.mOvershell == nullptr || !mRuntime.mOvershell->_CanJoinUsers()) {
        return;
    }
    if (mRuntime.mOverrideFlow != kOverrideFlowNone) {
        EndOverrideFlow(mRuntime.mOverrideFlow, false);
    } else if (state != kState_MirrorRemoteUser) {
        static_cast<void>(OvershellSlotStateToString(state));
        ShowState(kState_MirrorRemoteUser);
    }
}

// Reconstructed from eboot.elf at 0xC10190.
void RBOvershellSlotCom::_PollConnectionState() {
    if (!IsSomp()) {
        return;
    }
    const OvershellSlotConnectionState connection = GetConnectionState();
    if (connection != mRuntime.mConnectionState) {
        mRuntime.mConnectionState = connection;
        UpdateState();
        UpdateView();
        _QueueSompUpdate();
    }
}

// Reconstructed from eboot.elf at 0xC101F0. A change refreshes the slot
// and the track layout.
void RBOvershellSlotCom::_PollMessages() {
    _ShowNextMessage();
    if (mRuntime.mMessageChanged) {
        mRuntime.mMessageChanged = false;
    } else if (mRuntime.mShowingMessage && RealTimeMs() > mRuntime.mMessageEndMs) {
        mRuntime.mShowingMessage = false;
        mRuntime.mMessageEndMs = -1.0F;
        mRuntime.mMessageToken = Symbol();
        _ShowNextMessage();
    } else {
        return;
    }
    UpdateState();
    UpdateView();
    theRBTrackLayoutData.SetNeedsRefresh();
    _QueueSompUpdate();
}

// Reconstructed from eboot.elf at 0xC102E0. The release build keeps the
// calls of a stripped log.
void RBOvershellSlotCom::_PollServerWait() {
    RBOvershellSlotState* const state = mRuntime.mState;
    if (state == nullptr || !state->ServerWait()) {
        mRuntime.mServerWaitStartMs = -1.0F;
        return;
    }
    if (!IsSomp()) {
        static_cast<void>(state->GetStateID());
        static_cast<void>(OvershellSlotStateToString(state->GetStateID()));
        LeaveOptions(true);
        return;
    }
    if (mRuntime.mServerWaitStartMs <= 0.0F) {
        mRuntime.mServerWaitStartMs = RealTimeMs();
        return;
    }
    const float elapsed = RealTimeMs() - mRuntime.mServerWaitStartMs;
    if (!(elapsed >= 0.0F)) {
        mRuntime.mServerWaitStartMs = -1.0F;
        return;
    }
    if (elapsed < kServerWaitTimeoutMs) {
        return;
    }
    static Symbol booted;
    if (booted == Symbol()) {
        booted = Symbol("somp_error_booted_from_server");
    }
    theRBSompSessionMgr.LeaveSessionWithError(booted);
    LeaveOptions(true);
}

// Reconstructed from eboot.elf at 0xC108E0.
void RBOvershellSlotCom::_QueuePostPoll() {
    if (mRuntime.mPendingFocus) {
        mObject->mEntity->QueueForPostPoll(this);
    }
}

// Reconstructed from eboot.elf at 0xC10900.
void RBOvershellSlotCom::_PostPoll() {
    if (mRuntime.mPendingFocus) {
        _ApplyPendingFocus();
        mRuntime.mPendingFocus = false;
        mRuntime.mPendingFocusReceiver = nullptr;
    }
}

// Reconstructed from eboot.elf at 0xC10940.
void RBOvershellSlotCom::_ApplyPendingFocus() {
    if (mRuntime.mInputCom == nullptr || GetUser() == nullptr) {
        return;
    }
    const int padNum = GetUser()->GetPadNum();
    mRuntime.mInputCom->ClearFocus(padNum);
    if (mRuntime.mPendingFocusReceiver != nullptr) {
        mRuntime.mInputCom->SetFocusedObject(mRuntime.mPendingFocusReceiver, padNum, false);
    }
}

// Reconstructed from eboot.elf at 0xC109D0.
void RBOvershellSlotCom::_SetDefaultPart() {
    BandUser* const user = GetUser();
    const RBProfile* const profile = user->GetProfile(false);
    TrackType track;
    ScoreType score;
    switch (mRuntime.mBandSlot) {
    case kGameSlotGuitar0:
    case kGameSlotGuitar1: {
        track = profile != nullptr && profile->GetMoreScoredType(kScoreGuitar, kScoreBass) == kScoreBass ? kTrackBass
                                                                                                           : kTrackGuitar;
        const BandUser* const partner = theBandUserMgr->GetUserFromSlot(
            mRuntime.mBandSlot == kGameSlotGuitar0 ? kGameSlotGuitar1 : kGameSlotGuitar0);
        if (partner != nullptr && partner->GetTrackType() == track) {
            track = partner->GetTrackType() == kTrackGuitar ? kTrackBass : kTrackGuitar;
        }
        score = TrackTypeToScoreType(track, false, false);
        break;
    }
    case kGameSlotDrum:
        score = profile != nullptr ? profile->GetMoreScoredType(kScoreDrum, kScoreRealDrum) : kScoreDrum;
        _UpdateProDrumsIcon(score);
        track = kTrackDrum;
        break;
    case kGameSlotVocals:
        score = profile != nullptr ? profile->GetMoreScoredType(kScoreVocals, kScoreHarmony) : kScoreVocals;
        _UpdateVocalPartIcon(score);
        track = kTrackVocals;
        break;
    default:
        return;
    }
    user->SetTrackType(track);
    user->SetPreferredScoreType(score);
}

// Reconstructed from eboot.elf at 0xC10B40.
void RBOvershellSlotCom::_ChooseProfile(int platformUserId) {
    AddProfileToSlot(platformUserId);
    LeaveOptions(true);
}

// Reconstructed from eboot.elf at 0xC10BC0.
void RBOvershellSlotCom::_ChangeProfile() {
    BandUser* const user = GetUser();
    user->SetOvershellSlotState(user->IsSignedIn() ? kState_ChooseProfileDenial : kState_ChooseProfile);
    _RefreshOvershell(false);
}

// Reconstructed from eboot.elf at 0xC10C40.
void RBOvershellSlotCom::_CancelChangeProfile() {
    BandUser* const user = GetUser();
    if (mRuntime.mPreviousState != nullptr && mRuntime.mPreviousState->GetStateID() == kState_Options) {
        user->SetOvershellSlotState(kState_Options);
        _ExportCancelMsg();
    } else {
        AttemptRemoveUser();
    }
    _RefreshOvershell(false);
}

// Reconstructed from eboot.elf at 0xC10D20.
void RBOvershellSlotCom::_ExportCancelMsg() {
    const Message<0> msg(gOnCancel);
    static_cast<void>(mObject->mEntity->Export(msg.mData));
}

// Reconstructed from eboot.elf at 0xC10D90.
bool RBOvershellSlotCom::IsRemoteSlot() const {
    return mRuntime.mOvershell != nullptr && mRuntime.mOvershell->IsSlotOffline(mRuntime.mBandSlot);
}

// Reconstructed from eboot.elf at 0xC10DB0.
void RBOvershellSlotCom::_SetTooltipShowing(bool showing) {
    mRuntime.mTooltipShowing = showing;
}

// Reconstructed from eboot.elf at 0xC10DC0.
void RBOvershellSlotCom::_UpdateVocalPartIcon(ScoreType type) {
    if (type == kScoreHarmony) {
        mRuntime.mVocalFreestyleIconOverride = false;
        const eastl::vector<Symbol> songs = theGameData.GetSongsToLoadMetaSafe();
        mRuntime.mNumHarmonyMicsToDisplay =
            songs.empty() ? 3 : theSongMgr->GetMetadata(songs[0], true)->NumVocalParts();
    } else if (type == kScoreVocals) {
        mRuntime.mNumHarmonyMicsToDisplay = 0;
        mRuntime.mVocalFreestyleIconOverride = _ShouldShowVocalFreestyleIcon(GetUser()->GetDifficulty());
    }
}

// Reconstructed from eboot.elf at 0xC10EB0.
void RBOvershellSlotCom::_UpdateVocalFreestyleIcon(Difficulty difficulty) {
    mRuntime.mVocalFreestyleIconOverride = _ShouldShowVocalFreestyleIcon(difficulty);
}

// Reconstructed from eboot.elf at 0xC11050.
void RBOvershellSlotCom::_UpdateProDrumsIcon(ScoreType type) {
    mRuntime.mProDrumsIconOverride = type == kScoreRealDrum;
}

// Reconstructed from eboot.elf at 0xC11060.
void RBOvershellSlotCom::_SelectPart(TrackType type) {
    _SelectPartImpl(type, false, false, kState_ChoosePartWait);
    _RefreshOvershell(false);
}

// Reconstructed from eboot.elf at 0xC110B0.
void RBOvershellSlotCom::_SelectPartWithOptions(TrackType type, bool alternate, bool spotlight) {
    _SelectPartImpl(type, alternate, spotlight, kState_ChoosePartWait);
    _RefreshOvershell(false);
}

// Reconstructed from eboot.elf at 0xC11100.
void RBOvershellSlotCom::_SelectVocalPart(bool harmony) {
    _SelectPartImpl(kTrackVocals, harmony, false, kState_ChoosePartWait);
    _RefreshOvershell(false);
}

// Reconstructed from eboot.elf at 0xC11150.
void RBOvershellSlotCom::_SelectDrumPart(bool proDrums) {
    _SelectPartImpl(kTrackDrum, false, proDrums, kState_ChoosePartWait);
    _RefreshOvershell(false);
}

// Reconstructed from eboot.elf at 0xC111A0.
Symbol RBOvershellSlotCom::GetCurrentOvershellQuitToken() {
    return theGameData.GetCurrentOvershellQuitToken();
}

// Reconstructed from eboot.elf at 0xC111B0.
Symbol RBOvershellSlotCom::GetCurrentOvershellSkipToken() {
    return theGameData.GetCurrentOvershellSkipToken();
}

// Reconstructed from eboot.elf at 0xC111C0.
bool RBOvershellSlotCom::_ShouldShowRestartPrompt() {
    return theGameData.GetGameMode() != kGameModePractice;
}

// Reconstructed from eboot.elf at 0xC111E0.
bool RBOvershellSlotCom::_AllowContinueAfterDifficultyChange() {
    return theGameData.GetGameMode() != kGameModeBTM;
}

// Reconstructed from eboot.elf at 0xC11200.
Symbol RBOvershellSlotCom::_GetDifficultyChangePromptToken() {
    static Symbol confirm;
    if (confirm == Symbol()) {
        confirm = Symbol("overshell_difficulty_confirm");
    }
    static Symbol confirmBTM;
    if (confirmBTM == Symbol()) {
        confirmBTM = Symbol("overshell_difficulty_confirm_btm");
    }
    static Symbol confirmSomp;
    if (confirmSomp == Symbol()) {
        confirmSomp = Symbol("overshell_difficulty_confirm_somp");
    }
    if (IsSomp()) {
        return confirmSomp;
    }
    return theGameData.GetGameMode() == kGameModeBTM ? confirmBTM : confirm;
}

// Reconstructed from eboot.elf at 0xC11380.
void RBOvershellSlotCom::_SetupDifficultyChangeConfirmMenu() {
    const Symbol text = _GetDifficultyChangePromptToken();
    eastl::vector<Symbol> actions;
    actions.reserve(3);
    static Symbol continueSong;
    if (continueSong == Symbol()) {
        continueSong = Symbol("overshell_continue");
    }
    static Symbol restart;
    if (restart == Symbol()) {
        restart = Symbol("overshell_restart");
    }
    static Symbol cancel;
    if (cancel == Symbol()) {
        cancel = Symbol("overshell_cancel");
    }
    Symbol focus;
    if (IsSomp()) {
        actions.push_back(continueSong);
        actions.push_back(cancel);
        focus = continueSong;
    } else if (theGameData.GetGameMode() == kGameModeBTM) {
        actions.push_back(restart);
        actions.push_back(cancel);
        focus = cancel;
    } else {
        actions.push_back(continueSong);
        actions.push_back(restart);
        actions.push_back(cancel);
        focus = continueSong;
    }
    _ShowConfirmActions(text, actions, focus);
}

// Reconstructed from eboot.elf at 0xC11A90. The text is cleared, as a token
// and as literal text, before the token is set.
void RBOvershellSlotCom::_ShowConfirmActions(Symbol text, const eastl::vector<Symbol>& actions, Symbol focus) {
    const Symbol none;
    mRuntime.mConfirmActionTextCom->SetTextToken(none);
    mRuntime.mConfirmActionTextCom->SetTextUtf8(none.Str());
    mRuntime.mConfirmActionTextCom->SetTextToken(text);
    mRuntime.mListProvider->SetConfirmActions(actions, focus);
}

// Reconstructed from eboot.elf at 0xC11B00.
void RBOvershellSlotCom::_SetupSoloToggleConfirmMenu() {
    static Symbol confirm;
    if (confirm == Symbol()) {
        confirm = Symbol("overshell_gtrsolo_confirm");
    }
    eastl::vector<Symbol> actions;
    actions.reserve(3);
    static Symbol continueSong;
    if (continueSong == Symbol()) {
        continueSong = Symbol("overshell_continue");
    }
    static Symbol restart;
    if (restart == Symbol()) {
        restart = Symbol("overshell_restart");
    }
    static Symbol cancel;
    if (cancel == Symbol()) {
        cancel = Symbol("overshell_cancel");
    }
    const bool somp = IsSomp();
    actions.push_back(continueSong);
    if (!somp) {
        actions.push_back(restart);
    }
    actions.push_back(cancel);
    _ShowConfirmActions(confirm, actions, continueSong);
}

// Reconstructed from eboot.elf at 0xC12020. The same code as
// _SetupSoloToggleConfirmMenu, with its own statics.
void RBOvershellSlotCom::_SetupCymbalToggleConfirmMenu() {
    static Symbol confirm;
    if (confirm == Symbol()) {
        confirm = Symbol("overshell_gtrsolo_confirm");
    }
    eastl::vector<Symbol> actions;
    actions.reserve(3);
    static Symbol continueSong;
    if (continueSong == Symbol()) {
        continueSong = Symbol("overshell_continue");
    }
    static Symbol restart;
    if (restart == Symbol()) {
        restart = Symbol("overshell_restart");
    }
    static Symbol cancel;
    if (cancel == Symbol()) {
        cancel = Symbol("overshell_cancel");
    }
    const bool somp = IsSomp();
    actions.push_back(continueSong);
    if (!somp) {
        actions.push_back(restart);
    }
    actions.push_back(cancel);
    _ShowConfirmActions(confirm, actions, continueSong);
}

// Reconstructed from eboot.elf at 0xC12540. The user's state is set and the
// slot refreshed directly, without ShowState.
void RBOvershellSlotCom::_FinishCymbalSelect(bool confirmed) {
    BandUser* const user = GetUser();
    const bool inSongSettings = InOverrideFlow(kOverrideFlowSongSettings);
    if (confirmed) {
        if (inSongSettings) {
            user->SetOvershellSlotState(kState_ChoosePart);
        }
    } else if (inSongSettings) {
        user->SetOvershellSlotState(kState_OptionsDrumMessage);
    }
    if (!InOverrideFlow(kOverrideFlowSongSettings)) {
        user->SetOvershellSlotState(
            mRuntime.mActiveStatus == kOvershellInSong ? kState_InSongMenu : kState_Options);
    }
    _RefreshOvershell(false);
}

// Reconstructed from eboot.elf at 0xC126F0.
void RBOvershellSlotCom::_DismissCymbalMessage() {
    GetUser()->SetOvershellSlotState(kState_ChooseDiff);
    _RefreshOvershell(false);
}

// Reconstructed from eboot.elf at 0xC132A0. The release build reads the
// user for a stripped check.
void RBOvershellSlotCom::_ConfirmChooseDiff() {
    static_cast<void>(GetUser());
    static Symbol difficulty;
    if (difficulty == Symbol()) {
        difficulty = Symbol("difficulty");
    }
    _ApplyDifficulty(static_cast<Difficulty>(mRuntime.mState->GetProperty(difficulty).LiteralInt(nullptr)));
    if (mRuntime.mOverrideFlow != kOverrideFlowNone) {
        mRuntime.mPartChosen = false;
        _SetOverrideType(kOverrideFlowNone, true);
    }
    if (!IsSomp()) {
        LeaveOptions(true);
    }
}

// Reconstructed from eboot.elf at 0xC13400.
void RBOvershellSlotCom::_LeaveChoosePart() {
    if (IsSomp()) {
        _ClearSompPart();
    }
    _CancelSongSettings();
    _RefreshOvershell(false);
}

// Reconstructed from eboot.elf at 0xC13930. The SOMP cancel token is built
// but not used.
void RBOvershellSlotCom::_SetupSongOptionsCancelMenu() {
    eastl::vector<Symbol> actions;
    static Symbol cancelSong;
    if (cancelSong == Symbol()) {
        cancelSong = Symbol("overshell_cancel_song");
    }
    static Symbol sompCancelSong;
    if (sompCancelSong == Symbol()) {
        sompCancelSong = Symbol("overshell_somp_cancel_song");
    }
    static Symbol no;
    if (no == Symbol()) {
        no = Symbol("overshell_confirm_no");
    }
    static Symbol yes;
    if (yes == Symbol()) {
        yes = Symbol("overshell_confirm_yes");
    }
    static Symbol returnToGreenRoom;
    if (returnToGreenRoom == Symbol()) {
        returnToGreenRoom = Symbol("overshell_somp_return_to_green_room");
    }
    static Symbol returnToMusicLibrary;
    if (returnToMusicLibrary == Symbol()) {
        returnToMusicLibrary = Symbol("overshell_somp_return_to_music_library");
    }
    static Symbol leave;
    if (leave == Symbol()) {
        leave = Symbol("overshell_leave");
    }
    static Symbol cancel;
    if (cancel == Symbol()) {
        cancel = Symbol("overshell_cancel");
    }
    Symbol focus;
    if (IsSomp()) {
        const bool leader = static_cast<RBBandUser*>(GetUser())->mSompLeader;
        actions.push_back(cancel);
        if (leader) {
            actions.push_back(returnToGreenRoom);
            actions.push_back(returnToMusicLibrary);
        }
        actions.push_back(leave);
        focus = cancel;
    } else {
        actions.push_back(no);
        actions.push_back(yes);
        actions.push_back(leave);
        focus = no;
    }
    _ShowConfirmActions(cancelSong, actions, focus);
}

// Reconstructed from eboot.elf at 0xC147F0.
void RBOvershellSlotCom::_RequestSompSkipSong() {
    RBSompClient* client;
    if (IsSomp() && GetUser() != nullptr && static_cast<RBBandUser*>(GetUser())->mSompLeader &&
        (client = theRBSompSessionMgr.mClient) != nullptr) {
        client->RequestSkipSong(GetUser());
        ShowState(kState_OnlineLeaderSkipToNextSongWait);
    } else {
        ShowState(kState_OnlineError);
    }
}

// Reconstructed from eboot.elf at 0xC148A0. A user leaves at once in the
// venue meta mode and once the show ended; otherwise the slot asks first.
void RBOvershellSlotCom::_OnVenueMetaDropOut() {
    const int mode = theGameData.GetGameMode();
    if (IsSomp()) {
        _ShowSompDropOutConfirm(false);
        return;
    }
    if (((mode == kGameModeVenueMeta || mode == kGameModeQuickplay) && mRuntime.mOvershell->mRuntime.mInVenueMetaMode) ||
        (theRBGameSession != nullptr && theRBGameSession->mState == kGameSessionStateEnded)) {
        AttemptRemoveUser();
        return;
    }
    ShowState(kState_RemoveUserInSongConfirm);
}

// Reconstructed from eboot.elf at 0xC14930. Practice quits without asking,
// as do the venue meta mode and an ended show.
void RBOvershellSlotCom::_OnAttemptQuit() {
    const int mode = theGameData.GetGameMode();
    if (IsSomp()) {
        _ShowSompDropOutConfirm(false);
        return;
    }
    if (((mode == kGameModeVenueMeta || mode == kGameModeQuickplay) && mRuntime.mOvershell->mRuntime.mInVenueMetaMode) ||
        mode == kGameModePractice ||
        (theRBGameSession != nullptr && theRBGameSession->mState == kGameSessionStateEnded)) {
        _QuitEarly(kLayoutInvalid);
        return;
    }
    ShowState(kState_QuitEarlyConfirm);
}

// Reconstructed from eboot.elf at 0xC149D0.
void RBOvershellSlotCom::_OnQuitEarly() {
    _QuitEarly(kLayoutInvalid);
}

// Reconstructed from eboot.elf at 0xC14BE0. The session takes the band to
// its green room.
void RBOvershellSlotCom::_QuitSompSong() {
    RBSompClient* client;
    if (IsSomp() && GetUser() != nullptr && (client = theRBSompSessionMgr.mClient) != nullptr) {
        client->RequestLeaveSong(GetUser());
        theRBSompSessionMgr.GotoLayout(kLayoutSompGreenRoom, Symbol());
        LeaveOptions(true);
    } else {
        _DropOutOfSompSession();
    }
}

// Reconstructed from eboot.elf at 0xC15170.
void RBOvershellSlotCom::_ToggleLeftyFlip() {
    BandUser* const user = GetUser();
    const ControllerType type = user->GetControllerType();
    IGameplayOptions* const options = user->GetGameplayOptions();
    options->SetLefty(type, !options->IsLefty(type));
    static Symbol updateLeftyCheckbox;
    if (updateLeftyCheckbox == Symbol()) {
        updateLeftyCheckbox = Symbol("update_lefty_checkbox");
    }
    Message<1> update(updateLeftyCheckbox);
    update.mData->Node(2) = DataNode(static_cast<int>(options->IsLefty(type)));
    mRuntime.mListProvider->Handle(update.mData, true);
    SendSompOptions(GetUser(), options);
}

// Reconstructed from eboot.elf at 0xC15440.
void RBOvershellSlotCom::_CycleDrumFillLogicForward() {
    IGameplayOptions* const options = GetUser()->GetGameplayOptions();
    options->SetDrumFillLogic((options->GetDrumFillLogic() + 1) % 3);
    static Symbol updateDrumFillCombobox;
    if (updateDrumFillCombobox == Symbol()) {
        updateDrumFillCombobox = Symbol("update_drum_fill_combobox");
    }
    Message<1> update(updateDrumFillCombobox);
    update.mData->Node(2) = DataNode(options->GetDrumFillLogic());
    mRuntime.mListProvider->Handle(update.mData, true);
}

// Reconstructed from eboot.elf at 0xC156A0.
void RBOvershellSlotCom::_CycleDrumFillLogicBackward() {
    IGameplayOptions* const options = GetUser()->GetGameplayOptions();
    const int logic = options->GetDrumFillLogic();
    options->SetDrumFillLogic(logic > 0 ? logic - 1 : 2);
    static Symbol updateDrumFillCombobox;
    if (updateDrumFillCombobox == Symbol()) {
        updateDrumFillCombobox = Symbol("update_drum_fill_combobox");
    }
    Message<1> update(updateDrumFillCombobox);
    update.mData->Node(2) = DataNode(options->GetDrumFillLogic());
    mRuntime.mListProvider->Handle(update.mData, true);
}

// Reconstructed from eboot.elf at 0xC158F0. Outside a song the toggle
// applies at once. The release build keeps the calls of a stripped check on
// the user's part.
void RBOvershellSlotCom::_ToggleCymbal(unsigned int lane) {
    BandUser* const user = GetUser();
    static_cast<void>(user->GetControllerType());
    static_cast<void>(user->GetPreferredScoreType());
    if (mRuntime.mCymbalTogglePending) {
        static_cast<void>(MakeErrorName());
    }
    mRuntime.mCymbalTogglePending = true;
    mRuntime.mCymbalToggleLane = static_cast<int>(lane);
    if (theGame != nullptr) {
        ShowState(kState_ToggleCymbalConfirm);
    } else {
        _ConfirmToggleCymbal();
    }
}

// Reconstructed from eboot.elf at 0xC15990. Without a pending lane the
// release build keeps only the error's name.
void RBOvershellSlotCom::_ConfirmToggleCymbal() {
    if (!mRuntime.mCymbalTogglePending) {
        static_cast<void>(MakeErrorName());
        return;
    }
    RBSystemOptions* const systemOptions = &theRBProfileMgr->mSystemOptions;
    systemOptions->SetProDrumCymbalLanes(systemOptions->mProDrumCymbalLanes ^ (1U << mRuntime.mCymbalToggleLane));
    static Symbol updateCymbalCheckbox;
    if (updateCymbalCheckbox == Symbol()) {
        updateCymbalCheckbox = Symbol("update_cymbal_checkbox");
    }
    Message<1> update(updateCymbalCheckbox);
    update.mData->Node(2) = DataNode(mRuntime.mCymbalToggleLane);
    mRuntime.mListProvider->Handle(update.mData, true);
    RBSompClient* const client = theRBSompSessionMgr.mClient;
    if (client != nullptr) {
        BandUser* const user = GetUser();
        client->SendGameplayOptions(
            user, GetUser()->GetGameplayOptions(), systemOptions, theRBSompSessionMgr.IsSessionNoFail());
    }
    mRuntime.mCymbalTogglePending = false;
    mRuntime.mCymbalToggleLane = -1;
}

// Reconstructed from eboot.elf at 0xC15C90.
void RBOvershellSlotCom::_CancelToggleCymbal() {
    if (!mRuntime.mCymbalTogglePending) {
        static_cast<void>(MakeErrorName());
    }
    mRuntime.mCymbalTogglePending = false;
    mRuntime.mCymbalToggleLane = -1;
}

// Reconstructed from eboot.elf at 0xC15CD0.
void RBOvershellSlotCom::_ToggleGuitarSoloMode() {
    static_cast<void>(GetUser());
    const int mode = GetUser()->GetGameplayOptions()->ToggleGuitarSoloMode();
    static Symbol updateSoloModeCheckbox;
    if (updateSoloModeCheckbox == Symbol()) {
        updateSoloModeCheckbox = Symbol("update_solo_mode_checkbox");
    }
    Message<1> update(updateSoloModeCheckbox);
    update.mData->Node(2) = DataNode(mode);
    mRuntime.mListProvider->Handle(update.mData, true);
    if (mRuntime.mActiveStatus == kOvershellInSong) {
        _ShowConfirmSoloToggle();
    }
}

// Reconstructed from eboot.elf at 0xC15F30.
void RBOvershellSlotCom::_ShowConfirmSoloToggle() {
    ShowState(kState_ConfirmSoloToggle);
}

// Reconstructed from eboot.elf at 0xC15F40.
void RBOvershellSlotCom::_RevertGuitarSoloMode() {
    static_cast<void>(GetUser());
    GetUser()->GetGameplayOptions()->ToggleGuitarSoloMode();
    ShowState(kState_OptionsGuitar);
}

// Reconstructed from eboot.elf at 0xC162A0.
void RBOvershellSlotCom::_ExportWidgetScrollMsg() {
    Message<0> scroll(gOnWidgetScroll);
    mObject->mEntity->Export(scroll.mData);
}

// Reconstructed from eboot.elf at 0xC165F0.
void RBOvershellSlotCom::_ToggleVocalImprov() {
    BandUser* const user = GetUser();
    IGameplayOptions* const options = user->GetGameplayOptions();
    options->SetVocalImprovEnabled(!options->IsVocalImprovEnabled());
    mRuntime.mVocalFreestyleIconOverride = _ShouldShowVocalFreestyleIcon(user->GetDifficulty());
    static Symbol updateVocalImprovCheckbox;
    if (updateVocalImprovCheckbox == Symbol()) {
        updateVocalImprovCheckbox = Symbol("update_vocal_improv_checkbox");
    }
    Message<1> update(updateVocalImprovCheckbox);
    update.mData->Node(2) = DataNode(static_cast<int>(options->IsVocalImprovEnabled()));
    mRuntime.mListProvider->Handle(update.mData, true);
    SendSompOptions(GetUser(), options);
}

// Reconstructed from eboot.elf at 0xC168C0.
void RBOvershellSlotCom::_ActivateVolumeSlider() {
    static Symbol activateVolumeSlider;
    if (activateVolumeSlider == Symbol()) {
        activateVolumeSlider = Symbol("activate_volume_slider");
    }
    Message<0> activate(activateVolumeSlider);
    mRuntime.mListProvider->Handle(activate.mData, true);
    mRuntime.mSliderReturnState = mRuntime.mState->GetStateID();
    ShowState(kState_OptionsSlider);
}

// Reconstructed from eboot.elf at 0xC169F0. The provider answers a step it
// took with a non-zero value, read without evaluating the node.
void RBOvershellSlotCom::_IncrementVolumeSlider() {
    static Symbol incrementVolumeSlider;
    if (incrementVolumeSlider == Symbol()) {
        incrementVolumeSlider = Symbol("increment_volume_slider");
    }
    Message<1> increment(incrementVolumeSlider);
    increment.mData->Node(2) = ObjectNode(this);
    const DataNode stepped = mRuntime.mListProvider->Handle(increment.mData, true);
    if (stepped.mValue.integer != 0) {
        _ExportWidgetScrollMsg();
    }
}

// Reconstructed from eboot.elf at 0xC16C20.
void RBOvershellSlotCom::_DecrementVolumeSlider() {
    static Symbol decrementVolumeSlider;
    if (decrementVolumeSlider == Symbol()) {
        decrementVolumeSlider = Symbol("decrement_volume_slider");
    }
    Message<1> decrement(decrementVolumeSlider);
    decrement.mData->Node(2) = ObjectNode(this);
    const DataNode stepped = mRuntime.mListProvider->Handle(decrement.mData, true);
    if (stepped.mValue.integer != 0) {
        _ExportWidgetScrollMsg();
    }
}

// Reconstructed from eboot.elf at 0xC16E50.
void RBOvershellSlotCom::_DeactivateVolumeSlider() {
    static Symbol deactivateVolumeSlider;
    if (deactivateVolumeSlider == Symbol()) {
        deactivateVolumeSlider = Symbol("deactivate_volume_slider");
    }
    Message<0> deactivate(deactivateVolumeSlider);
    mRuntime.mListProvider->Handle(deactivate.mData, true);
    if (mRuntime.mSliderReturnState == kState_JoinedDefault) {
        _ShowInstrumentOptions();
    } else {
        ShowState(mRuntime.mSliderReturnState);
    }
    _ResetSliderReturnState();
}

// Reconstructed from eboot.elf at 0xC16F90.
void RBOvershellSlotCom::_ResetSliderReturnState() {
    mRuntime.mSliderReturnState = kState_JoinedDefault;
}

// Reconstructed from eboot.elf at 0xC16FA0.
void RBOvershellSlotCom::_ActivateTextSlider() {
    static Symbol activateTextSlider;
    if (activateTextSlider == Symbol()) {
        activateTextSlider = Symbol("activate_text_slider");
    }
    Message<0> activate(activateTextSlider);
    mRuntime.mListProvider->Handle(activate.mData, true);
    // State 22, which the state table does not name.
    ShowState(kStateOptionsTextSlider);
}

// Reconstructed from eboot.elf at 0xC170C0.
void RBOvershellSlotCom::_DeactivateTextSlider() {
    static Symbol deactivateTextSlider;
    if (deactivateTextSlider == Symbol()) {
        deactivateTextSlider = Symbol("deactivate_text_slider");
    }
    Message<0> deactivate(deactivateTextSlider);
    mRuntime.mListProvider->Handle(deactivate.mData, true);
    _ShowInstrumentOptions();
}

// Reconstructed from eboot.elf at 0xC17380.
void RBOvershellSlotCom::_PrintUserInfo() {
    static_cast<void>(GetUser()->GetPlatformUserId());
    static_cast<void>(GetUser()->GetPlatformName());
}

// Reconstructed from eboot.elf at 0xC173F0.
void RBOvershellSlotCom::_ToggleMicOverdriveDeploy() {
    IGameplayOptions* const options = GetUser()->GetGameplayOptions();
    options->ToggleGameplayFlag();
    static Symbol updateMicOdDeployCheckbox;
    if (updateMicOdDeployCheckbox == Symbol()) {
        updateMicOdDeployCheckbox = Symbol("update_mic_od_deploy_checkbox");
    }
    Message<1> update(updateMicOdDeployCheckbox);
    update.mData->Node(2) = ObjectNode(this);
    mRuntime.mListProvider->Handle(update.mData, true);
    SendSompOptions(GetUser(), options);
}

// Reconstructed from eboot.elf at 0xC17690.
void RBOvershellSlotCom::_ShowTrackTextureMenu() {
    static_cast<void>(GetUser());
    // State 58, which the state table does not name.
    ShowState(kStateTrackTextureMenu);
}

// Reconstructed from eboot.elf at 0xC176D0.
void RBOvershellSlotCom::_ToggleShowLocalTrackTexture() {
    RBBandUser* const user = static_cast<RBBandUser*>(GetUser());
    user->SetPreferenceFlag0(!user->GetPreferenceFlag0());
}

// Reconstructed from eboot.elf at 0xC17720.
void RBOvershellSlotCom::_ToggleAllowRemoteTrackTextures() {
    if (IsSomp()) {
        theRBSompSessionMgr.SetAllowRemoteTrackTextures(!theRBSompSessionMgr.AllowsRemoteTrackTextures());
    }
}

// Reconstructed from eboot.elf at 0xC17760.
void RBOvershellSlotCom::_ToggleRandomTrackTexture() {
    RBBandUser* const user = static_cast<RBBandUser*>(GetUser());
    user->SetPreferenceFlag2(!user->GetPreferenceFlag2());
}

// Reconstructed from eboot.elf at 0xC177B0.
void RBOvershellSlotCom::_ToggleRandomTrackColor() {
    RBBandUser* const user = static_cast<RBBandUser*>(GetUser());
    user->SetPreferenceFlag3(!user->GetPreferenceFlag3());
}

// Reconstructed from eboot.elf at 0xC17960.
RBCharacterPersistentData* RBOvershellSlotCom::_GetManagementSessionCharacterDataOrFail() {
    static Symbol player;
    if (player == Symbol()) {
        player = Symbol("char_type_player");
    }
    static Symbol prefab;
    if (prefab == Symbol()) {
        prefab = Symbol("char_type_prefab");
    }
    static Symbol otherUser;
    if (otherUser == Symbol()) {
        otherUser = Symbol("char_type_other_user");
    }
    if (mRuntime.mCharManagementSession) {
        if (mRuntime.mManagedCharType == player) {
            return _GetCharacterDataOnProfileOrFail();
        }
        if (mRuntime.mManagedCharType == otherUser) {
            return _GetCharacterDataOnOtherProfileOrFail(String(mRuntime.mManagedCharOwner));
        }
    }
    static_cast<void>(MakeErrorName());
    ShowState(kState_CharacterError);
    return nullptr;
}

// Reconstructed from eboot.elf at 0xC17C10.
bool RBOvershellSlotCom::_AllowManageChar() {
    return !IsSomp();
}

// Reconstructed from eboot.elf at 0xC17C30. A prefab is not managed.
bool RBOvershellSlotCom::_StartCharManagementSession(Symbol name, Symbol type, String owner) {
    if (mRuntime.mCharManagementSession) {
        static_cast<void>(MakeErrorName());
        _EndCharManagementSession();
    }
    static Symbol player;
    if (player == Symbol()) {
        player = Symbol("char_type_player");
    }
    static Symbol prefab;
    if (prefab == Symbol()) {
        prefab = Symbol("char_type_prefab");
    }
    static Symbol otherUser;
    if (otherUser == Symbol()) {
        otherUser = Symbol("char_type_other_user");
    }
    if (type == player && _GetCharacterDataOnProfileOrFail() == nullptr) {
        return false;
    }
    if (type != player && type != otherUser) {
        static_cast<void>(MakeErrorName());
        ShowState(kState_CharacterError);
        return false;
    }
    mRuntime.mCharManagementSession = true;
    mRuntime.mManagedCharName = name;
    mRuntime.mManagedCharType = type;
    mRuntime.mManagedCharOwner = owner;
    return true;
}

// Reconstructed from eboot.elf at 0xC17EC0.
void RBOvershellSlotCom::_EndCharManagementSession() {
    mRuntime.mCharManagementSession = false;
    mRuntime.mManagedCharName = Symbol("");
    mRuntime.mManagedCharType = Symbol("");
    mRuntime.mManagedCharOwner = "";
}

// Reconstructed from eboot.elf at 0xC17F50. The user's name is read for
// the release build's stripped report.
RBCharacterPersistentData* RBOvershellSlotCom::_GetCharacterDataOnProfileOrFail() {
    BandUser* const user = GetUser();
    if (user != nullptr && RBBandUserMgr::HasProfileAndController(user)) {
        RBProfile* const profile = user->GetProfile(false);
        if (profile != nullptr) {
            return &profile->mCharacterData;
        }
        static_cast<void>(MakeErrorName());
        static_cast<void>(user->GetPlatformName());
    } else {
        static_cast<void>(MakeErrorName());
    }
    ShowState(kState_CharacterError);
    return nullptr;
}

// Reconstructed from eboot.elf at 0xC17FE0.
bool RBOvershellSlotCom::_IsInCharManagementSessionOrFail() {
    if (mRuntime.mCharManagementSession) {
        return true;
    }
    static_cast<void>(MakeErrorName());
    ShowState(kState_CharacterError);
    return false;
}

// Reconstructed from eboot.elf at 0xC18020.
Symbol RBOvershellSlotCom::_GetManagementSessionChar() {
    if (mRuntime.mCharManagementSession) {
        return mRuntime.mManagedCharName;
    }
    static_cast<void>(MakeErrorName());
    return Symbol("");
}

// Reconstructed from eboot.elf at 0xC18080. Managing the slot's own user
// this way is reported but goes on.
RBCharacterPersistentData* RBOvershellSlotCom::_GetCharacterDataOnOtherProfileOrFail(String owner) {
    UserGuid guid;
    guid = String(owner);
    BandUser* const user = theBandUserMgr->GetBandUser(guid, false);
    if (user == nullptr) {
        static_cast<void>(MakeErrorName());
    } else {
        if (user == GetUser()) {
            static_cast<void>(MakeErrorName());
        }
        RBProfile* const profile = user->GetProfile(false);
        if (profile != nullptr) {
            return &profile->mCharacterData;
        }
        static_cast<void>(MakeErrorName());
        static_cast<void>(user->GetPlatformName());
    }
    ShowState(kState_CharacterError);
    return nullptr;
}

// Reconstructed from eboot.elf at 0xC18180.
bool RBOvershellSlotCom::_GetCharTypeFromSymbols(
    const Symbol& type,
    const String& owner,
    RBBandUser::CharType& charType,
    UserGuid& ownerGuid) const {
    static Symbol player;
    if (player == Symbol()) {
        player = Symbol("char_type_player");
    }
    static Symbol prefab;
    if (prefab == Symbol()) {
        prefab = Symbol("char_type_prefab");
    }
    static Symbol otherUser;
    if (otherUser == Symbol()) {
        otherUser = Symbol("char_type_other_user");
    }
    if (type == player) {
        charType = kCharTypePlayer;
        return true;
    }
    if (type == prefab) {
        charType = kCharTypePrefab;
        return true;
    }
    if (type == otherUser) {
        charType = kCharTypeOtherUser;
        ownerGuid = String(owner);
        return true;
    }
    charType = RBBandUser::kCharTypeNone;
    return false;
}

// Reconstructed from eboot.elf at 0xC18350.
void RBOvershellSlotCom::_StartCharCreate() {
    RBCharacterPersistentData* const data = _GetCharacterDataOnProfileOrFail();
    if (data == nullptr) {
        return;
    }
    if (!data->HasRoomForMoreCharacters()) {
        ShowState(kState_CreateCharacterDenialNoRoom);
        return;
    }
    if (RBCharacterMenuMgr::TryStartPlayerSession(
            GetUser(),
            GetUser()->GetPadNum(),
            data->mProfile,
            RBCharacterMenuMgr::kCharSessionCreate,
            theRBMetaState.mCurrentLayout)) {
        theRBMetaState.Goto(kLayoutCharacterCreate, false);
    }
}

// Reconstructed from eboot.elf at 0xC18440.
void RBOvershellSlotCom::_StartCharCreateOther(String owner) {
    RBCharacterPersistentData* const data = _GetCharacterDataOnOtherProfileOrFail(String(owner));
    if (data == nullptr) {
        return;
    }
    if (!data->HasRoomForMoreCharacters()) {
        ShowState(kState_CreateCharacterDenialNoRoom);
        return;
    }
    if (RBCharacterMenuMgr::TryStartPlayerSession(
            GetUser(),
            GetUser()->GetPadNum(),
            data->mProfile,
            RBCharacterMenuMgr::kCharSessionCreate,
            theRBMetaState.mCurrentLayout)) {
        theRBMetaState.Goto(kLayoutCharacterCreate, false);
    }
}

// Reconstructed from eboot.elf at 0xC18550.
void RBOvershellSlotCom::_StartRockShop() {
    RBCharacterPersistentData* const data = _GetManagementSessionCharacterDataOrFail();
    if (data == nullptr) {
        return;
    }
    if (!RBCharacterMenuMgr::TryStartPlayerSession(
            GetUser(),
            GetUser()->GetPadNum(),
            data->mProfile,
            RBCharacterMenuMgr::kCharSessionShop,
            theRBMetaState.mCurrentLayout)) {
        return;
    }
    if (_TryToSetActiveCharForUser(
            GetUser(), mRuntime.mManagedCharName, mRuntime.mManagedCharType, String(mRuntime.mManagedCharOwner))) {
        _EnterShop();
    } else {
        RBCharacterMenuMgr::EndPlayerSession();
        ShowState(kState_CharacterError);
    }
}

// Reconstructed from eboot.elf at 0xC18960.
void RBOvershellSlotCom::_StartGenericShopSession() {
    _StartShopSession(false);
}

// Reconstructed from eboot.elf at 0xC18970.
void RBOvershellSlotCom::_StartShopSession(bool trackTexShop) {
    if (!_CanStartShopSession()) {
        ShowState(kState_CharacterError);
        return;
    }
    BandUser* const user = GetUser();
    const int padNum = GetUser()->GetPadNum();
    if (RBCharacterMenuMgr::TryStartPlayerSession(
            user,
            padNum,
            GetUser()->GetProfile(false),
            RBCharacterMenuMgr::kCharSessionShop,
            theRBMetaState.mCurrentLayout)) {
        RBCharacterMenuMgr::SetTrackTexShopSession(trackTexShop);
        _EnterShop();
    }
}

// Reconstructed from eboot.elf at 0xC18A70.
void RBOvershellSlotCom::_StartTrackTexShopSession() {
    _StartShopSession(true);
}

// Reconstructed from eboot.elf at 0xC18A80.
void RBOvershellSlotCom::_StartCharEdit() {
    RBCharacterPersistentData* const data = _GetManagementSessionCharacterDataOrFail();
    if (data == nullptr) {
        return;
    }
    if (!RBCharacterMenuMgr::TryStartPlayerSession(
            GetUser(),
            GetUser()->GetPadNum(),
            data->mProfile,
            RBCharacterMenuMgr::kCharSessionEdit,
            theRBMetaState.mCurrentLayout)) {
        return;
    }
    if (_TryToSetActiveCharForUser(
            GetUser(), mRuntime.mManagedCharName, mRuntime.mManagedCharType, String(mRuntime.mManagedCharOwner))) {
        theRBMetaState.Goto(kLayoutCharacterCreate, false);
    } else {
        RBCharacterMenuMgr::EndPlayerSession();
        ShowState(kState_CharacterError);
    }
}

// Reconstructed from eboot.elf at 0xC18FD0.
void RBOvershellSlotCom::_SelectActiveChar(Symbol name, Symbol type, String owner) {
    ShowState(_SetActiveChar(name, type, String(owner)) ? kState_Options : kState_CharacterError);
}

// Reconstructed from eboot.elf at 0xC19170. A failure is only reported.
void RBOvershellSlotCom::_SelectBandChar(Symbol name, Symbol type, String owner) {
    if (_SetActiveChar(name, type, String(owner))) {
        ShowState(kState_ChooseBandMemberReady);
    } else {
        static_cast<void>(MakeErrorName());
    }
}

// Reconstructed from eboot.elf at 0xC19210. The message carries the slot
// as its object.
void RBOvershellSlotCom::_RefreshCharList() {
    if (GetUser() == nullptr) {
        return;
    }
    static Symbol refreshCharData;
    if (refreshCharData == Symbol()) {
        refreshCharData = Symbol("refresh_char_data");
    }
    Message<1> refresh(refreshCharData);
    DataNode slot;
    slot.mType = kDataObject;
    slot.mValue.object = this;
    refresh.mData->Node(2) = slot;
    static_cast<void>(mRuntime.mListProvider->Handle(refresh.mData, true));
}

// Reconstructed from eboot.elf at 0xC19430.
void RBOvershellSlotCom::_ActivateCharPreviewScene() {
    mRuntime.mNeedsCharPreviewScene = true;
    mRuntime.mOvershell->UpdateCharPreviewSceneState();
}

// Reconstructed from eboot.elf at 0xC19520.
bool RBOvershellSlotCom::NeedsCharPreviewScene() {
    return mRuntime.mNeedsCharPreviewScene;
}

// Reconstructed from eboot.elf at 0xC19790.
bool RBOvershellSlotCom::_ShowPreviewOfManagedChar() {
    if (!mRuntime.mCharManagementSession) {
        static_cast<void>(MakeErrorName());
        return false;
    }
    return _ShowCharPreview(
        mRuntime.mManagedCharName, mRuntime.mManagedCharType, String(mRuntime.mManagedCharOwner));
}

// Reconstructed from eboot.elf at 0xC19830. The idle flow's original
// master decides first; otherwise any user who may be the master.
void RBOvershellSlotCom::_CheckIfIdleMaster() {
    RBOvershellCom* const overshell = mRuntime.mOvershell;
    OvershellSlotStateID state;
    if (overshell->IsAnySlotOriginalIdleMaster()) {
        state = overshell->SlotIsOriginalIdleMaster(mRuntime.mBandSlot) ? kState_IdleMaster
                                                                          : kState_IdleNotTheMaster;
    } else if (overshell->IsAnySlotPossibleIdleMaster()) {
        state = mRuntime.mOvershell->SlotIsPossibleIdleMaster(mRuntime.mBandSlot) ? kState_IdleMaster
                                                                                   : kState_IdleNotTheMaster;
    } else {
        state = kState_IdleNoMaster;
    }
    ShowState(state);
}

// Reconstructed from eboot.elf at 0xC198A0.
UIListCom* RBOvershellSlotCom::_GetUIList() const {
    return mRuntime.mUIList;
}

// Reconstructed from eboot.elf at 0xC198B0.
float RBOvershellSlotCom::GetSompWaitTime() const {
    if (GetUser() == nullptr) {
        return -1.0F;
    }
    return TheTimeMgr->mRealTime.SplitMs() - mRuntime.mLastInputMs;
}

// Reconstructed from eboot.elf at 0xC19950.
bool RBOvershellSlotCom::_IsLocalSlot() const {
    if (mRuntime.mOvershell == nullptr) {
        return true;
    }
    return !mRuntime.mOvershell->IsSlotOffline(mRuntime.mBandSlot);
}

// Reconstructed from eboot.elf at 0xC19980. The tables of the remote
// statuses are interned in full on the first call; sorting, previewing,
// selected, removed, filtering, overshell up and taunt are not set here.
void RBOvershellSlotCom::_UpdateSompStatusToken() {
    static Symbol browsing;
    if (browsing == Symbol()) {
        browsing = Symbol("somp_remote_browsing");
    }
    static Symbol searching;
    if (searching == Symbol()) {
        searching = Symbol("somp_remote_searching");
    }
    static Symbol sorting;
    if (sorting == Symbol()) {
        sorting = Symbol("somp_remote_sorting");
    }
    static Symbol previewing;
    if (previewing == Symbol()) {
        previewing = Symbol("somp_remote_previewing");
    }
    static Symbol selected;
    if (selected == Symbol()) {
        selected = Symbol("somp_remote_selected");
    }
    static Symbol removed;
    if (removed == Symbol()) {
        removed = Symbol("somp_remote_removed");
    }
    static Symbol filtering;
    if (filtering == Symbol()) {
        filtering = Symbol("somp_remote_filtering");
    }
    static Symbol overshellUp;
    if (overshellUp == Symbol()) {
        overshellUp = Symbol("somp_remote_overshell_up");
    }
    static Symbol songResults;
    if (songResults == Symbol()) {
        songResults = Symbol("somp_remote_song_results");
    }
    static Symbol taunt;
    if (taunt == Symbol()) {
        taunt = Symbol("somp_remote_taunt");
    }
    static Symbol ready;
    if (ready == Symbol()) {
        ready = Symbol("somp_remote_ready");
    }
    static Symbol greenRoom;
    if (greenRoom == Symbol()) {
        greenRoom = Symbol("somp_remote_green_room");
    }
    static Symbol playing;
    if (playing == Symbol()) {
        playing = Symbol("somp_remote_playing");
    }
    static Symbol choosingDifficulty;
    if (choosingDifficulty == Symbol()) {
        choosingDifficulty = Symbol("somp_choosing_difficulty");
    }
    for (Symbol& token : mRuntime.mSompStatusTokens) {
        token = Symbol();
    }
    if (theRBSompSessionMgr.mClient == nullptr) {
        return;
    }
    BandUser* const user = GetUser();
    switch (theRBMetaState.mCurrentLayout) {
    case kLayoutSongSelect:
        mRuntime.mSompStatusTokens[0] = browsing;
        break;
    case kLayoutDifficultySelect:
        mRuntime.mSompStatusTokens[0] = choosingDifficulty;
        break;
    case kLayoutGame:
        mRuntime.mSompStatusTokens[0] =
            theRBMetaState.GetMetaGameplayState() == kMetaGameplayStateOnResultsScreen ? songResults : playing;
        break;
    case kLayoutLoading:
        mRuntime.mSompStatusTokens[0] = playing;
        break;
    case kLayoutSongSelectSearch:
        mRuntime.mSompStatusTokens[0] = searching;
        break;
    case kLayoutSompGreenRoom:
        mRuntime.mSompStatusTokens[0] =
            user != nullptr && static_cast<RBBandUser*>(user)->mSompHost ? ready : greenRoom;
        break;
    default:
        break;
    }
}

// Reconstructed from eboot.elf at 0xC1A1A0.
bool RBOvershellSlotCom::IsInDifficultySelect() {
    return theRBMetaState.mCurrentLayout == kLayoutDifficultySelect;
}

// Reconstructed from eboot.elf at 0xC1A1B0.
bool RBOvershellSlotCom::IsDifficultySelectLayout(RBUILayout layout) {
    return layout == kLayoutDifficultySelect;
}

// Reconstructed from eboot.elf at 0xC1A240. Only an online overshell's
// online slot takes them.
void RBOvershellSlotCom::SetSompStatusTokens(Symbol status, Symbol status2, Symbol status3) {
    RBOvershellCom* const overshell = mRuntime.mOvershell;
    if (overshell != nullptr && overshell->IsOnline() && overshell->IsSlotOnline(mRuntime.mBandSlot)) {
        mRuntime.mSompStatusTokens[0] = status;
        mRuntime.mSompStatusTokens[1] = status2;
        mRuntime.mSompStatusTokens[2] = status3;
    }
}

// Reconstructed from eboot.elf at 0xC1A580. The release build keeps the
// calls of a stripped log of the user.
void RBOvershellSlotCom::_OnSompResponseRejected() {
    _ClearSompPart();
    const RBUILayout layout = theRBMetaState.mCurrentLayout;
    if (layout != kLayoutDifficultySelect && layout != kLayoutGame) {
        return;
    }
    if (theUI->IsInTransition() || theRBSompSessionMgr.IsSompSessionEnding()) {
        return;
    }
    BandUser* const user = GetUser();
    if (user == nullptr || !static_cast<RBBandUser*>(user)->IsLocal()) {
        return;
    }
    static_cast<void>(user->GetPlatformUserId());
    static_cast<void>(user->GetPlatformName());
    ShowState(kState_OnlineError);
}

// Reconstructed from eboot.elf at 0xC1A970. An answer for another target
// shows as a failure; the release build keeps the calls of a stripped log
// of the user.
void RBOvershellSlotCom::OnSompKickResult(bool success, Symbol name, Symbol id) {
    if (GetUser() == nullptr || mRuntime.mState->GetStateID() != kState_OnlinePlayerKickResultWait) {
        return;
    }
    if (GetKickTargetName() == name && GetKickTargetId() == id) {
        mRuntime.mKickSucceeded = success;
    } else {
        static_cast<void>(GetUser()->GetPlatformUserId());
        static_cast<void>(GetUser()->GetPlatformName());
        mRuntime.mKickSucceeded = false;
    }
    ShowState(kState_OnlinePlayerKickResultDisplay);
}

// Reconstructed from eboot.elf at 0xC1AA80.
Symbol RBOvershellSlotCom::GetKickTargetName() const {
    return mRuntime.mKickTargetSet ? mRuntime.mKickTargetName : Symbol();
}

// Reconstructed from eboot.elf at 0xC1AAA0.
Symbol RBOvershellSlotCom::GetKickTargetId() const {
    return mRuntime.mKickTargetSet ? mRuntime.mKickTargetId : Symbol();
}

// Reconstructed from eboot.elf at 0xC1AAC0.
void RBOvershellSlotCom::OnSompReturnToGreenRoomResult(BandUser* user, const UserGuid& guid, bool success) {
    bool current;
    if (!_IsSompResponseForUser(user, guid, -1, kState_OnlineLeaderReturnToGreenRoomWait, &current)) {
        return;
    }
    if (success) {
        if (current) {
            LeaveOptions(true);
        }
    } else if (current) {
        _RevertToOnlineReturnState();
    }
}

// Reconstructed from eboot.elf at 0xC1ABA0.
void RBOvershellSlotCom::_RevertToOnlineReturnState() {
    const OvershellSlotStateID state = mRuntime.mOnlineReturnState;
    mRuntime.mOnlineReturnState = kState_JoinedDefault;
    ShowState(state);
}

// Reconstructed from eboot.elf at 0xC1ABC0.
void RBOvershellSlotCom::OnSompReturnToMusicLibraryResult(BandUser* user, const UserGuid& guid, bool success) {
    bool current;
    if (!_IsSompResponseForUser(user, guid, -1, kState_OnlineLeaderReturnToMusicLibraryWait, &current)) {
        return;
    }
    if (success) {
        if (current) {
            LeaveOptions(true);
        }
    } else if (current) {
        _RevertToOnlineReturnState();
    }
}

// Reconstructed from eboot.elf at 0xC1ACA0.
void RBOvershellSlotCom::OnSompSkipToNextSongResult(BandUser* user, const UserGuid& guid, bool success) {
    static_cast<void>(success);
    bool current;
    if (_IsSompResponseForUser(user, guid, -1, kState_OnlineLeaderSkipToNextSongWait, &current) && current) {
        LeaveOptions(true);
    }
}

// Reconstructed from eboot.elf at 0xC1AE50. The release build keeps the
// calls of a stripped log: the state's id, and a remote user.
void RBOvershellSlotCom::_SendSompChoosingDifficulty() {
    if (!IsSomp()) {
        return;
    }
    RBSompClient* const client = theRBSompSessionMgr.mClient;
    if (client == nullptr) {
        return;
    }
    static_cast<void>(mRuntime.mState->GetStateID());
    if (mRuntime.mState->IsRemote()) {
        return;
    }
    BandUser* const user = GetUser();
    if (user == nullptr) {
        return;
    }
    if (static_cast<RBBandUser*>(user)->IsLocal()) {
        client->SendChoosingDifficulty(GetUser());
    } else {
        static_cast<void>(user->GetPlatformUserId());
        static_cast<void>(user->GetPlatformName());
    }
}

// Reconstructed from eboot.elf at 0xC1AFA0.
bool RBOvershellSlotCom::SetKickTarget(Symbol name, Symbol id) {
    ClearKickTarget();
    mRuntime.mKickTargetSet = true;
    mRuntime.mKickTargetName = name;
    mRuntime.mKickTargetId = id;
    return true;
}

// Reconstructed from eboot.elf at 0xC1AFE0.
void RBOvershellSlotCom::ClearKickTarget() {
    if (mRuntime.mKickTargetSet) {
        mRuntime.mKickTargetSet = false;
        mRuntime.mKickTargetName = Symbol();
        mRuntime.mKickTargetId = Symbol();
    }
}

// Reconstructed from eboot.elf at 0xC1B010.
bool RBOvershellSlotCom::_HasKickTarget() {
    if (mRuntime.mKickTargetSet) {
        return true;
    }
    ShowState(kState_OnlineError);
    return false;
}

// Reconstructed from eboot.elf at 0xC1B040. Without a client the request
// still goes to the null client after the error shows.
void RBOvershellSlotCom::_KickPlayer() {
    RBSompClient* const client = theRBSompSessionMgr.mClient;
    if (client == nullptr) {
        ShowState(kState_OnlineError);
    }
    client->RequestKickPlayer(GetUser(), GetKickTargetName(), GetKickTargetId());
    ShowState(kState_OnlinePlayerKickResultWait);
}

// Reconstructed from eboot.elf at 0xC1B0D0. The text names the target when
// the token has a localization.
void RBOvershellSlotCom::_ShowKickPlayerConfirmation() {
    static Symbol confirmation;
    if (confirmation == Symbol()) {
        confirmation = Symbol("overshell_somp_kick_player_confirmation");
    }
    bool found = false;
    Symbol text(Localize(confirmation, &found, 0, TheLocale));
    if (found) {
        FormatString format(text.Str());
        format << GetKickTargetName().Str();
        text = Symbol(format.Str());
    }
    static Symbol yes;
    if (yes == Symbol()) {
        yes = Symbol("overshell_confirm_yes");
    }
    static Symbol no;
    if (no == Symbol()) {
        no = Symbol("overshell_confirm_no");
    }
    eastl::vector<Symbol> actions;
    actions.push_back(yes);
    actions.push_back(no);
    _ShowConfirmActionsText(text.Str(), actions, no);
}

// Reconstructed from eboot.elf at 0xC1B4B0. The token and the old text are
// cleared first.
void RBOvershellSlotCom::_ShowConfirmActionsText(
    const char* text,
    const eastl::vector<Symbol>& actions,
    Symbol focus) {
    mRuntime.mConfirmActionTextCom->SetTextToken(Symbol());
    mRuntime.mConfirmActionTextCom->SetTextUtf8(Symbol().Str());
    mRuntime.mConfirmActionTextCom->SetTextUtf8(text);
    mRuntime.mListProvider->SetConfirmActions(actions, focus);
}

// Reconstructed from eboot.elf at 0xC1B520.
void RBOvershellSlotCom::_ShowKickPlayerResult() {
    static Symbol success;
    if (success == Symbol()) {
        success = Symbol("overshell_somp_kick_player_success");
    }
    static Symbol failure;
    if (failure == Symbol()) {
        failure = Symbol("overshell_somp_kick_player_failure");
    }
    bool found = false;
    Symbol text(Localize(mRuntime.mKickSucceeded ? success : failure, &found, 0, TheLocale));
    if (found) {
        FormatString format(text.Str());
        format << GetKickTargetName().Str();
        text = Symbol(format.Str());
    }
    static Symbol continueAction;
    if (continueAction == Symbol()) {
        continueAction = Symbol("overshell_continue");
    }
    eastl::vector<Symbol> actions;
    actions.push_back(continueAction);
    _ShowConfirmActionsText(text.Str(), actions, continueAction);
}

// Reconstructed from eboot.elf at 0xC1B860.
void RBOvershellSlotCom::_SetupEmoteCategoryList() {
    static Symbol setup;
    if (setup == Symbol()) {
        setup = Symbol("setup_online_pick_emote_category");
    }
    Message<1> msg(setup);
    msg.mData->Node(2) = DataNode(mRuntime.mEmoteCategory);
    static_cast<void>(mRuntime.mListProvider->Handle(msg.mData, true));
}

// Reconstructed from eboot.elf at 0xC1BA50.
void RBOvershellSlotCom::_SetupEmoteList() {
    static Symbol setup;
    if (setup == Symbol()) {
        setup = Symbol("setup_online_pick_emote");
    }
    Message<1> msg(setup);
    msg.mData->Node(2) = DataNode(mRuntime.mEmoteCategory);
    static_cast<void>(mRuntime.mListProvider->Handle(msg.mData, true));
}

// Reconstructed from eboot.elf at 0xC1BC40.
int RBOvershellSlotCom::GetEmoteCategory() const {
    return mRuntime.mEmoteCategory;
}

// Reconstructed from eboot.elf at 0xC1BC50.
void RBOvershellSlotCom::_ClearListData() {
    if (mRuntime.mListProvider != nullptr) {
        mRuntime.mListProvider->ClearListData();
    }
}

// Reconstructed from eboot.elf at 0xC1BC60.
void RBOvershellSlotCom::SetEmoteCategory(int category) {
    mRuntime.mEmoteCategory = category;
}

// Reconstructed from eboot.elf at 0xC1BC70.
void RBOvershellSlotCom::_ClearEmote() {
    if (mRuntime.mEmoteCategory != -1) {
        mRuntime.mEmoteCategory = -1;
    }
    mRuntime.mEmote = -1;
}

// Reconstructed from eboot.elf at 0xC1BDF0.
void RBOvershellSlotCom::_ExportSelectMsg() {
    Message<0> msg(gOnSelect);
    mObject->mEntity->Export(msg.mData);
}

// Reconstructed from eboot.elf at 0xC1C2D0. The pending request's id is
// forgotten either way.
void RBOvershellSlotCom::_OnSompPickEmoteResult(
    BandUser* user,
    const UserGuid& guid,
    bool succeeded,
    bool throttled) {
    bool current;
    if (!_IsSompResponseForUser(user, guid, -1, kState_OnlinePickEmoteWait, &current)) {
        return;
    }
    if (succeeded) {
        const RBSompEmoteRecord* const emote = theRBSompSessionMgr.FindEmoteRecord(mRuntime.mEmote);
        LeaveOptions(true);
        if (emote != nullptr) {
            QueueMessage(emote->mToken, true);
        }
    } else {
        ShowState(throttled ? kState_OnlinePickEmoteRateThrottle : kState_OnlineError);
    }
    mRuntime.mEmote = -1;
}

// Reconstructed from eboot.elf at 0xC1C500.
void RBOvershellSlotCom::QueueMessage(Symbol token, bool literal) {
    mRuntime.mMessageChanged = true;
    mRuntime.mMessageQueue.push_back(QueuedMessage{token, literal});
}

// Reconstructed from eboot.elf at 0xC1C5E0.
bool RBOvershellSlotCom::_GetSompClient(bool leaderOnly, RBSompClient** client) const {
    *client = nullptr;
    if (!IsSomp() || GetUser() == nullptr) {
        return false;
    }
    if (leaderOnly && !static_cast<RBBandUser*>(GetUser())->mSompLeader) {
        return false;
    }
    *client = theRBSompSessionMgr.mClient;
    return *client != nullptr;
}

// Reconstructed from eboot.elf at 0xC1C680.
void RBOvershellSlotCom::_GotoSompLayout(RBUILayout layout) {
    theRBSompSessionMgr.GotoLayout(layout, Symbol());
    LeaveOptions(true);
}

// Reconstructed from eboot.elf at 0xC1C860.
void RBOvershellSlotCom::_ReturnToGreenRoom() {
    RBSompClient* client;
    if (_GetSompClient(true, &client)) {
        client->RequestReturnToGreenRoom(GetUser());
        ShowState(kState_OnlineLeaderReturnToGreenRoomWait);
    } else {
        ShowState(kState_OnlineError);
    }
}

// Reconstructed from eboot.elf at 0xC1C910.
void RBOvershellSlotCom::_ReturnToMusicLibrary() {
    RBSompClient* client;
    if (_GetSompClient(true, &client)) {
        client->RequestReturnToMusicLibrary(GetUser());
        ShowState(kState_OnlineLeaderReturnToMusicLibraryWait);
    } else {
        ShowState(kState_OnlineError);
    }
}

// Reconstructed from eboot.elf at 0xC1C9C0.
void RBOvershellSlotCom::_ShowQuitSessionConfirm() {
    mRuntime.mOnlineReturnState = mRuntime.mState->GetStateID();
    ShowState(kState_OnlineQuitSessionConfirm);
}

// Reconstructed from eboot.elf at 0xC1C9F0.
void RBOvershellSlotCom::_ShowReturnToGreenRoomConfirm() {
    mRuntime.mOnlineReturnState = mRuntime.mState->GetStateID();
    ShowState(kState_OnlineLeaderReturnToGreenRoomConfirm);
}

// Reconstructed from eboot.elf at 0xC1CA20.
void RBOvershellSlotCom::_ShowReturnToMusicLibraryConfirm() {
    mRuntime.mOnlineReturnState = mRuntime.mState->GetStateID();
    ShowState(kState_OnlineLeaderReturnToMusicLibraryConfirm);
}

// Reconstructed from eboot.elf at 0xC1CA50.
void RBOvershellSlotCom::_ShowOnlineSettings() {
    mRuntime.mOnlineSettingsReturnState = mRuntime.mState->GetStateID();
    ShowState(kState_OnlineSettings);
}

// Reconstructed from eboot.elf at 0xC1CA80.
void RBOvershellSlotCom::_LeaveOnlineSettings() {
    const OvershellSlotStateID state = mRuntime.mOnlineSettingsReturnState;
    if (state == kState_JoinedDefault) {
        ShowState(kState_OnlineError);
        return;
    }
    mRuntime.mOnlineSettingsReturnState = kState_JoinedDefault;
    ShowState(state);
}

// Reconstructed from eboot.elf at 0xC1CAB0.
void RBOvershellSlotCom::_ReturnToOnlineSettings() {
    ShowState(kState_OnlineSettings);
}

// Reconstructed from eboot.elf at 0xC1CAC0.
void RBOvershellSlotCom::_ShowOnlineExitSubmenu() {
    mRuntime.mExitSubmenuReturnState = mRuntime.mState->GetStateID();
    ShowState(kState_OnlineExitSubmenu);
}

// Reconstructed from eboot.elf at 0xC1CAF0.
void RBOvershellSlotCom::_LeaveOnlineExitSubmenu() {
    const OvershellSlotStateID state = mRuntime.mExitSubmenuReturnState;
    if (state == kState_JoinedDefault) {
        ShowState(kState_OnlineError);
        return;
    }
    mRuntime.mExitSubmenuReturnState = kState_JoinedDefault;
    ShowState(state);
}

// Reconstructed from eboot.elf at 0xC1CB20. Offline outside a SOMP
// session.
OvershellOnlineState RBOvershellSlotCom::GetOvershellOnlineState() const {
    const RBOvershellCom* const overshell = mRuntime.mOvershell;
    if (overshell == nullptr || !IsSomp()) {
        return kOvershellOnlineStateOffline;
    }
    if (overshell->IsPendingOnline()) {
        return kOvershellOnlineStatePendingOnline;
    }
    if (overshell->IsPendingOffline()) {
        return kOvershellOnlineStatePendingOffline;
    }
    return overshell->IsOnline() ? kOvershellOnlineStateOnline : kOvershellOnlineStateOffline;
}

// Reconstructed from eboot.elf at 0xC1CB80.
bool RBOvershellSlotCom::IsUserSompLeader() const {
    if (!IsSomp() || GetUser() == nullptr) {
        return false;
    }
    return static_cast<RBBandUser*>(GetUser())->mSompLeader;
}

// Reconstructed from eboot.elf at 0xC1CC00.
bool RBOvershellSlotCom::ShowsSompConnectionSpinner() const {
    RBOvershellSlotState* const state = mRuntime.mState;
    return state != nullptr && state->ShowOnlineSpinner();
}

// Reconstructed from eboot.elf at 0xC1CC20.
bool RBOvershellSlotCom::_IsNotOnlineSlot() const {
    return mRuntime.mOvershell != nullptr && mRuntime.mOvershell->IsNotOnlineSlot(mRuntime.mBandSlot);
}

// Reconstructed from eboot.elf at 0xC1CC40. A remote user's state comes
// from the flags of the session's record; the slot is offline unless the
// online overshell shows.
OvershellSlotConnectionState RBOvershellSlotCom::GetConnectionState() const {
    if (!IsFullOvershellOnline()) {
        return kSlotConnectionOffline;
    }
    BandUser* const user = GetUser();
    if (user == nullptr || static_cast<RBBandUser*>(user)->IsLocal()) {
        return kSlotConnectionOffline;
    }
    const RemoteBandUser* const remote = static_cast<RemoteBandUser*>(user);
    if (remote->mSessionFlags[2]) {
        return kSlotConnectionDisconnecting;
    }
    if (remote->mSessionFlags[1]) {
        return kSlotConnectionIdle;
    }
    return remote->mSessionFlags[0] ? kSlotConnectionHighLatency : kSlotConnectionConnected;
}

// Reconstructed from eboot.elf at 0xC1CD00. Only in a SOMP session: the
// pending states of a slot outside the online slots, the shown message, a
// remote user's connection, and the joined view during a layout change.
Symbol RBOvershellSlotCom::GetForcedView() const {
    if (!IsSomp()) {
        return Symbol();
    }
    static Symbol pendingOnline;
    if (pendingOnline == Symbol()) {
        pendingOnline = Symbol("somp_pending_online");
    }
    static Symbol pendingOffline;
    if (pendingOffline == Symbol()) {
        pendingOffline = Symbol("somp_pending_offline");
    }
    const OvershellOnlineState state = GetOvershellOnlineState();
    RBOvershellCom* const overshell = mRuntime.mOvershell;
    if (overshell != nullptr) {
        const bool online = !overshell->IsNotOnlineSlot(mRuntime.mBandSlot);
        if (state == kOvershellOnlineStatePendingOnline && !online) {
            return pendingOnline;
        }
        if (state == kOvershellOnlineStatePendingOffline && !online) {
            return pendingOffline;
        }
    }
    static Symbol disconnectedMessage;
    if (disconnectedMessage == Symbol()) {
        disconnectedMessage = Symbol("somp_disconnected_message");
    }
    if (mRuntime.mShowingMessage) {
        return disconnectedMessage;
    }
    if (state != kOvershellOnlineStateOnline) {
        return Symbol();
    }
    static Symbol inactive;
    if (inactive == Symbol()) {
        inactive = Symbol("somp_inactive");
    }
    static Symbol highLatency;
    if (highLatency == Symbol()) {
        highLatency = Symbol("somp_high_latency");
    }
    static Symbol disconnecting;
    if (disconnecting == Symbol()) {
        disconnecting = Symbol("somp_disconnecting");
    }
    // The views of the idle, high latency and disconnecting states (the
    // table at 0x1972440).
    static const Symbol* const kConnectionViews[] = {&inactive, &highLatency, &disconnecting};
    const unsigned int connection = static_cast<unsigned int>(GetConnectionState()) - 1;
    if (connection < 3) {
        return *kConnectionViews[connection];
    }
    static Symbol joinedDefault;
    if (joinedDefault == Symbol()) {
        joinedDefault = Symbol("joined_default");
    }
    if (GetUser() != nullptr && overshell != nullptr && overshell->IsInSompLayoutTransition()) {
        return joinedDefault;
    }
    return Symbol();
}

// Reconstructed from eboot.elf at 0xC1D130.
bool RBOvershellSlotCom::HasMessage() const {
    return mRuntime.mShowingMessage || !mRuntime.mMessageQueue.empty();
}

// Reconstructed from eboot.elf at 0xC1D150.
float RBOvershellSlotCom::GetMessageEndMs() const {
    return mRuntime.mMessageEndMs;
}

// Reconstructed from eboot.elf at 0xC1D160.
void RBOvershellSlotCom::_ShowNextMessage() {
    if (mRuntime.mShowingMessage || mRuntime.mMessageQueue.empty()) {
        return;
    }
    const QueuedMessage message = mRuntime.mMessageQueue.front();
    mRuntime.mMessageQueue.pop_front();
    mRuntime.mShowingMessage = true;
    mRuntime.mMessageEndMs = RealTimeMs() + kMessageShowMs;
    mRuntime.mMessageToken = message.mToken;
    if (message.mLiteral) {
        _SetConfirmActionText(message.mToken.Str());
    } else {
        _SetConfirmActionToken(message.mToken);
    }
    mRuntime.mMessageChanged = true;
}

// Reconstructed from eboot.elf at 0xC1D2B0. The text and the token are
// cleared first.
void RBOvershellSlotCom::_SetConfirmActionText(const char* text) {
    const Symbol none;
    mRuntime.mConfirmActionTextCom->SetTextToken(none);
    mRuntime.mConfirmActionTextCom->SetTextUtf8(none.Str());
    mRuntime.mConfirmActionTextCom->SetTextUtf8(text);
}

// Reconstructed from eboot.elf at 0xC1D300.
void RBOvershellSlotCom::_SetConfirmActionToken(Symbol token) {
    const Symbol none;
    mRuntime.mConfirmActionTextCom->SetTextToken(none);
    mRuntime.mConfirmActionTextCom->SetTextUtf8(none.Str());
    mRuntime.mConfirmActionTextCom->SetTextToken(token);
}

// Reconstructed from eboot.elf at 0xC1D350.
void RBOvershellSlotCom::BeginGoingOnline() {
    mRuntime.mSompTransitionDone = false;
}

// Reconstructed from eboot.elf at 0xC1D360.
void RBOvershellSlotCom::FinishGoingOnline() {
    if (!mRuntime.mOvershell->IsSlotOnline(mRuntime.mBandSlot)) {
        BandUser* const user = GetUser();
        if (user != nullptr && static_cast<RBBandUser*>(user)->IsLocal()) {
            AttemptRemoveUser();
        }
        ShowState(kState_MirrorRemoteUser);
    }
    _QueueSompUpdate();
}

// Reconstructed from eboot.elf at 0xC1D3E0.
void RBOvershellSlotCom::BeginGoingOffline() {
    mRuntime.mSompTransitionDone = false;
}

// Reconstructed from eboot.elf at 0xC1D3F0.
void RBOvershellSlotCom::FinishGoingOffline() {
    BandUser* const user = GetUser();
    if (user != nullptr && !static_cast<RBBandUser*>(user)->IsLocal()) {
        AttemptRemoveUser();
    }
    if (mRuntime.mState->GetStateID() == kState_MirrorRemoteUser) {
        LeaveOptions(true);
    }
}

// Reconstructed from eboot.elf at 0xC1D4B0.
bool RBOvershellSlotCom::IsOnlineTransitionDone() const {
    const RBOvershellCom* const overshell = mRuntime.mOvershell;
    if (overshell != nullptr && !overshell->IsPendingOffline() && !overshell->IsPendingOnline()) {
        return true;
    }
    return mRuntime.mSompTransitionDone;
}

// Reconstructed from eboot.elf at 0xC1D4F0.
bool RBOvershellSlotCom::_CanHandleMsg(BandUser* user, DataNode& result) const {
    if (mRuntime.mOvershell == nullptr) {
        return false;
    }
    const RBOvershellSlotCom* const slot = mRuntime.mOvershell->GetSlotFromUser(user);
    if (slot == this) {
        return mRuntime.mActiveStatus != kOvershellInactive && !mRuntime.mInputDisabled;
    }
    if (slot == nullptr) {
        result = DataNode(0);
    }
    return false;
}

// Reconstructed from eboot.elf at 0xC1D630. A press the state leaves
// unhandled stays unhandled only while the state lets the input through
// to the shell.
DataNode RBOvershellSlotCom::OnMsg(const UIButtonDownMsg& msg) {
    DataNode result;
    if (!_CanHandleMsg(static_cast<BandUser*>(msg.GetUser()), result)) {
        return result;
    }
    mRuntime.mLastInputMs = RealTimeMs();
    static Symbol join;
    if (join == Symbol()) {
        join = Symbol("join");
    }
    if (mRuntime.mState->GetView() == join) {
        static Symbol buttonPulse;
        if (buttonPulse == Symbol()) {
            buttonPulse = Symbol("button_pulse");
        }
        const Message<0> pulse(buttonPulse);
        mObject->ExportToSiblings(this, pulse.mData);
    }
    DataNode handled = mRuntime.mState->HandleMsg(msg.mData);
    if (handled.Type() == kDataUnhandled) {
        if (msg.GetAction() == kActionCancel) {
            static Symbol onCancel;
            if (onCancel == Symbol()) {
                onCancel = Symbol("on_cancel");
            }
            const Message<0> cancel(onCancel);
            handled = mRuntime.mState->HandleMsg(cancel.mData);
        } else if (msg.GetAction() == kActionStart) {
            static Symbol onStart;
            if (onStart == Symbol()) {
                onStart = Symbol("on_start");
            }
            const Message<1> start(onStart);
            DataNode user;
            user.mType = kDataObject;
            user.mValue.object = static_cast<MsgSink*>(msg.GetUser());
            start.mData->Node(2) = user;
            handled = mRuntime.mState->HandleMsg(start.mData);
        } else if (msg.GetAction() == kActionViewModify) {
            static Symbol onViewModify;
            if (onViewModify == Symbol()) {
                onViewModify = Symbol("on_view_modify");
            }
            const Message<0> viewModify(onViewModify);
            handled = mRuntime.mState->HandleMsg(viewModify.mData);
        }
    }
    if (mRuntime.mState->AllowsInputToShell() && handled.Type() == kDataUnhandled) {
        return DataNode();
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xC1DF40.
DataNode RBOvershellSlotCom::OnMsg(const UIButtonUpMsg& msg) {
    DataNode result;
    if (!_CanHandleMsg(static_cast<BandUser*>(msg.GetUser()), result)) {
        return result;
    }
    mRuntime.mLastInputMs = RealTimeMs();
    if (mRuntime.mState->AllowsInputToShell()) {
        return DataNode();
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xC1E110. The selection's fields are read
// for the release build's stripped checks.
DataNode RBOvershellSlotCom::OnMsg(const UISelectMsg& msg) {
    static_cast<void>(msg.mData->Sym(3));
    static_cast<void>(msg.GetSelectedObject());
    DataNode result = mRuntime.mState->HandleMsg(msg.mData);
    mRuntime.mLastInputMs = RealTimeMs();
    _QueueSompUpdate();
    return result;
}

// Reconstructed from eboot.elf at 0xC1E1D0.
void RBOvershellSlotCom::_OnListFocus(DataArray* msg) {
    static_cast<void>(mRuntime.mState->HandleMsg(msg));
    mRuntime.mLastInputMs = RealTimeMs();
    _QueueSompUpdate();
}

// Reconstructed from eboot.elf at 0xC1E2D0.
void RBOvershellSlotCom::_OnSetupConfirmActions(DataArray* msg) {
    _SetConfirmActionToken(msg->Sym(2));
    static_cast<void>(mRuntime.mListProvider->Handle(msg, false));
}

// Reconstructed from eboot.elf at 0xC1E3B0.
void RBOvershellSlotCom::_OnSetupTooltip(DataArray* msg) {
    mRuntime.mTooltipButton = static_cast<JoypadAction>(msg->Int(2));
    mRuntime.mTooltipToken = msg->Sym(3);
}

// Reconstructed from eboot.elf at 0xC1E3F0. The profile must be one the
// profile manager knows for the pad. The event's symbol is built for the
// release build's stripped checks.
void RBOvershellSlotCom::_OnAccountPickerResult(int padNum, bool cancelled) {
    mRuntime.mAccountPickerUser = nullptr;
    static Symbol accountPickerResult;
    if (accountPickerResult == Symbol()) {
        accountPickerResult = Symbol("account_picker_result");
    }
    if (mRuntime.mAccountPickerSink.mSource != nullptr) {
        mRuntime.mAccountPickerSink.mSource->RemoveSink(&mRuntime.mAccountPickerSink);
    }
    BandUser* const user = GetUser();
    if (padNum == -1 || user == nullptr || cancelled) {
        return;
    }
    Profile* profile = nullptr;
    for (Profile* const candidate : theRBProfileMgr->mProfiles) {
        if (candidate->mPadNum == padNum) {
            profile = candidate;
            break;
        }
    }
    if (profile != nullptr) {
        RemoveProfileFromSlot();
        AddProfileToSlot(padNum);
    }
}

// Reconstructed from eboot.elf at 0xC1E540.
void RBOvershellSlotCom::_OnAccountPickerJoinUser(int padNum, bool cancelled) {
    _JoinUser(mRuntime.mAccountPickerUser);
    _OnAccountPickerResult(padNum, cancelled);
    if (mRuntime.mOvershell != nullptr) {
        mRuntime.mOvershell->QueueUpdateAll();
    }
    UpdateState();
    UpdateView();
    _QueueSompUpdate();
}

// Reconstructed from eboot.elf at 0xC1E5A0.
void RBOvershellSlotCom::_ExportStartMsg() {
    const Message<0> start(gOnStart);
    mObject->mEntity->Export(start.mData);
}

// Reconstructed from eboot.elf at 0xC1E610.
void RBOvershellSlotCom::_ExportSelectDisabledMsg() {
    const Message<0> selectDisabled(gOnSelectDisabled);
    mObject->mEntity->Export(selectDisabled.mData);
}

// Reconstructed from eboot.elf at 0xC1E680. The UI messages go to their
// OnMsg handlers (OnMsg(UISelectMsg const&) inlined); every other message
// or script call is answered here, and a message no case handles goes to
// the slot's list provider. Script calls return 0 unless they answer a
// value.
DataNode RBOvershellSlotCom::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    if (type == UIButtonDownMsg::Event()) {
        DataNode result = OnMsg(UIButtonDownMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    if (type == UIButtonUpMsg::Event()) {
        DataNode result = OnMsg(UIButtonUpMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    if (type == UISelectMsg::Event()) {
        DataNode result = OnMsg(UISelectMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol listFocus;
    if (listFocus == Symbol()) {
        listFocus = Symbol("list_focus");
    }
    if (type == listFocus) {
        _OnListFocus(msg);
        return DataNode(0);
    }
    static Symbol onAccountPickerResult;
    if (onAccountPickerResult == Symbol()) {
        onAccountPickerResult = Symbol("on_account_picker_result");
    }
    if (type == onAccountPickerResult) {
        _OnAccountPickerResult(msg->Node(2).LiteralInt(msg), msg->Node(3).LiteralInt(msg) != 0);
        return DataNode(0);
    }
    static Symbol onAccountPickerResultJoinUser;
    if (onAccountPickerResultJoinUser == Symbol()) {
        onAccountPickerResultJoinUser = Symbol("on_account_picker_result_join_user");
    }
    if (type == onAccountPickerResultJoinUser) {
        _OnAccountPickerJoinUser(msg->Node(2).LiteralInt(msg), msg->Node(3).LiteralInt(msg) != 0);
        return DataNode(0);
    }
    static Symbol getUser;
    if (getUser == Symbol()) {
        getUser = Symbol("get_user");
    }
    if (type == getUser) {
        return ObjectNode(GetUser());
    }
    static Symbol getUiList;
    if (getUiList == Symbol()) {
        getUiList = Symbol("get_ui_list");
    }
    if (type == getUiList) {
        return ObjectNode(mRuntime.mUIList);
    }
    static Symbol inSong;
    if (inSong == Symbol()) {
        inSong = Symbol("in_song");
    }
    if (type == inSong) {
        return DataNode(static_cast<int>(mRuntime.mActiveStatus == kOvershellInSong));
    }
    static Symbol inVenueMetaMode;
    if (inVenueMetaMode == Symbol()) {
        inVenueMetaMode = Symbol("in_venue_meta_mode");
    }
    if (type == inVenueMetaMode) {
        return DataNode(static_cast<int>(mRuntime.mOvershell->mRuntime.mInVenueMetaMode));
    }
    static Symbol managedCharName;
    if (managedCharName == Symbol()) {
        managedCharName = Symbol("managed_char_name");
    }
    if (type == managedCharName) {
        if (!mRuntime.mCharManagementSession) {
            static_cast<void>(MakeErrorName());
            return DataNode(Symbol(""));
        }
        return DataNode(mRuntime.mManagedCharName);
    }
    static Symbol getNumMicsConnected;
    if (getNumMicsConnected == Symbol()) {
        getNumMicsConnected = Symbol("get_num_mics_connected");
    }
    if (type == getNumMicsConnected) {
        return DataNode(theRBMetaState.mNumMicsConnected);
    }
    static Symbol getCurrentOvershellQuitToken;
    if (getCurrentOvershellQuitToken == Symbol()) {
        getCurrentOvershellQuitToken = Symbol("get_current_overshell_quit_token");
    }
    if (type == getCurrentOvershellQuitToken) {
        return DataNode(theGameData.GetCurrentOvershellQuitToken());
    }
    static Symbol getCurrentOvershellSkipToken;
    if (getCurrentOvershellSkipToken == Symbol()) {
        getCurrentOvershellSkipToken = Symbol("get_current_overshell_skip_token");
    }
    if (type == getCurrentOvershellSkipToken) {
        return DataNode(theGameData.GetCurrentOvershellSkipToken());
    }
    static Symbol showRestartPrompt;
    if (showRestartPrompt == Symbol()) {
        showRestartPrompt = Symbol("show_restart_prompt");
    }
    if (type == showRestartPrompt) {
        return DataNode(static_cast<int>(theGameData.GetGameMode() != kGameModePractice));
    }
    static Symbol allowContinueAfterDifficultyChange;
    if (allowContinueAfterDifficultyChange == Symbol()) {
        allowContinueAfterDifficultyChange = Symbol("allow_continue_after_difficulty_change");
    }
    if (type == allowContinueAfterDifficultyChange) {
        return DataNode(static_cast<int>(theGameData.GetGameMode() != kGameModeBTM));
    }
    static Symbol getCurrentDifficultyChangePromptToken;
    if (getCurrentDifficultyChangePromptToken == Symbol()) {
        getCurrentDifficultyChangePromptToken = Symbol("get_current_difficulty_change_prompt_token");
    }
    if (type == getCurrentDifficultyChangePromptToken) {
        return DataNode(_GetDifficultyChangePromptToken());
    }
    static Symbol setupDifficultyChangeConfirmMenu;
    if (setupDifficultyChangeConfirmMenu == Symbol()) {
        setupDifficultyChangeConfirmMenu = Symbol("setup_difficulty_change_confirm_menu");
    }
    if (type == setupDifficultyChangeConfirmMenu) {
        _SetupDifficultyChangeConfirmMenu();
        return DataNode(0);
    }
    static Symbol setupSoloToggleConfirmMenu;
    if (setupSoloToggleConfirmMenu == Symbol()) {
        setupSoloToggleConfirmMenu = Symbol("setup_solo_toggle_confirm_menu");
    }
    if (type == setupSoloToggleConfirmMenu) {
        _SetupSoloToggleConfirmMenu();
        return DataNode(0);
    }
    static Symbol setupCymbalToggleConfirmMenu;
    if (setupCymbalToggleConfirmMenu == Symbol()) {
        setupCymbalToggleConfirmMenu = Symbol("setup_cymbal_toggle_confirm_menu");
    }
    if (type == setupCymbalToggleConfirmMenu) {
        _SetupCymbalToggleConfirmMenu();
        return DataNode(0);
    }
    static Symbol isSomp;
    if (isSomp == Symbol()) {
        isSomp = Symbol("is_somp");
    }
    if (type == isSomp) {
        return DataNode(static_cast<int>(IsSomp()));
    }
    static Symbol getJoinToken;
    if (getJoinToken == Symbol()) {
        getJoinToken = Symbol("get_join_token");
    }
    if (type == getJoinToken) {
        return DataNode(_GetJoinToken());
    }
    static Symbol getAutoListView;
    if (getAutoListView == Symbol()) {
        getAutoListView = Symbol("get_auto_list_view");
    }
    if (type == getAutoListView) {
        return DataNode(_GetListView());
    }
    static Symbol getAutoConfirmView;
    if (getAutoConfirmView == Symbol()) {
        getAutoConfirmView = Symbol("get_auto_confirm_view");
    }
    if (type == getAutoConfirmView) {
        return DataNode(_GetConfirmActionView());
    }
    static Symbol attemptRemoveUser;
    if (attemptRemoveUser == Symbol()) {
        attemptRemoveUser = Symbol("attempt_remove_user");
    }
    if (type == attemptRemoveUser) {
        _DropOutUser();
        return DataNode(0);
    }
    static Symbol addProfileToSlot;
    if (addProfileToSlot == Symbol()) {
        addProfileToSlot = Symbol("add_profile_to_slot");
    }
    if (type == addProfileToSlot) {
        AddProfileToSlot(msg->Int(2));
        LeaveOptions(true);
        return DataNode(0);
    }
    static Symbol changeProfile;
    if (changeProfile == Symbol()) {
        changeProfile = Symbol("change_profile");
    }
    if (type == changeProfile) {
        _ChangeProfile();
        return DataNode(0);
    }
    static Symbol cancelChangeProfile;
    if (cancelChangeProfile == Symbol()) {
        cancelChangeProfile = Symbol("cancel_change_profile");
    }
    if (type == cancelChangeProfile) {
        _CancelChangeProfile();
        return DataNode(0);
    }
    static Symbol showState;
    if (showState == Symbol()) {
        showState = Symbol("show_state");
    }
    if (type == showState) {
        ShowState(static_cast<OvershellSlotStateID>(msg->Int(2)));
        return DataNode(0);
    }
    static Symbol beginOverrideFlow;
    if (beginOverrideFlow == Symbol()) {
        beginOverrideFlow = Symbol("begin_override_flow");
    }
    if (type == beginOverrideFlow) {
        BeginOverrideFlow(static_cast<OvershellOverrideFlow>(msg->Int(2)), true);
        return DataNode(0);
    }
    static Symbol endOverrideFlow;
    if (endOverrideFlow == Symbol()) {
        endOverrideFlow = Symbol("end_override_flow");
    }
    if (type == endOverrideFlow) {
        EndOverrideFlow(static_cast<OvershellOverrideFlow>(msg->Int(2)), true);
        return DataNode(0);
    }
    static Symbol setupConfirmActions;
    if (setupConfirmActions == Symbol()) {
        setupConfirmActions = Symbol("setup_confirm_actions");
    }
    if (type == setupConfirmActions) {
        _OnSetupConfirmActions(msg);
        return DataNode(0);
    }
    static Symbol setupTooltip;
    if (setupTooltip == Symbol()) {
        setupTooltip = Symbol("setup_tooltip");
    }
    if (type == setupTooltip) {
        _OnSetupTooltip(msg);
        return DataNode(0);
    }
    static Symbol showTooltip;
    if (showTooltip == Symbol()) {
        showTooltip = Symbol("show_tooltip");
    }
    if (type == showTooltip) {
        _SetTooltipShowing(true);
        return DataNode(0);
    }
    static Symbol hideTooltip;
    if (hideTooltip == Symbol()) {
        hideTooltip = Symbol("hide_tooltip");
    }
    if (type == hideTooltip) {
        _SetTooltipShowing(false);
        return DataNode(0);
    }
    static Symbol setPreviewTrackType;
    if (setPreviewTrackType == Symbol()) {
        setPreviewTrackType = Symbol("set_preview_track_type");
    }
    if (type == setPreviewTrackType) {
        _SetPreviewTrackType(static_cast<TrackType>(msg->Int(2)));
        return DataNode(0);
    }
    static Symbol updateVocalPartIcon;
    if (updateVocalPartIcon == Symbol()) {
        updateVocalPartIcon = Symbol("update_vocal_part_icon");
    }
    if (type == updateVocalPartIcon) {
        _UpdateVocalPartIcon(static_cast<ScoreType>(msg->Int(2)));
        return DataNode(0);
    }
    static Symbol updateVocalFreestyleIcon;
    if (updateVocalFreestyleIcon == Symbol()) {
        updateVocalFreestyleIcon = Symbol("update_vocal_freestyle_icon");
    }
    if (type == updateVocalFreestyleIcon) {
        mRuntime.mVocalFreestyleIconOverride = _ShouldShowVocalFreestyleIcon(static_cast<Difficulty>(msg->Int(2)));
        return DataNode(0);
    }
    static Symbol updateDrumPartIcon;
    if (updateDrumPartIcon == Symbol()) {
        updateDrumPartIcon = Symbol("update_drum_part_icon");
    }
    if (type == updateDrumPartIcon) {
        mRuntime.mProDrumsIconOverride = msg->Int(2) == kScoreRealDrum;
        return DataNode(0);
    }
    static Symbol leaveOptions;
    if (leaveOptions == Symbol()) {
        leaveOptions = Symbol("leave_options");
    }
    if (type == leaveOptions) {
        LeaveOptions(true);
        return DataNode(0);
    }
    static Symbol showInSongMenu;
    if (showInSongMenu == Symbol()) {
        showInSongMenu = Symbol("show_in_song_menu");
    }
    if (type == showInSongMenu) {
        ShowState(kState_InSongMenu);
        if (mRuntime.mPreviousState->GetStateID() == kState_JoinedDefault && !mRuntime.mOvershell->CheckAndAutoPauseGame()) {
            ShowState(kState_JoinedDefault);
        }
        return DataNode(0);
    }
    static Symbol showInstrumentOptions;
    if (showInstrumentOptions == Symbol()) {
        showInstrumentOptions = Symbol("show_instrument_options");
    }
    if (type == showInstrumentOptions) {
        _ShowInstrumentOptions();
        return DataNode(0);
    }
    static Symbol showChooseChar;
    if (showChooseChar == Symbol()) {
        showChooseChar = Symbol("show_choose_char");
    }
    if (type == showChooseChar) {
        ShowState(kState_ChooseCharacter);
        return DataNode(0);
    }
    static Symbol showManageChar;
    if (showManageChar == Symbol()) {
        showManageChar = Symbol("show_manage_char");
    }
    if (type == showManageChar) {
        ShowState(kState_CharacterManagement);
        return DataNode(0);
    }
    static Symbol showDeleteCharConfirm;
    if (showDeleteCharConfirm == Symbol()) {
        showDeleteCharConfirm = Symbol("show_delete_char_confirm");
    }
    if (type == showDeleteCharConfirm) {
        ShowState(kState_CharacterDeleteConfirm);
        return DataNode(0);
    }
    static Symbol showCreateCharDenialNoRoom;
    if (showCreateCharDenialNoRoom == Symbol()) {
        showCreateCharDenialNoRoom = Symbol("show_create_char_denial_no_room");
    }
    if (type == showCreateCharDenialNoRoom) {
        ShowState(kState_CreateCharacterDenialNoRoom);
        return DataNode(0);
    }
    static Symbol showCreateCharDenialError;
    if (showCreateCharDenialError == Symbol()) {
        showCreateCharDenialError = Symbol("show_create_char_denial_error");
    }
    if (type == showCreateCharDenialError) {
        ShowState(kState_CharacterError);
        return DataNode(0);
    }
    static Symbol showSongOptions;
    if (showSongOptions == Symbol()) {
        showSongOptions = Symbol("show_song_options");
    }
    if (type == showSongOptions) {
        ShowSongOptions();
        return DataNode(0);
    }
    static Symbol showChoosePartWait;
    if (showChoosePartWait == Symbol()) {
        showChoosePartWait = Symbol("show_choose_part_wait");
    }
    if (type == showChoosePartWait) {
        _ShowChoosePartWait();
        return DataNode(0);
    }
    static Symbol checkIfIdleMaster;
    if (checkIfIdleMaster == Symbol()) {
        checkIfIdleMaster = Symbol("check_if_idle_master");
    }
    if (type == checkIfIdleMaster) {
        _CheckIfIdleMaster();
        return DataNode(0);
    }
    static Symbol setupCustomStateList;
    if (setupCustomStateList == Symbol()) {
        setupCustomStateList = Symbol("setup_custom_state_list");
    }
    if (type == setupCustomStateList) {
        _SetupCustomStateList();
        return DataNode(0);
    }
    static Symbol showOnlineSettings;
    if (showOnlineSettings == Symbol()) {
        showOnlineSettings = Symbol("show_online_settings");
    }
    if (type == showOnlineSettings) {
        mRuntime.mOnlineSettingsReturnState = mRuntime.mState->GetStateID();
        ShowState(kState_OnlineSettings);
        return DataNode(0);
    }
    static Symbol leaveOnlineSettings;
    if (leaveOnlineSettings == Symbol()) {
        leaveOnlineSettings = Symbol("leave_online_settings");
    }
    if (type == leaveOnlineSettings) {
        const OvershellSlotStateID state = mRuntime.mOnlineSettingsReturnState;
        if (state == kState_JoinedDefault) {
            ShowState(kState_OnlineError);
        } else {
            mRuntime.mOnlineSettingsReturnState = kState_JoinedDefault;
            ShowState(state);
        }
        return DataNode(0);
    }
    static Symbol backToOnlineSettings;
    if (backToOnlineSettings == Symbol()) {
        backToOnlineSettings = Symbol("back_to_online_settings");
    }
    if (type == backToOnlineSettings) {
        ShowState(kState_OnlineSettings);
        return DataNode(0);
    }
    static Symbol setupSongOptionsCancel;
    if (setupSongOptionsCancel == Symbol()) {
        setupSongOptionsCancel = Symbol("setup_song_options_cancel");
    }
    if (type == setupSongOptionsCancel) {
        _SetupSongOptionsCancelMenu();
        return DataNode(0);
    }
    static Symbol showSompQuitSessionConfirm;
    if (showSompQuitSessionConfirm == Symbol()) {
        showSompQuitSessionConfirm = Symbol("show_somp_quit_session_confirm");
    }
    if (type == showSompQuitSessionConfirm) {
        mRuntime.mOnlineReturnState = mRuntime.mState->GetStateID();
        ShowState(kState_OnlineQuitSessionConfirm);
        return DataNode(0);
    }
    static Symbol showSompReturnToGreenRoomConfirm;
    if (showSompReturnToGreenRoomConfirm == Symbol()) {
        showSompReturnToGreenRoomConfirm = Symbol("show_somp_return_to_green_room_confirm");
    }
    if (type == showSompReturnToGreenRoomConfirm) {
        mRuntime.mOnlineReturnState = mRuntime.mState->GetStateID();
        ShowState(kState_OnlineLeaderReturnToGreenRoomConfirm);
        return DataNode(0);
    }
    static Symbol showSompReturnToMusicLibraryConfirm;
    if (showSompReturnToMusicLibraryConfirm == Symbol()) {
        showSompReturnToMusicLibraryConfirm = Symbol("show_somp_return_to_music_library_confirm");
    }
    if (type == showSompReturnToMusicLibraryConfirm) {
        mRuntime.mOnlineReturnState = mRuntime.mState->GetStateID();
        ShowState(kState_OnlineLeaderReturnToMusicLibraryConfirm);
        return DataNode(0);
    }
    static Symbol sompCancelTransition;
    if (sompCancelTransition == Symbol()) {
        sompCancelTransition = Symbol("somp_cancel_transition");
    }
    if (type == sompCancelTransition) {
        const OvershellSlotStateID state = mRuntime.mOnlineReturnState;
        mRuntime.mOnlineReturnState = kState_JoinedDefault;
        ShowState(state);
        return DataNode(0);
    }
    static Symbol showOnlineExitSubmenu;
    if (showOnlineExitSubmenu == Symbol()) {
        showOnlineExitSubmenu = Symbol("show_online_exit_submenu");
    }
    if (type == showOnlineExitSubmenu) {
        mRuntime.mExitSubmenuReturnState = mRuntime.mState->GetStateID();
        ShowState(kState_OnlineExitSubmenu);
        return DataNode(0);
    }
    static Symbol leaveOnlineExitSubmenu;
    if (leaveOnlineExitSubmenu == Symbol()) {
        leaveOnlineExitSubmenu = Symbol("leave_online_exit_submenu");
    }
    if (type == leaveOnlineExitSubmenu) {
        const OvershellSlotStateID state = mRuntime.mExitSubmenuReturnState;
        if (state == kState_JoinedDefault) {
            ShowState(kState_OnlineError);
        } else {
            mRuntime.mExitSubmenuReturnState = kState_JoinedDefault;
            ShowState(state);
        }
        return DataNode(0);
    }
    static Symbol showTrackTextureMenu;
    if (showTrackTextureMenu == Symbol()) {
        showTrackTextureMenu = Symbol("show_track_texture_menu");
    }
    if (type == showTrackTextureMenu) {
        _ShowTrackTextureMenu();
        return DataNode(0);
    }
    static Symbol selectPart;
    if (selectPart == Symbol()) {
        selectPart = Symbol("select_part");
    }
    if (type == selectPart) {
        _SelectPart(static_cast<TrackType>(msg->Int(2)));
        return DataNode(0);
    }
    static Symbol selectVocalPart;
    if (selectVocalPart == Symbol()) {
        selectVocalPart = Symbol("select_vocal_part");
    }
    if (type == selectVocalPart) {
        _SelectVocalPart(msg->Int(2) != 0);
        return DataNode(0);
    }
    static Symbol selectDrumPart;
    if (selectDrumPart == Symbol()) {
        selectDrumPart = Symbol("select_drum_part");
    }
    if (type == selectDrumPart) {
        _SelectDrumPart(msg->Int(2) != 0);
        return DataNode(0);
    }
    static Symbol selectDifficulty;
    if (selectDifficulty == Symbol()) {
        selectDifficulty = Symbol("select_difficulty");
    }
    if (type == selectDifficulty) {
        _SelectDifficulty(static_cast<Difficulty>(msg->Int(2)));
        return DataNode(0);
    }
    static Symbol leaveChoosePart;
    if (leaveChoosePart == Symbol()) {
        leaveChoosePart = Symbol("leave_choose_part");
    }
    if (type == leaveChoosePart) {
        _LeaveChoosePart();
        return DataNode(0);
    }
    static Symbol leaveChoosePartWait;
    if (leaveChoosePartWait == Symbol()) {
        leaveChoosePartWait = Symbol("leave_choose_part_wait");
    }
    if (type == leaveChoosePartWait) {
        _LeaveChoosePartWait();
        return DataNode(0);
    }
    static Symbol leaveChooseDifficulty;
    if (leaveChooseDifficulty == Symbol()) {
        leaveChooseDifficulty = Symbol("leave_choose_difficulty");
    }
    if (type == leaveChooseDifficulty) {
        _LeaveChooseDifficulty();
        return DataNode(0);
    }
    static Symbol confirmChooseDiff;
    if (confirmChooseDiff == Symbol()) {
        confirmChooseDiff = Symbol("confirm_choose_diff");
    }
    if (type == confirmChooseDiff) {
        _ConfirmChooseDiff();
        return DataNode(0);
    }
    static Symbol clearSompPart;
    if (clearSompPart == Symbol()) {
        clearSompPart = Symbol("clear_somp_part");
    }
    if (type == clearSompPart) {
        _ClearSompPart();
        return DataNode(0);
    }
    static Symbol clearSompDifficulty;
    if (clearSompDifficulty == Symbol()) {
        clearSompDifficulty = Symbol("clear_somp_difficulty");
    }
    if (type == clearSompDifficulty) {
        _SendSompChoosingDifficulty();
        return DataNode(0);
    }
    static Symbol toggleLeftyFlip;
    if (toggleLeftyFlip == Symbol()) {
        toggleLeftyFlip = Symbol("toggle_lefty_flip");
    }
    if (type == toggleLeftyFlip) {
        _ToggleLeftyFlip();
        return DataNode(0);
    }
    static Symbol cycleDrumFillsForward;
    if (cycleDrumFillsForward == Symbol()) {
        cycleDrumFillsForward = Symbol("cycle_drum_fills_forward");
    }
    if (type == cycleDrumFillsForward) {
        _CycleDrumFillLogicForward();
        return DataNode(0);
    }
    static Symbol cycleDrumFillsBackward;
    if (cycleDrumFillsBackward == Symbol()) {
        cycleDrumFillsBackward = Symbol("cycle_drum_fills_backward");
    }
    if (type == cycleDrumFillsBackward) {
        _CycleDrumFillLogicBackward();
        return DataNode(0);
    }
    static Symbol tryToggleCymbal;
    if (tryToggleCymbal == Symbol()) {
        tryToggleCymbal = Symbol("try_toggle_cymbal");
    }
    if (type == tryToggleCymbal) {
        _ToggleCymbal(static_cast<unsigned int>(msg->Int(2)));
        return DataNode(0);
    }
    static Symbol confirmToggleCymbal;
    if (confirmToggleCymbal == Symbol()) {
        confirmToggleCymbal = Symbol("confirm_toggle_cymbal");
    }
    if (type == confirmToggleCymbal) {
        _ConfirmToggleCymbal();
        return DataNode(0);
    }
    static Symbol cancelToggleCymbal;
    if (cancelToggleCymbal == Symbol()) {
        cancelToggleCymbal = Symbol("cancel_toggle_cymbal");
    }
    if (type == cancelToggleCymbal) {
        _CancelToggleCymbal();
        return DataNode(0);
    }
    static Symbol finishCymbalSelection;
    if (finishCymbalSelection == Symbol()) {
        finishCymbalSelection = Symbol("finish_cymbal_selection");
    }
    if (type == finishCymbalSelection) {
        _FinishCymbalSelect(msg->Int(2) != 0);
        return DataNode(0);
    }
    static Symbol dismissCymbalMessage;
    if (dismissCymbalMessage == Symbol()) {
        dismissCymbalMessage = Symbol("dismiss_cymbal_message");
    }
    if (type == dismissCymbalMessage) {
        _DismissCymbalMessage();
        return DataNode(0);
    }
    static Symbol toggleGuitarSoloMode;
    if (toggleGuitarSoloMode == Symbol()) {
        toggleGuitarSoloMode = Symbol("toggle_guitar_solo_mode");
    }
    if (type == toggleGuitarSoloMode) {
        _ToggleGuitarSoloMode();
        return DataNode(0);
    }
    static Symbol revertGuitarSoloMode;
    if (revertGuitarSoloMode == Symbol()) {
        revertGuitarSoloMode = Symbol("revert_guitar_solo_mode");
    }
    if (type == revertGuitarSoloMode) {
        static_cast<void>(GetUser());
        GetUser()->GetGameplayOptions()->ToggleGuitarSoloMode();
        ShowState(kState_OptionsGuitar);
        return DataNode(0);
    }
    static Symbol incrementBreakneckSlider;
    if (incrementBreakneckSlider == Symbol()) {
        incrementBreakneckSlider = Symbol("increment_breakneck_slider");
    }
    if (type == incrementBreakneckSlider) {
        _IncrementBreakneckSpeed();
        return DataNode(0);
    }
    static Symbol decrementBreakneckSlider;
    if (decrementBreakneckSlider == Symbol()) {
        decrementBreakneckSlider = Symbol("decrement_breakneck_slider");
    }
    if (type == decrementBreakneckSlider) {
        _DecrementBreakneckSpeed();
        return DataNode(0);
    }
    static Symbol toggleVocalImprov;
    if (toggleVocalImprov == Symbol()) {
        toggleVocalImprov = Symbol("toggle_vocal_improv");
    }
    if (type == toggleVocalImprov) {
        _ToggleVocalImprov();
        return DataNode(0);
    }
    static Symbol activateVolumeSlider;
    if (activateVolumeSlider == Symbol()) {
        activateVolumeSlider = Symbol("activate_volume_slider");
    }
    if (type == activateVolumeSlider) {
        _ActivateVolumeSlider();
        return DataNode(0);
    }
    static Symbol incrementVolumeSlider;
    if (incrementVolumeSlider == Symbol()) {
        incrementVolumeSlider = Symbol("increment_volume_slider");
    }
    if (type == incrementVolumeSlider) {
        _IncrementVolumeSlider();
        return DataNode(0);
    }
    static Symbol decrementVolumeSlider;
    if (decrementVolumeSlider == Symbol()) {
        decrementVolumeSlider = Symbol("decrement_volume_slider");
    }
    if (type == decrementVolumeSlider) {
        _DecrementVolumeSlider();
        return DataNode(0);
    }
    static Symbol deactivateVolumeSlider;
    if (deactivateVolumeSlider == Symbol()) {
        deactivateVolumeSlider = Symbol("deactivate_volume_slider");
    }
    if (type == deactivateVolumeSlider) {
        _DeactivateVolumeSlider();
        return DataNode(0);
    }
    static Symbol activateTextSlider;
    if (activateTextSlider == Symbol()) {
        activateTextSlider = Symbol("activate_text_slider");
    }
    if (type == activateTextSlider) {
        _ActivateTextSlider();
        return DataNode(0);
    }
    static Symbol deactivateTextSlider;
    if (deactivateTextSlider == Symbol()) {
        deactivateTextSlider = Symbol("deactivate_text_slider");
    }
    if (type == deactivateTextSlider) {
        _DeactivateTextSlider();
        return DataNode(0);
    }
    static Symbol toggleNoFail;
    if (toggleNoFail == Symbol()) {
        toggleNoFail = Symbol("toggle_no_fail");
    }
    if (type == toggleNoFail) {
        _ToggleNoFail();
        return DataNode(0);
    }
    static Symbol toggleSompDoNotDisturb;
    if (toggleSompDoNotDisturb == Symbol()) {
        toggleSompDoNotDisturb = Symbol("toggle_somp_do_not_disturb");
    }
    if (type == toggleSompDoNotDisturb) {
        static_cast<void>(GetUser()->GetPlatformUserId());
        static_cast<void>(GetUser()->GetPlatformName());
        return DataNode(0);
    }
    static Symbol toggleMicOdDeploy;
    if (toggleMicOdDeploy == Symbol()) {
        toggleMicOdDeploy = Symbol("toggle_mic_od_deploy");
    }
    if (type == toggleMicOdDeploy) {
        _ToggleMicOverdriveDeploy();
        return DataNode(0);
    }
    static Symbol toggleShowLocalTrackTexture;
    if (toggleShowLocalTrackTexture == Symbol()) {
        toggleShowLocalTrackTexture = Symbol("toggle_show_local_track_texture");
    }
    if (type == toggleShowLocalTrackTexture) {
        _ToggleShowLocalTrackTexture();
        return DataNode(0);
    }
    static Symbol toggleAllowRemoteTrackTextures;
    if (toggleAllowRemoteTrackTextures == Symbol()) {
        toggleAllowRemoteTrackTextures = Symbol("toggle_allow_remote_track_textures");
    }
    if (type == toggleAllowRemoteTrackTextures) {
        _ToggleAllowRemoteTrackTextures();
        return DataNode(0);
    }
    static Symbol toggleRandomTrackTexture;
    if (toggleRandomTrackTexture == Symbol()) {
        toggleRandomTrackTexture = Symbol("toggle_random_track_texture");
    }
    if (type == toggleRandomTrackTexture) {
        _ToggleRandomTrackTexture();
        return DataNode(0);
    }
    static Symbol toggleRandomTrackColor;
    if (toggleRandomTrackColor == Symbol()) {
        toggleRandomTrackColor = Symbol("toggle_random_track_color");
    }
    if (type == toggleRandomTrackColor) {
        _ToggleRandomTrackColor();
        return DataNode(0);
    }
    static Symbol clearVolumeSliderLastState;
    if (clearVolumeSliderLastState == Symbol()) {
        clearVolumeSliderLastState = Symbol("clear_volume_slider_last_state");
    }
    if (type == clearVolumeSliderLastState) {
        mRuntime.mSliderReturnState = kState_JoinedDefault;
        return DataNode(0);
    }
    static Symbol showModifiers;
    if (showModifiers == Symbol()) {
        showModifiers = Symbol("show_modifiers");
    }
    if (type == showModifiers) {
        ShowState(kState_Modifiers);
        return DataNode(0);
    }
    static Symbol showModifiersDrumWarning;
    if (showModifiersDrumWarning == Symbol()) {
        showModifiersDrumWarning = Symbol("show_modifiers_drum_warning");
    }
    if (type == showModifiersDrumWarning) {
        ShowState(kState_ModifiersDrumWarning);
        return DataNode(0);
    }
    static Symbol showModifierUnlock;
    if (showModifierUnlock == Symbol()) {
        showModifierUnlock = Symbol("show_modifier_unlock");
    }
    if (type == showModifierUnlock) {
        ShowState(kState_ModifierUnlock);
        return DataNode(0);
    }
    static Symbol showModifierDelayedEffect;
    if (showModifierDelayedEffect == Symbol()) {
        showModifierDelayedEffect = Symbol("show_modifier_delayed_effect");
    }
    if (type == showModifierDelayedEffect) {
        ShowState(kState_ModifierDelayedEffect);
        return DataNode(0);
    }
    static Symbol showToggleNoFailDenial;
    if (showToggleNoFailDenial == Symbol()) {
        showToggleNoFailDenial = Symbol("show_toggle_no_fail_denial");
    }
    if (type == showToggleNoFailDenial) {
        ShowState(kState_ToggleNoFailDenial);
        return DataNode(0);
    }
    static Symbol isUnlockingModifiers;
    if (isUnlockingModifiers == Symbol()) {
        isUnlockingModifiers = Symbol("is_unlocking_modifiers");
    }
    if (type == isUnlockingModifiers) {
        return DataNode(static_cast<int>(mRuntime.mState->GetStateID() == kState_ModifierUnlock));
    }
    static Symbol togglePause;
    if (togglePause == Symbol()) {
        togglePause = Symbol("toggle_pause");
    }
    if (type == togglePause) {
        _OnTogglePause();
        return DataNode(0);
    }
    static Symbol restartSong;
    if (restartSong == Symbol()) {
        restartSong = Symbol("restart_song");
    }
    if (type == restartSong) {
        _OnRestartSong();
        return DataNode(0);
    }
    static Symbol skipSong;
    if (skipSong == Symbol()) {
        skipSong = Symbol("skip_song");
    }
    if (type == skipSong) {
        _OnSkipSongInShow();
        return DataNode(0);
    }
    static Symbol venueMetaDropOut;
    if (venueMetaDropOut == Symbol()) {
        venueMetaDropOut = Symbol("venue_meta_drop_out");
    }
    if (type == venueMetaDropOut) {
        _OnVenueMetaDropOut();
        return DataNode(0);
    }
    static Symbol attemptQuit;
    if (attemptQuit == Symbol()) {
        attemptQuit = Symbol("attempt_quit");
    }
    if (type == attemptQuit) {
        _OnAttemptQuit();
        return DataNode(0);
    }
    static Symbol quitEarly;
    if (quitEarly == Symbol()) {
        quitEarly = Symbol("quit_early");
    }
    if (type == quitEarly) {
        _QuitEarly(kLayoutInvalid);
        return DataNode(0);
    }
    static Symbol practiceNewSection;
    if (practiceNewSection == Symbol()) {
        practiceNewSection = Symbol("practice_new_section");
    }
    if (type == practiceNewSection) {
        _QuitEarly(kLayoutPracticeSectionSelect);
        return DataNode(0);
    }
    static Symbol btmRestartShow;
    if (btmRestartShow == Symbol()) {
        btmRestartShow = Symbol("btm_restart_show");
    }
    if (type == btmRestartShow) {
        _OnRestartShow();
        return DataNode(0);
    }
    static Symbol goToPlayerStats;
    if (goToPlayerStats == Symbol()) {
        goToPlayerStats = Symbol("go_to_player_stats");
    }
    if (type == goToPlayerStats) {
        _GoToPlayerStats();
        return DataNode(0);
    }
    static Symbol sompLeaderReturnToGreenRoom;
    if (sompLeaderReturnToGreenRoom == Symbol()) {
        sompLeaderReturnToGreenRoom = Symbol("somp_leader_return_to_green_room");
    }
    if (type == sompLeaderReturnToGreenRoom) {
        _ReturnToGreenRoom();
        return DataNode(0);
    }
    static Symbol sompLeaderReturnToMusicLibrary;
    if (sompLeaderReturnToMusicLibrary == Symbol()) {
        sompLeaderReturnToMusicLibrary = Symbol("somp_leader_return_to_music_library");
    }
    if (type == sompLeaderReturnToMusicLibrary) {
        _ReturnToMusicLibrary();
        return DataNode(0);
    }
    static Symbol sompDropOut;
    if (sompDropOut == Symbol()) {
        sompDropOut = Symbol("somp_drop_out");
    }
    if (type == sompDropOut) {
        _QuitSompSong();
        return DataNode(0);
    }
    static Symbol sompQuitSession;
    if (sompQuitSession == Symbol()) {
        sompQuitSession = Symbol("somp_quit_session");
    }
    if (type == sompQuitSession) {
        _DropOutOfSompSession();
        return DataNode(0);
    }
    static Symbol setCharAsProfilePic;
    if (setCharAsProfilePic == Symbol()) {
        setCharAsProfilePic = Symbol("set_char_as_profile_pic");
    }
    if (type == setCharAsProfilePic) {
        _ConfirmCharManagementSession();
        return DataNode(0);
    }
    static Symbol allowManageChar;
    if (allowManageChar == Symbol()) {
        allowManageChar = Symbol("allow_manage_char");
    }
    if (type == allowManageChar) {
        return DataNode(static_cast<int>(!IsSomp()));
    }
    static Symbol startCharManagementSession;
    if (startCharManagementSession == Symbol()) {
        startCharManagementSession = Symbol("start_char_management_session");
    }
    if (type == startCharManagementSession) {
        _StartCharManagementSession(msg->Sym(2), msg->Sym(3), String(msg->Str(4)));
        return DataNode(0);
    }
    static Symbol endCharManagementSession;
    if (endCharManagementSession == Symbol()) {
        endCharManagementSession = Symbol("end_char_management_session");
    }
    if (type == endCharManagementSession) {
        mRuntime.mCharManagementSession = false;
        mRuntime.mManagedCharName = Symbol("");
        mRuntime.mManagedCharType = Symbol("");
        mRuntime.mManagedCharOwner = "";
        return DataNode(0);
    }
    static Symbol confirmCharManagementSession;
    if (confirmCharManagementSession == Symbol()) {
        confirmCharManagementSession = Symbol("confirm_char_management_session");
    }
    if (type == confirmCharManagementSession) {
        if (!mRuntime.mCharManagementSession) {
            static_cast<void>(MakeErrorName());
            ShowState(kState_CharacterError);
        }
        return DataNode(0);
    }
    static Symbol startCharCreateSession;
    if (startCharCreateSession == Symbol()) {
        startCharCreateSession = Symbol("start_char_create_session");
    }
    if (type == startCharCreateSession) {
        _StartCharCreate();
        return DataNode(0);
    }
    static Symbol startCharCreateOtherUserSession;
    if (startCharCreateOtherUserSession == Symbol()) {
        startCharCreateOtherUserSession = Symbol("start_char_create_other_user_session");
    }
    if (type == startCharCreateOtherUserSession) {
        _StartCharCreateOther(String(msg->Str(2)));
        return DataNode(0);
    }
    static Symbol startCharShopSession;
    if (startCharShopSession == Symbol()) {
        startCharShopSession = Symbol("start_char_shop_session");
    }
    if (type == startCharShopSession) {
        _StartRockShop();
        return DataNode(0);
    }
    static Symbol startGenericShopSession;
    if (startGenericShopSession == Symbol()) {
        startGenericShopSession = Symbol("start_generic_shop_session");
    }
    if (type == startGenericShopSession) {
        _StartShopSession(false);
        return DataNode(0);
    }
    static Symbol startTrackTexShopSession;
    if (startTrackTexShopSession == Symbol()) {
        startTrackTexShopSession = Symbol("start_track_tex_shop_session");
    }
    if (type == startTrackTexShopSession) {
        _StartShopSession(true);
        return DataNode(0);
    }
    static Symbol startCharEditSession;
    if (startCharEditSession == Symbol()) {
        startCharEditSession = Symbol("start_char_edit_session");
    }
    if (type == startCharEditSession) {
        _StartCharEdit();
        return DataNode(0);
    }
    static Symbol selectChar;
    if (selectChar == Symbol()) {
        selectChar = Symbol("select_char");
    }
    if (type == selectChar) {
        const String owner(msg->Str(4));
        if (_SetActiveChar(msg->Sym(2), msg->Sym(3), owner)) {
            ShowState(kState_Options);
        } else {
            ShowState(kState_CharacterError);
        }
        return DataNode(0);
    }
    static Symbol deleteChar;
    if (deleteChar == Symbol()) {
        deleteChar = Symbol("delete_char");
    }
    if (type == deleteChar) {
        _DeleteManagementSessionChar();
        return DataNode(0);
    }
    static Symbol selectBandChar;
    if (selectBandChar == Symbol()) {
        selectBandChar = Symbol("select_band_char");
    }
    if (type == selectBandChar) {
        const String owner(msg->Str(4));
        if (_SetActiveChar(msg->Sym(2), msg->Sym(3), owner)) {
            ShowState(kState_ChooseBandMemberReady);
        } else {
            static_cast<void>(MakeErrorName());
        }
        return DataNode(0);
    }
    static Symbol refreshCharList;
    if (refreshCharList == Symbol()) {
        refreshCharList = Symbol("refresh_char_list");
    }
    if (type == refreshCharList) {
        _RefreshCharList();
        return DataNode(0);
    }
    static Symbol activatePreviewSceneIfNeeded;
    if (activatePreviewSceneIfNeeded == Symbol()) {
        activatePreviewSceneIfNeeded = Symbol("activate_preview_scene_if_needed");
    }
    if (type == activatePreviewSceneIfNeeded) {
        _ActivateCharPreviewScene();
        return DataNode(0);
    }
    static Symbol deactivatePreviewSceneIfNeeded;
    if (deactivatePreviewSceneIfNeeded == Symbol()) {
        deactivatePreviewSceneIfNeeded = Symbol("deactivate_preview_scene_if_needed");
    }
    if (type == deactivatePreviewSceneIfNeeded) {
        _DeactivateCharPreviewScene();
        return DataNode(0);
    }
    static Symbol showPreviewChar;
    if (showPreviewChar == Symbol()) {
        showPreviewChar = Symbol("show_preview_char");
    }
    if (type == showPreviewChar) {
        _ShowCharPreview(msg->Sym(2), msg->Sym(3), String(msg->Str(4)));
        return DataNode(0);
    }
    static Symbol showPreviewOfManagedChar;
    if (showPreviewOfManagedChar == Symbol()) {
        showPreviewOfManagedChar = Symbol("show_preview_of_managed_char");
    }
    if (type == showPreviewOfManagedChar) {
        if (mRuntime.mCharManagementSession) {
            _ShowCharPreview(mRuntime.mManagedCharName, mRuntime.mManagedCharType, mRuntime.mManagedCharOwner);
        } else {
            static_cast<void>(MakeErrorName());
        }
        return DataNode(0);
    }
    static Symbol hidePreviewChar;
    if (hidePreviewChar == Symbol()) {
        hidePreviewChar = Symbol("hide_preview_char");
    }
    if (type == hidePreviewChar) {
        _HideCharPreview();
        return DataNode(0);
    }
    static Symbol startSompPlayerManagementSession;
    if (startSompPlayerManagementSession == Symbol()) {
        startSompPlayerManagementSession = Symbol("start_somp_player_management_session");
    }
    if (type == startSompPlayerManagementSession) {
        static_cast<void>(SetKickTarget(msg->Sym(2), msg->Sym(3)));
        return DataNode(0);
    }
    static Symbol confirmSompPlayerManagement;
    if (confirmSompPlayerManagement == Symbol()) {
        confirmSompPlayerManagement = Symbol("confirm_somp_player_management");
    }
    if (type == confirmSompPlayerManagement) {
        static_cast<void>(_HasKickTarget());
        return DataNode(0);
    }
    static Symbol endSompPlayerManagementSession;
    if (endSompPlayerManagementSession == Symbol()) {
        endSompPlayerManagementSession = Symbol("end_somp_player_management_session");
    }
    if (type == endSompPlayerManagementSession) {
        ClearKickTarget();
        return DataNode(0);
    }
    static Symbol showSompKickPlayers;
    if (showSompKickPlayers == Symbol()) {
        showSompKickPlayers = Symbol("show_somp_kick_players");
    }
    if (type == showSompKickPlayers) {
        ShowState(kState_OnlineKickPlayer);
        return DataNode(0);
    }
    static Symbol showKickSompPlayerConfirm;
    if (showKickSompPlayerConfirm == Symbol()) {
        showKickSompPlayerConfirm = Symbol("show_kick_somp_player_confirm");
    }
    if (type == showKickSompPlayerConfirm) {
        ShowState(kState_OnlinePlayerKickConfirm);
        return DataNode(0);
    }
    static Symbol showSompPlayerInfo;
    if (showSompPlayerInfo == Symbol()) {
        showSompPlayerInfo = Symbol("show_somp_player_info");
    }
    if (type == showSompPlayerInfo) {
        static_cast<void>(msg->Sym(2));
        static_cast<void>(msg->Sym(3));
        return DataNode(0);
    }
    static Symbol kickSelectedSompPlayer;
    if (kickSelectedSompPlayer == Symbol()) {
        kickSelectedSompPlayer = Symbol("kick_selected_somp_player");
    }
    if (type == kickSelectedSompPlayer) {
        RBSompClient* const client = theRBSompSessionMgr.mClient;
        if (client == nullptr) {
            ShowState(kState_OnlineError);
        }
        BandUser* const user = GetUser();
        client->RequestKickPlayer(
            user,
            mRuntime.mKickTargetSet ? mRuntime.mKickTargetName : Symbol(),
            mRuntime.mKickTargetSet ? mRuntime.mKickTargetId : Symbol());
        ShowState(kState_OnlinePlayerKickResultWait);
        return DataNode(0);
    }
    static Symbol setupKickSompPlayerConfirmDisplay;
    if (setupKickSompPlayerConfirmDisplay == Symbol()) {
        setupKickSompPlayerConfirmDisplay = Symbol("setup_kick_somp_player_confirm_display");
    }
    if (type == setupKickSompPlayerConfirmDisplay) {
        _ShowKickPlayerConfirmation();
        return DataNode(0);
    }
    static Symbol setupKickSompPlayerResultDisplay;
    if (setupKickSompPlayerResultDisplay == Symbol()) {
        setupKickSompPlayerResultDisplay = Symbol("setup_kick_somp_player_result_display");
    }
    if (type == setupKickSompPlayerResultDisplay) {
        _ShowKickPlayerResult();
        return DataNode(0);
    }
    static Symbol setupSompEmoteCategoryList;
    if (setupSompEmoteCategoryList == Symbol()) {
        setupSompEmoteCategoryList = Symbol("setup_somp_emote_category_list");
    }
    if (type == setupSompEmoteCategoryList) {
        _SetupEmoteCategoryList();
        return DataNode(0);
    }
    static Symbol showSompPickEmoteCategory;
    if (showSompPickEmoteCategory == Symbol()) {
        showSompPickEmoteCategory = Symbol("show_somp_pick_emote_category");
    }
    if (type == showSompPickEmoteCategory) {
        ShowState(kState_OnlinePickEmoteCategory);
        return DataNode(0);
    }
    static Symbol clearSelectedEmoteCategory;
    if (clearSelectedEmoteCategory == Symbol()) {
        clearSelectedEmoteCategory = Symbol("clear_selected_emote_category");
    }
    if (type == clearSelectedEmoteCategory) {
        _ClearEmote();
        return DataNode(0);
    }
    static Symbol handleSelectEmoteCategoryButtonDown;
    if (handleSelectEmoteCategoryButtonDown == Symbol()) {
        handleSelectEmoteCategoryButtonDown = Symbol("handle_select_emote_category_button_down");
    }
    if (type == handleSelectEmoteCategoryButtonDown) {
        return _OnEmoteCategorySelect(msg->Int(2));
    }
    static Symbol handleCancelEmoteCategory;
    if (handleCancelEmoteCategory == Symbol()) {
        handleCancelEmoteCategory = Symbol("handle_cancel_emote_category");
    }
    if (type == handleCancelEmoteCategory) {
        return _OnEmoteCategoryCancel();
    }
    static Symbol showSompPickSpecificEmote;
    if (showSompPickSpecificEmote == Symbol()) {
        showSompPickSpecificEmote = Symbol("show_somp_pick_specific_emote");
    }
    if (type == showSompPickSpecificEmote) {
        ShowState(kState_OnlinePickEmote);
        return DataNode(0);
    }
    static Symbol setupSompEmoteList;
    if (setupSompEmoteList == Symbol()) {
        setupSompEmoteList = Symbol("setup_somp_emote_list");
    }
    if (type == setupSompEmoteList) {
        _SetupEmoteList();
        return DataNode(0);
    }
    static Symbol handleSelectEmoteButtonDown;
    if (handleSelectEmoteButtonDown == Symbol()) {
        handleSelectEmoteButtonDown = Symbol("handle_select_emote_button_down");
    }
    if (type == handleSelectEmoteButtonDown) {
        return _OnEmoteSelect(msg->Int(2));
    }
    static Symbol handleCancelEmote;
    if (handleCancelEmote == Symbol()) {
        handleCancelEmote = Symbol("handle_cancel_emote");
    }
    if (type == handleCancelEmote) {
        return _OnEmoteCancel();
    }
    static Symbol triggerCancelEffects;
    if (triggerCancelEffects == Symbol()) {
        triggerCancelEffects = Symbol("trigger_cancel_effects");
    }
    if (type == triggerCancelEffects) {
        const Message<0> effect(gOnCancel);
        mObject->mEntity->Export(effect.mData);
        return DataNode(0);
    }
    static Symbol triggerStartEffects;
    if (triggerStartEffects == Symbol()) {
        triggerStartEffects = Symbol("trigger_start_effects");
    }
    if (type == triggerStartEffects) {
        const Message<0> effect(gOnStart);
        mObject->mEntity->Export(effect.mData);
        return DataNode(0);
    }
    static Symbol triggerSelectEffects;
    if (triggerSelectEffects == Symbol()) {
        triggerSelectEffects = Symbol("trigger_select_effects");
    }
    if (type == triggerSelectEffects) {
        const Message<0> effect(gOnSelect);
        mObject->mEntity->Export(effect.mData);
        return DataNode(0);
    }
    static Symbol triggerSelectDisabledEffects;
    if (triggerSelectDisabledEffects == Symbol()) {
        triggerSelectDisabledEffects = Symbol("trigger_select_disabled_effects");
    }
    if (type == triggerSelectDisabledEffects) {
        const Message<0> effect(gOnSelectDisabled);
        mObject->mEntity->Export(effect.mData);
        return DataNode(0);
    }
    static Symbol triggerWidgetScrollEffects;
    if (triggerWidgetScrollEffects == Symbol()) {
        triggerWidgetScrollEffects = Symbol("trigger_widget_scroll_effects");
    }
    if (type == triggerWidgetScrollEffects) {
        const Message<0> effect(gOnWidgetScroll);
        mObject->mEntity->Export(effect.mData);
        return DataNode(0);
    }
    if (mRuntime.mListProvider != nullptr) {
        DataNode result = mRuntime.mListProvider->Handle(msg, false);
        if (result != DataNode()) {
            return result;
        }
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0xC24010.
Symbol RBOvershellSlotCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0xC24020.
Symbol RBOvershellSlotCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0xC24030.
int RBOvershellSlotCom::CurrentRev() const {
    return const_cast<RBOvershellSlotCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0xC24050.
bool RBOvershellSlotCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xC24080.
Component* RBOvershellSlotCom::AsComponent() {
    return this;
}

// Inlined into _Imprint (0xC24090): the properties are copied and the
// run-time state starts afresh.
RBOvershellSlotCom::RBOvershellSlotCom(const RBOvershellSlotCom& other)
    : Component(other),
      mRuntime(),
      mList(other.mList),
      mConfirmActionText(other.mConfirmActionText),
      mBaseHeight(other.mBaseHeight),
      mMaxHeight(other.mMaxHeight),
      mStateBlenders(other.mStateBlenders) {}

// Reconstructed from eboot.elf at 0xC24090. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RBOvershellSlotCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RBOvershellSlotCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RBOvershellSlotCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0xC24230.
PropRegistry& RBOvershellSlotCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0xC24240.
ComMetaData& RBOvershellSlotCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0xB88970. The binary emits the
// registration with the other rb_meta classes'.
void RBOvershellSlotCom::Init() {
    InitComponentClass<RBOvershellSlotCom>();
}

// Reconstructed from eboot.elf at 0xBABA30. The binary emits the factory
// with the other rb_meta factories.
Component* RBOvershellSlotCom::_Create() {
    return new RBOvershellSlotCom();
}
