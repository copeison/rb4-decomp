#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "os/joypads/Joypad.h"
#include "os/users/UserGuid.h"
#include "rb_beatmatch/core/Defines.h"
#include "rb_beatmatch/tracks/TrackType.h"
#include "rb_game/core/Defines.h"
#include "rb_game/users/RBBandUser.h"
#include "rb_meta/overshell/RBOvershellCom.h"
#include "rb_meta/overshell/RBOvershellDefines.h"
#include "utl/containers/Deque.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

class BandUser;
class DataArray;
class PropertyBlenderCom;
class RBCharacterPersistentData;
class RBOvershellCom;
class RBOvershellListProviderCom;
class RBOvershellSlotState;
class RBOvershellSlotStateMgr;
class RBProfile;
class RBSompClient;
class RBUIInputCom;
class RndDrawNodeCom;
class RndTextCom;
class UIButtonDownMsg;
class UIButtonUpMsg;
class UIInputReceiverCom;
class UIListCom;
class UISelectMsg;

// The SOMP connection of a slot's user ("this_overshell_slot_connection_state",
// GetConnectionState). Newer than the map; the type and enumerator names are
// inferred from the allowed values' labels.
enum OvershellSlotConnectionState : int {
    kSlotConnectionConnected = 0,
    kSlotConnectionIdle = 1,
    kSlotConnectionHighLatency = 2,
    kSlotConnectionDisconnecting = 3,
    kSlotConnectionOffline = 4,
};

// One slot of the overshell ("Holds state for an overshell slot.", author
// Sheila Porter, category UI, allowed resource RndEntityResource;
// rb_meta/RBOvershellSlotCom.o, 0xC05910-0xC24AFF with the static
// initializer at 0xC24990). The overshell (RBOvershellCom) owns four, one
// per game slot ("slot"). A slot holds at most one band user (through the
// band slot it put the user in), shows the user's state (an
// RBOvershellSlotState of its RBOvershellSlotStateMgr, chosen by the
// user's overshell slot state id), runs the state's view through its
// "list" RBOvershellListProviderCom and UIListCom and its
// "state_blenders", and handles the user's input: joining, choosing a
// profile, part, difficulty and character, the instrument options and the
// in-song menu, and the SOMP session's online states. The vtable at
// 0x19722A8 has 41 slots. The object is 760 bytes.
class RBOvershellSlotCom : public Component {
public:
    // A token the slot shows in its message text for a while
    // ("slot_message_token"): the queue's front is shown until its time
    // runs out. Name and field names not in the reference map.
    struct QueuedMessage {
        Symbol mToken;
        // Shows the token as literal text rather than as a localization
        // token (_ShowNextMessage).
        bool mLiteral;
    };

    // The run-time state, which the imprint's copy starts afresh. The map
    // names the type; the field names are not in the reference map.
    struct RuntimeData {
        RuntimeData();   // 0xC05910
        ~RuntimeData();  // 0xC05DE0

        // "slot": the game slot the slot stands for.
        GameSlot mGameSlot;
        // The overshell's input component (SetInputCom).
        RBUIInputCom* mInputCom;
        // The overshell that owns the slot.
        RBOvershellCom* mOvershell;
        // The "confirm_action_text" object's text component.
        RndTextCom* mConfirmActionTextCom;
        // The object's draw node; _Poll hides the list while it is hidden.
        RndDrawNodeCom* mDrawNode;
        // The "state_blenders" objects' PropertyBlenderComs.
        eastl::vector<PropertyBlenderCom*> mStateBlenderComs;
        // The "list" object's components.
        RBOvershellListProviderCom* mListProvider;
        UIListCom* mUIList;
        UIInputReceiverCom* mInputReceiver;
        // The state shown, and the one before it.
        RBOvershellSlotState* mState;
        RBOvershellSlotState* mPreviousState;
        // The slot's states, created by the constructor and read from
        // slot_states.dta by _OnResourcesLoaded.
        RBOvershellSlotStateMgr* mStateMgr;
        // The state an override flow returns the user to
        // (SetOverrideFlowReturnState); kState_JoinedDefault otherwise.
        OvershellSlotStateID mOverrideFlowReturnState;
        // "current_view".
        Symbol mCurrentView;
        // "active_status".
        OvershellActiveStatus mActiveStatus;
        // The band slot of the slot's user (BandUserMgr), or -1 without
        // one.
        unsigned long mBandSlot;
        // Set when the user chose a part in the song settings flow and
        // cleared when the flow starts again or the user leaves. The
        // evidence for the name is weak.
        bool mPartChosen;
        // "slot_name" and "slot_label".
        Symbol mSlotName;
        Symbol mSlotLabel;
        // The SOMP status tokens the slot shows and sends to the session:
        // the first from the local layout (_UpdateSompStatusToken), all
        // three from the session for an online slot (SetSompStatusTokens).
        Symbol mSompStatusTokens[3];
        // The controllers a user of the slot may play (AddValidController).
        eastl::vector<ControllerType> mValidControllers;
        // The users who may join the slot (AddPotentialUser).
        eastl::vector<BandUser*> mPotentialUsers;
        // The character management session: whether one is open, and the
        // character it manages (name, type and the owner's profile name).
        // The evidence for the names is weak.
        bool mCharManagementSession;
        Symbol mManagedCharName;
        Symbol mManagedCharType;
        String mManagedCharOwner;
        // The override flow the slot is in (BeginOverrideFlow).
        OvershellOverrideFlow mOverrideFlow;
        // "green_button_down" and the others: the user's held buttons,
        // indexed by the JoypadAction each button stands for (_Poll maps
        // them through ButtonToAction).
        bool mButtonDown[15];
        // The part the choose part state focuses ("display_track_type"
        // shows it there; _SetPreviewTrackType).
        TrackType mPreviewTrackType;
        // "num_harmony_mics_to_display", "vocal_freestyle_icon_override" and
        // "pro_drums_icon_override" when the state gives none.
        int mNumHarmonyMicsToDisplay;
        bool mVocalFreestyleIconOverride;
        bool mProDrumsIconOverride;
        // "tooltips": "showing", "button" and "token".
        bool mTooltipShowing;
        JoypadAction mTooltipButton;
        Symbol mTooltipToken;
        // "mic_monitors": "showing" and the three mics' energy.
        bool mMicMonitorsShowing;
        float mMicEnergy[3];
        // Set while the slot needs the character preview scene
        // (NeedsCharPreviewScene).
        bool mNeedsCharPreviewScene;
        // Set and cleared by RemoveProfileFromSlot. Not identified; the
        // evidence for the name is weak.
        bool mWaitingForProfile;
        // Never written in this object; _CanHandleMsg refuses the user's
        // input while it is set. The evidence for the name is weak.
        bool mInputDisabled;
        // The user the account picker joins (_OnAccountPickerJoinUser).
        BandUser* mAccountPickerUser;
        // The subscriptions: the "list" object's "list_focus", and the
        // account picker's result.
        MsgSource::EventSinkElem mListFocusSink;
        MsgSource::EventSinkElem mAccountPickerSink;
        // The cymbal lane "try_toggle_cymbal" waits to toggle, -1 without
        // one.
        bool mCymbalTogglePending;
        int mCymbalToggleLane;
        // Set by _QueueSompUpdate until the poll (_PollSompUpdate) sends the
        // slot's state to the SOMP session, and when it last did.
        bool mSompUpdatePending;
        float mLastSompUpdateMs;
        // The member the kick player flow targets (SetKickTarget): whether
        // one is set, its name and its id.
        bool mKickTargetSet;
        Symbol mKickTargetName;
        Symbol mKickTargetId;
        // Whether the kick request succeeded (OnSompKickResult).
        bool mKickSucceeded;
        // When the user last gave the slot input, in real-time
        // milliseconds (GetSompWaitTime measures from it).
        float mLastInputMs;
        // The shown "slot_message_token" and when it goes.
        bool mShowingMessage;
        float mMessageEndMs;
        Symbol mMessageToken;
        // Set when the message text changed.
        bool mMessageChanged;
        // The messages waiting to be shown.
        eastl::deque<QueuedMessage> mMessageQueue;
        // The emote category and the emote the pick emote flow shows, or
        // -1.
        int mEmoteCategory;
        int mEmote;
        // The slot's "this_overshell_slot_connection_state", as last
        // polled.
        OvershellSlotConnectionState mConnectionState;
        // The states the online flows return to: the drop out, quit and
        // leader confirmations, the online settings and the online exit
        // submenu.
        OvershellSlotStateID mOnlineReturnState;
        OvershellSlotStateID mOnlineSettingsReturnState;
        OvershellSlotStateID mExitSubmenuReturnState;
        // "somp_offline_or_online_transition_done".
        bool mSompTransitionDone;
        // When a state that waits for the server started waiting, or -1
        // (_PollServerWait).
        float mServerWaitStartMs;
        // Set when _PostPoll must hand the input focus to
        // mPendingFocusReceiver (null for the slot's list).
        bool mPendingFocus;
        UIInputReceiverCom* mPendingFocusReceiver;
        // The state the volume slider returns to.
        OvershellSlotStateID mSliderReturnState;
    };

    RBOvershellSlotCom();  // 0xC0A530
    // Copies the properties for an imprint and starts afresh. Inlined into
    // _Imprint (0xC24090). Not in the reference map.
    RBOvershellSlotCom(const RBOvershellSlotCom& other);
    // Slots 0-1: 0xC0A5C0, 0xC0A810. Takes the user out of the slot and
    // deletes the states.
    ~RBOvershellSlotCom() override;

    // Slot 2: the slot's messages and 158 script calls ("show_state",
    // "select_part", "toggle_lefty_flip", ...); UI button presses, releases
    // and selections go to the OnMsg handlers first, and a message no case
    // handles goes to the slot's list provider.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xC1E680
    Symbol GetId() const override;          // slot 4: 0xC24010
    Symbol GetClassName() const override;   // slot 5: 0xC24020
    int CurrentRev() const override;        // slot 7: 0xC24030
    bool IsA(Symbol type) const override;   // slot 8: 0xC24050
    Component* AsComponent() override;      // slot 9: 0xC24080
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0xC24090
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0xC24230
    ComMetaData& _GetMetaData() override;       // slot 23: 0xC24240
    // Slot 29: creates the states. The map's _LoadResources(ObjPtr
    // const&).
    bool _OnResourcesLoaded() override;  // 0xC0F420
    // Slot 31: finds the list, text, draw node and blender components. The
    // map's _Enter(ObjPtr const&).
    void _Enter() override;  // 0xC0F440
    // Slot 33. The map's _Poll(ObjPtr const&).
    void _Poll() override;  // 0xC0FC30
    // Slot 39 (0xC24250). Name not in the reference map.
    bool _HasPostPoll() const override {
        return true;
    }
    // Slot 40: hands the input focus over when _PollFocus asked for it.
    // Name not in the reference map.
    void _PostPoll() override;  // 0xC10900

    // The slot's user: the band user of the slot's band slot (of band slot 0
    // without one).
    BandUser* GetUser() const;  // 0xC0A750
    // Enters or leaves an override flow: takes the slot out of the character
    // preview and, when `update` is set, updates the overshell and the slot.
    // The map's _SetOverrideType(OvershellOverrideFlow, bool).
    void _SetOverrideType(OvershellOverrideFlow flow, bool update);  // 0xC0A770
    // Gives the overshell's input component the input before the slot.
    void SetInputCom(RBUIInputCom* input);  // 0xC0A830
    // Also tells the list provider. The map's
    // SetActiveStatus(OvershellActiveStatus).
    void SetActiveStatus(OvershellActiveStatus status);  // 0xC0A850
    // GetUser again. The binary keeps an uncalled copy. Name not in the
    // reference map.
    BandUser* _GetUser() const;  // 0xC0A870
    // The drop out the user asked for: the confirmation in a SOMP session,
    // else AttemptRemoveUser. Inlined; the binary keeps an uncalled copy.
    // Name not in the reference map.
    void _DropOutUser();  // 0xC0A890
    // Whether a SOMP session is under way. Name not in the reference map.
    static bool IsSomp();  // 0xC0A8B0
    // Shows the SOMP drop out confirmation, the leader's unless `nonLeader`
    // is set or the user does not lead the session, remembering the state
    // to return to; the online error without a user. Name not in the
    // reference map.
    void _ShowSompDropOutConfirm(bool nonLeader);  // 0xC0A8C0
    // Takes the user out of the slot: in a song or the venue meta mode the
    // game drops the user's player first (theMutableGame, 0xA48BB0) and
    // the overshell updates at once. Not reconstructed: the game
    // (theGame/theMutableGame) and the object at 0x1AE5AD8 (0x99E690) are
    // not modelled.
    void AttemptRemoveUser();  // 0xC0A960
    // Joins the user to the slot, unless the user is a local user waiting
    // to join (then only a stripped log is left) and `force` is not set.
    // The map has AddUser(BandUser*).
    void AddUser(BandUser* user, bool force);  // 0xC0AA60
    // Whether the user waits to join the slot.
    bool LookupUserInJoinList(const BandUser* user) const;  // 0xC0AB00
    // Puts the user in the slot's band slot with a fresh character, locks
    // the input receiver to the user's pad ("locked_to_pad_num"), gives the
    // user a profile, applies the vocal volumes and, in a song, starts the
    // drop in ("drop_in_started" to the game). Not reconstructed: it needs
    // the game (theGame/theMutableGame, 0xA45FB0, 0xA48A20), the
    // character manager's 0xFC9750 and an undeclared RBBandUser member
    // (0x9AAA90, see _ClearCurrentCharForUser).
    void _JoinUser(BandUser* user);  // 0xC0AB40
    // Updates the overshell (at once with `now`, else queued) and the slot.
    // Inlined; the binary keeps an uncalled copy. Name not in the reference
    // map.
    void _RefreshOvershell(bool now);  // 0xC0B2B0
    // Takes the user out of the slot: leaves the SOMP session when asked
    // and the user hosts it, clears the character, the profile, the band
    // slot and the override flow, unlocks the input receiver's pad and
    // resets the user. Not reconstructed: it needs an undeclared
    // RBBandUser member (0x9AAA90), the SOMP session's member object
    // (theRBSompSessionMgr+0x28, 0x989890) and the character manager's
    // 0xFC9750. The map has RemoveUser().
    void RemoveUser(bool leaveSession);  // 0xC0B300
    // Clears the user's current character and frees the user's characters
    // in the participating profiles. Inlined into its users; the binary
    // keeps an uncalled copy. Not reconstructed: the clearing is an
    // RBBandUser member RBBandUser.h does not declare (0x9AAA90, which
    // takes a name, a type and an owner, clears the preview character and
    // returns whether the character was set).
    void _ClearCurrentCharForUser(BandUser* user);  // 0xC0B830
    // Hides the character preview and tells the overshell the slot no
    // longer needs the preview scene. The binary keeps an uncalled copy.
    void _DeactivateCharPreviewScene();  // 0xC0B910
    // Asks the poll (_PollSompUpdate) to send the slot's state to the SOMP
    // session when the slot is one of the online overshell's local slots.
    // Name not in the reference map.
    void _QueueSompUpdate();  // 0xC0B940
    // Lets the user join the slot later.
    void AddPotentialUser(BandUser* user);  // 0xC0BA00
    unsigned long NumPotentialUsers() const;  // 0xC0BAF0
    // Whether the user holds the slot, or waits to join an empty slot.
    bool IsValidUser(BandUser* user) const;  // 0xC0BB10
    void ClearPotentialUsers();  // 0xC0BB90
    // Gives the slot the profile of the platform user (a new one unless the
    // id is invalid), sets the user's character from it and shows the
    // joined state, or the shared profile warning when the slot does not
    // own the profile's settings. Not reconstructed: undeclared RBBandUser
    // member 0x9AAA90. The map has AddProfileToSlot(int).
    void AddProfileToSlot(int platformUserId);  // 0xC0BBA0
    // Gives the user the profile's saved character. Not reconstructed:
    // undeclared RBBandUser members (0x9AAA90, 0x9AB2D0, 0x9AB160) and the
    // profile manager's 0xD5D060 and the global at 0x1ADFE20 (0x9315F0).
    void _SetupCurrentCharForUser(BandUser* user, RBProfile* profile);  // 0xC0BDA0
    // Takes the profile out of the slot. Not reconstructed: undeclared
    // RBBandUser member 0x9AAA90 (_ClearCurrentCharForUser inlined).
    void RemoveProfileFromSlot();  // 0xC0BFB0
    // "slot_name": the state's player name, the user's platform name, or
    // the localized join or connect mic prompt.
    void _UpdateSlotName();  // 0xC0C180
    // "slot_label": the state's label. Inlined; the binary keeps an
    // uncalled copy. Name not in the reference map.
    void _UpdateSlotLabel();  // 0xC0C360
    void AddValidController(ControllerType type);  // 0xC0C3B0
    bool IsValidControllerType(ControllerType type) const;  // 0xC0C4A0
    // Whether the user plays a controller other than the one connected.
    // Name not in the reference map.
    bool IsUsingOtherController() const;  // 0xC0C4E0
    // Whether the slot shows the reconnect controller state. Name not in the
    // reference map.
    bool IsReconnectingController() const;  // 0xC0C520
    // The map's GetState() const.
    RBOvershellSlotState* GetState() const;  // 0xC0C540
    // Shows the reconnect controller state and tells the SOMP session.
    // Name not in the reference map.
    void ShowReconnectController();  // 0xC0C550
    // Sets the user's overshell slot state and updates the overshell and
    // the slot.
    void ShowState(OvershellSlotStateID state);  // 0xC0C5B0
    // Shows the user's options: choosing a profile without a platform
    // user, the joined state in the venue meta mode, else the in-song menu
    // or the options. Name not in the reference map.
    void ShowOptions();  // 0xC0C660
    // Whether the user, playing the connected controller on this console,
    // is in the override flow: the overshell's, else the slot's.
    bool InOverrideFlow(OvershellOverrideFlow flow) const;  // 0xC0C740
    OvershellOverrideFlow GetOverrideFlow() const;  // 0xC0C7E0
    bool IsInVenueMetaMode() const;  // 0xC0C7F0
    // Whether the user sees the shared profile warning. Name not in the
    // reference map.
    bool IsShowingSharedProfileWarning() const;  // 0xC0C800
    // Starts the drop-in flow of a user without a player.
    void ForceDropInFlow();  // 0xC0C850
    // Marks the user's part pending and starts the song settings flow.
    void _StartDropInFlow();  // 0xC0C8A0
    // Chooses the part (a SOMP session asks the server first) and shows the
    // next state, or a warning (the solo player's unplayable part, the
    // spotlight warnings). The map has _SelectPartImpl(TrackType, bool,
    // bool); this build adds the next state. Not reconstructed: undeclared
    // RBBandUser member 0x9ABBF0 and the song and game data
    // (theGameData 0x9661A0, 0x8F2E60, 0xA604E0, 0xF28DE0, 0xB12700,
    // 0xB0CA20).
    void _SelectPartImpl(TrackType type, bool alternate, bool spotlight, OvershellSlotStateID next);  // 0xC0C950
    // _SelectPartImpl's SOMP branch. The binary keeps an uncalled copy. Not
    // reconstructed (RBBandUser 0x9ABBF0). Name not in the reference map.
    void _SelectSompPart(TrackType type, bool alternate, bool spotlight);  // 0xC0CD80
    // Shows the choose part wait state and has the list provider disable
    // the parts the user did not select ("disable_unselected_parts").
    void _ShowChoosePartWait();  // 0xC0CE40
    // Leaves the override flow and the options for the joined state;
    // `unpause` resumes a song the overshell paused.
    void LeaveOptions(bool unpause);  // 0xC0D0A0
    // Leaves the override flow without updating. Inlined; the binary keeps
    // an uncalled copy. Name not in the reference map.
    void _ClearOverrideFlow();  // 0xC0D120
    // Shows choosing a part, or the difficulty when the game mode skips the
    // part choice ("skip_choose_part", "forced_part"). Not reconstructed:
    // the game mode configuration (the static at 0x1AE4538, constructor
    // 0xA4ABE0) is not modelled.
    void ShowSongOptions();  // 0xC0D140
    // In a SOMP session, asks the server for the part. Not reconstructed:
    // undeclared RBBandUser members (0x9AB960, 0x9AB970, 0x9AB9D0). Name not
    // in the reference map.
    void _ShowChoosePartServerQueryWait();  // 0xC0D3B0
    // Shows the in-song menu, back to the joined state when the game would
    // not pause.
    void ShowInSongMenu();  // 0xC0D550
    void ShowBandMemberSelect();  // 0xC0D5A0
    void ShowIdle();              // 0xC0D5B0
    // Names not in the reference map.
    void ShowDisabled();          // 0xC0D5C0
    void ShowScreenTransition();  // 0xC0D5D0
    // Has the list provider refresh the options, and shows the instrument
    // options again while they show.
    void RefreshInstrumentOptions();  // 0xC0D5E0
    // The options of the slot's part, or LeaveOptions(true) for a slot
    // without one.
    void _ShowInstrumentOptions();  // 0xC0D800
    // The map's BeginOverrideFlow(OvershellOverrideFlow, bool) and
    // EndOverrideFlow(OvershellOverrideFlow, bool).
    void BeginOverrideFlow(OvershellOverrideFlow flow, bool update);  // 0xC0D8B0
    void EndOverrideFlow(OvershellOverrideFlow flow, bool update);    // 0xC0D920
    void SetOverrideFlowReturnState(OvershellSlotStateID state);  // 0xC0D940
    void RevertToOverrideFlowReturnState();  // 0xC0D950
    // Has the slot's list provider set up the state's own list
    // ("setup_custom_state_list") after the state set the confirmation
    // text. Name not in the reference map.
    void _SetupCustomStateList();  // 0xC0D9D0
    // Has the list provider set up the part list ("setup_part_select_list").
    // Name not in the reference map.
    void _SetupPartSelectList();  // 0xC0DBE0
    // "display_track_type": the state's part, else the part the choose part
    // state focuses, else the user's or the slot's part. Name not in the
    // reference map.
    TrackType GetDisplayTrackType() const;  // 0xC0DDD0
    // The getters of "num_harmony_mics_to_display",
    // "vocal_freestyle_icon_override" and "pro_drums_icon_override": the
    // state's value, else the slot's. Inlined into the accessors; the
    // binary keeps uncalled copies. Names not in the reference map.
    unsigned int GetNumHarmonyMicsToDisplay() const;  // 0xC0DEC0
    bool GetVocalFreestyleIconOverride() const;       // 0xC0DF00
    bool GetProDrumsIconOverride() const;             // 0xC0DF40
    // "slot_join_token" and "slot_empty_token". Names not in the reference
    // map.
    static Symbol _GetJoinToken();  // 0xC0DF80
    Symbol _GetEmptyToken() const;  // 0xC0E090
    // "full_overshell_is_online": a SOMP session's overshell is online or
    // going offline. Inlined into the accessors; the binary keeps an
    // uncalled copy. Name not in the reference map.
    bool IsFullOvershellOnline() const;  // 0xC0E280
    // "slot_message_token". Inlined into the accessor; the binary keeps an
    // uncalled copy. Name not in the reference map.
    Symbol GetMessageToken() const;  // 0xC0E2E0
    // The list view for the list's rows: "list_short" up to three rows,
    // "list_medium" up to five, "list_long" beyond or without a list. Name
    // not in the reference map.
    Symbol _GetListView() const;  // 0xC0E350
    // "confirm_action", or "confirm_action_long" beyond three rows or
    // without a list. Name not in the reference map.
    Symbol _GetConfirmActionView() const;  // 0xC0E4E0
    // Whether the view shows the slot's list: the list views, the
    // confirmation views and "somp_disconnected_message". Names not in the
    // reference map.
    bool _IsListView(const Symbol& view) const;  // 0xC0E600
    // Whether the view is a confirmation view or
    // "somp_disconnected_message".
    bool _IsConfirmActionView(const Symbol& view) const;  // 0xC0E890
    // Updates the state from the user's overshell slot state id: leaves
    // the old state ("exit") and enters the new one ("enter").
    void UpdateState();  // 0xC0EA00
    // The state for the user's overshell slot state id; without a user,
    // kState_MirrorRemoteUser for a remote slot of the online overshell;
    // while users wait to join, kState_Join, or the unnamed state 2 when the
    // overshell is full; else kState_NoInstrument. Inlined into UpdateState.
    RBOvershellSlotState* _GenerateCurrentState() const;  // 0xC0EE30
    // Sets the part the choose part state focuses. Inlined into
    // "display_track_type"'s setter.
    void _SetPreviewTrackType(TrackType type);  // 0xC0EED0
    // Applies the state's list input to the input receiver and the list.
    // Name not in the reference map.
    void _UpdateListInput();  // 0xC0EEE0
    // Focuses or disables the list's input receiver as the state allows, and
    // asks _PostPoll to move the input focus. Name not in the reference
    // map.
    void _UpdateInputReceiverState();  // 0xC0EF90
    // Whether the list scrolls only from code, and whether its rows can be
    // selected, as the state allows. Inlined into _UpdateListInput; the
    // binary keeps uncalled copies. Names not in the reference map.
    void _UpdateListScrolling();   // 0xC0F060
    void _UpdateListSelection();   // 0xC0F0B0
    // Asks _PostPoll to focus the receiver (the slot's list for null).
    // Inlined; the binary keeps an uncalled copy. Name not in the reference
    // map.
    void _QueueFocus(UIInputReceiverCom* receiver);  // 0xC0F100
    // Updates the view: the list, the text and the blenders.
    void UpdateView();  // 0xC0F110
    // Sets the view and asks for the list's focus when the state takes the
    // list input. The map's _SetView(Symbol).
    void _SetView(Symbol view);  // 0xC0F380
    // UpdateState, then UpdateView.
    void Update();  // 0xC0F400
    // Starts the drop-in flow of a local user who has no part or difficulty
    // in a song that requires the song options. Inlined into _Poll; name not
    // in the reference map.
    void _PollDropIn();  // 0xC0FF60
    // A remote user's slot mirrors the remote console
    // (kState_MirrorRemoteUser) once the overshell takes users; a local
    // user leaves the mirror state. Name not in the reference map.
    void _PollRemoteUserState();  // 0xC10000
    // Updates the slot when the user's SOMP connection state changed.
    // Inlined into _Poll; the binary keeps an uncalled copy. Name not in the
    // reference map.
    void _PollConnectionState();  // 0xC10190
    // Shows the next queued message, and drops the shown one when its time
    // ran out. Name not in the reference map.
    void _PollMessages();  // 0xC101F0
    // Times a state that waits for the server: outside a SOMP session the
    // slot leaves its options; after 15 seconds the session ends with
    // "somp_error_booted_from_server". Name not in the reference map.
    void _PollServerWait();  // 0xC102E0
    // Sends the slot's record to the SOMP session client at most every
    // 4 seconds once _QueueSompUpdate asked. Not reconstructed: the record
    // (built by _FillSompSlotRecord) and the client are not modelled. Name
    // not in the reference map.
    void _PollSompUpdate();  // 0xC10590
    // Queues the post-poll while a focus change waits. Inlined into _Poll;
    // the binary keeps an uncalled copy. Name not in the reference map.
    void _QueuePostPoll();  // 0xC108E0
    // Moves the user's pad focus to the queued receiver. Name not in the
    // reference map.
    void _ApplyPendingFocus();  // 0xC10940
    // Gives the user the slot's part: guitar or bass (the profile's more
    // scored one, the other one when the slot's partner plays it), pro or
    // standard drums, vocals or harmonies. Name not in the reference map.
    void _SetDefaultPart();  // 0xC109D0
    // Puts the pad's profile in the slot and leaves the options.
    void _ChooseProfile(int platformUserId);  // 0xC10B40
    // Shows the profile choice, or its denial for a signed-in user.
    void _ChangeProfile();  // 0xC10BC0
    // Back to the options the profile choice came from (exporting
    // "on_cancel"), else drops the user.
    void _CancelChangeProfile();  // 0xC10C40
    // Exports "on_cancel" through the slot's entity.
    void _ExportCancelMsg();  // 0xC10D20
    // Whether the slot shows a remote user of the online overshell.
    // Inlined into "this_overshell_slot_is_remote"; the binary keeps an
    // uncalled copy. Name not in the reference map.
    bool IsRemoteSlot() const;  // 0xC10D90
    void _SetTooltipShowing(bool showing);  // 0xC10DB0
    // Harmonies show the song's number of vocal parts (3 without a song);
    // solo vocals show none and the freestyle icon when it applies.
    void _UpdateVocalPartIcon(ScoreType type);  // 0xC10DC0
    void _UpdateVocalFreestyleIcon(Difficulty difficulty);  // 0xC10EB0
    // Whether a local solo vocalist on hard or expert of a song with vocal
    // improvisation (the gameplay options allowing it) in a game mode that allows it gets the
    // freestyle icon. Not reconstructed: it reads the game mode (a static
    // GameMode at 0x1AE4538), which is not modelled.
    bool _ShouldShowVocalFreestyleIcon(Difficulty difficulty) const;  // 0xC10ED0
    // The pro drums icon for real drum scoring. Inlined into
    // _SetDefaultPart; the binary keeps an uncalled copy. Name not in the
    // reference map.
    void _UpdateProDrumsIcon(ScoreType type);  // 0xC11050
    // The part selections: _SelectPartImpl with kState_ChoosePartWait, then
    // the overshell and the slot refresh.
    void _SelectPart(TrackType type);  // 0xC11060
    // Name not in the reference map.
    void _SelectPartWithOptions(TrackType type, bool alternate, bool spotlight);  // 0xC110B0
    void _SelectVocalPart(bool harmony);    // 0xC11100
    void _SelectDrumPart(bool proDrums);    // 0xC11150
    // The game data's quit and skip tokens of the in-song menu. Inlined into
    // Handle; the binary keeps uncalled copies. Names not in the reference
    // map.
    static Symbol GetCurrentOvershellQuitToken();  // 0xC111A0
    static Symbol GetCurrentOvershellSkipToken();  // 0xC111B0
    // Whether the in-song menu offers to restart the song: not in game mode
    // 7 (likely practice). Inlined into Handle ("show_restart_prompt"); the
    // binary keeps an uncalled copy. Name not in the reference map; the
    // evidence is weak.
    static bool _ShouldShowRestartPrompt();  // 0xC111C0
    // Whether a changed difficulty lets the song continue: not in game mode
    // 8. Inlined into Handle ("allow_continue_after_difficulty_change");
    // the binary keeps an uncalled copy. Name not in the reference map.
    static bool _AllowContinueAfterDifficultyChange();  // 0xC111E0
    // "get_current_difficulty_change_prompt_token": the SOMP or the game
    // mode 8 ("_btm") variant of "overshell_difficulty_confirm". Name not in
    // the reference map.
    static Symbol _GetDifficultyChangePromptToken();  // 0xC11200
    // "setup_difficulty_change_confirm_menu": continue and cancel in a SOMP
    // session, restart and cancel in game mode 8, else all three. Name not
    // in the reference map.
    void _SetupDifficultyChangeConfirmMenu();  // 0xC11380
    // Sets the confirmation text token (clearing the text first) and gives
    // the list provider the actions with the focused one. Inlined into the
    // setup functions; the binary keeps an uncalled copy. Name not in the
    // reference map.
    void _ShowConfirmActions(Symbol text, const eastl::vector<Symbol>& actions, Symbol focus);  // 0xC11A90
    // "setup_solo_toggle_confirm_menu" and "setup_cymbal_toggle_confirm_menu":
    // "overshell_gtrsolo_confirm" with continue, restart (not in a SOMP
    // session) and cancel. The two are the same code. Names not in the
    // reference map.
    void _SetupSoloToggleConfirmMenu();    // 0xC11B00
    void _SetupCymbalToggleConfirmMenu();  // 0xC12020
    // "finish_cymbal_selection": in the song settings flow, back to choosing
    // the part, or to the drum message when not confirmed; otherwise back to
    // the options.
    void _FinishCymbalSelect(bool confirmed);  // 0xC12540
    // "dismiss_cymbal_message": back to choosing the difficulty.
    void _DismissCymbalMessage();  // 0xC126F0
    // "select_difficulty": a quarantined song in the song settings flow asks
    // to confirm a changed difficulty in song, the vocal improvisation
    // onboarding popup may come first, else the difficulty is applied. Not
    // reconstructed: it reads the quarantine config (0x1AE4538), the
    // session object at 0x1AE4938 and the band user manager's onboarding
    // events (0x9261C0, 0x926620), none of them modelled.
    void _SelectDifficulty(Difficulty difficulty);  // 0xC12760
    // Gives the user the difficulty: in a SOMP session through the session
    // (_RequestSompDifficulty), else ready to play; in song the game takes
    // the player in. Name not in the reference map. Not reconstructed: it
    // calls the game (theGame 0x1AEF770: 0xA45FB0, 0xA48A20), which is not
    // modelled.
    void _ApplyDifficulty(Difficulty difficulty);  // 0xC12DC0
    // Cancels the song settings: with the song options required, back to
    // the in-song menu, the SOMP drop out confirmation, or the drop in is
    // cancelled ("drop_in_cancelled" to the game); otherwise the song
    // options cancel confirmation, or the overshell's flow ends. Exports
    // "on_cancel". Not reconstructed: it calls the game (theGame 0x1AEF770,
    // its mutable copy's 0xA47490), which is not modelled.
    void _CancelSongSettings();  // 0xC12EB0
    // In the choose difficulty states, asks the SOMP session for the
    // difficulty and waits for it; otherwise the response is treated as
    // rejected. Name not in the reference map. Not reconstructed: it calls
    // an RBBandUser member (0x9ABDD0) that is not declared.
    void _RequestSompDifficulty(Difficulty difficulty);  // 0xC13180
    // "confirm_choose_diff": applies the state's "difficulty" and leaves
    // the override flow, and outside a SOMP session the options.
    void _ConfirmChooseDiff();  // 0xC132A0
    // "leave_choose_part": cancels the song settings, clearing the SOMP
    // part first.
    void _LeaveChoosePart();  // 0xC13400
    // "clear_somp_part": tells the SOMP client that a local user has no
    // part. Name not in the reference map. Not reconstructed: it calls the
    // SOMP session manager's client (0x1B156C8, 0x98AFB0), which is not
    // modelled.
    void _ClearSompPart();  // 0xC13460
    // "leave_choose_part_wait": back to choosing the part. Not
    // reconstructed: the SOMP branch calls an RBBandUser member (0x9AB970)
    // that is not declared.
    void _LeaveChoosePartWait();  // 0xC13540
    // The SOMP branch of _LeaveChoosePartWait. Inlined; the binary keeps an
    // uncalled copy. Name not in the reference map. Not reconstructed (as
    // _LeaveChoosePartWait).
    void _LeaveSompChoosePartWait();  // 0xC13620
    // "leave_choose_difficulty": back to choosing the part, or the song
    // settings are cancelled when the part choice is skipped
    // ("skip_choose_part" in the quarantine config) or not yet made in
    // song. Exports "on_cancel". Not reconstructed: it reads the quarantine
    // config (0x1AE4538) and calls an RBBandUser member (0x9AB970).
    void _LeaveChooseDifficulty();  // 0xC13690
    // The SOMP branch of _LeaveChooseDifficulty. Inlined; the binary keeps an
    // uncalled copy. Name not in the reference map. Not reconstructed (as
    // _LeaveChooseDifficulty).
    void _LeaveSompChooseDifficulty();  // 0xC138C0
    // "setup_song_options_cancel": "overshell_cancel_song" with no, yes and
    // leave; in a SOMP session cancel, the leader's return to the green
    // room or the music library, and leave. Name not in the reference map.
    void _SetupSongOptionsCancelMenu();  // 0xC13930
    // The in-song menu's actions (0xC14240-0xC14D00).
    // Pauses or resumes the game and returns the slot to its joined
    // state. Not reconstructed: it pauses through the mutable game
    // (theMutableGame, 0x1AEF778), which is not modelled.
    void _OnTogglePause();  // 0xC14240
    // Restarts the song from the slot: in a SOMP session only the session
    // leader may (the others only log the user); during the song, the
    // controller check and the mutable game's restart. Not reconstructed:
    // the game (0x1AEF778) and the meta state's member at 0xBBF3D0 are not
    // modelled.
    void _OnRestartSong();  // 0xC14300
    // Sends theGameData "btm_handle_restart_show" (a message class with
    // the vtable 0x19638D8) and returns the slot to its joined state. Name
    // not in the reference map. Not reconstructed: RBGameData is not
    // modelled.
    void _OnRestartShow();  // 0xC144E0
    // Skips to the next song of a show: a SOMP session asks the session
    // (_RequestSompSkipSong), otherwise theGameData gets "gig_skip_song".
    // Not reconstructed: RBGameData is not modelled.
    void _OnSkipSongInShow();  // 0xC14630
    // The session leader asks the SOMP client to skip the song and waits
    // (kState_OnlineLeaderSkipToNextSongWait); anyone else gets
    // kState_OnlineError. Name not in the reference map.
    void _RequestSompSkipSong();  // 0xC147F0
    // Drops the user out of the venue meta mode, or asks first
    // (kState_RemoveUserInSongConfirm); a SOMP session shows its drop out
    // confirmation. Inlined into Handle; the binary keeps an uncalled copy.
    void _OnVenueMetaDropOut();  // 0xC148A0
    // Quits the song, or asks first (kState_QuitEarlyConfirm). Inlined into
    // Handle; the binary keeps an uncalled copy.
    void _OnAttemptQuit();  // 0xC14930
    // Quits the song to the game mode's layout.
    void _OnQuitEarly();  // 0xC149D0
    // Quits the song to the layout (kLayoutInvalid: the game mode's), or
    // in a SOMP session leaves it (_QuitSompSong). The map has only
    // _OnQuitEarly(). Name not in the reference map. Not reconstructed: the
    // game, RBGameData, the campaign manager and the meta state's member at
    // 0xBBF3D0 are not modelled.
    void _QuitEarly(RBUILayout layout);  // 0xC149E0
    // Leaves the SOMP song: the client drops the user and the session goes
    // to the green room; without a session or user, the slot drops out of
    // the session. Name not in the reference map.
    void _QuitSompSong();  // 0xC14BE0
    // "go_to_player_stats": goes to kLayoutPlayerStats for the user's
    // profile, or shows why the online services are closed
    // ("store_need_to_signin", "store_need_to_patch",
    // "store_restricted_age"). Name not in the reference map. Not
    // reconstructed: the profile's store status (+0xC58), the dialog
    // parameters and the entry call (0xC37F40, 0xB0EEA0) are not modelled.
    void _GoToPlayerStats();  // 0xC14D00
    // The instrument options (0xC15170-0xC177FC). Each change tells the
    // list provider ("update_..._checkbox") and, in a SOMP session, sends
    // the options to the session's client.
    void _ToggleLeftyFlip();  // 0xC15170
    // The drum fill logic steps through its three values.
    void _CycleDrumFillLogicForward();   // 0xC15440
    void _CycleDrumFillLogicBackward();  // 0xC156A0
    // Starts toggling the cymbal of the lane ("try_toggle_cymbal"): during a
    // song it asks first (kState_ToggleCymbalConfirm), otherwise it toggles
    // at once. Inlined into Handle; the binary keeps an uncalled copy.
    void _ToggleCymbal(unsigned int lane);  // 0xC158F0
    // Toggles the pending cymbal lane in the system options
    // ("confirm_toggle_cymbal"). Name not in the reference map.
    void _ConfirmToggleCymbal();  // 0xC15990
    // Forgets the pending cymbal toggle ("cancel_toggle_cymbal"). Inlined
    // into Handle; the binary keeps an uncalled copy. Name not in the
    // reference map.
    void _CancelToggleCymbal();  // 0xC15C90
    // Toggles the guitar solo mode; during a song the change is confirmed
    // (kState_ConfirmSoloToggle).
    void _ToggleGuitarSoloMode();  // 0xC15CD0
    // Inlined into the button handler; the binary keeps an uncalled copy.
    // Name not in the reference map.
    void _ShowConfirmSoloToggle();  // 0xC15F30
    // Toggles the solo mode back and returns to the guitar options. The
    // binary keeps an uncalled copy. Name not in the reference map; the
    // evidence is weak.
    void _RevertGuitarSoloMode();  // 0xC15F40
    // Raises the breakneck speed (exporting "on_widget_scroll" when it
    // changed) and updates the player. The map's _ToggleBreakneckSpeed;
    // this build steps up and down. Names not in the reference map. Not
    // reconstructed: the player's object at RBPlayer+0x490 (0xA3BE20) is not
    // modelled.
    void _IncrementBreakneckSpeed();  // 0xC15FC0
    // Exports "on_widget_scroll" through the slot's entity.
    void _ExportWidgetScrollMsg();  // 0xC162A0
    void _DecrementBreakneckSpeed();  // 0xC16310
    // Toggles vocal improvisation and updates the freestyle icon.
    void _ToggleVocalImprov();  // 0xC165F0
    // The volume slider: activating it remembers the state to return to
    // (kState_OptionsSlider); a step the provider takes exports
    // "on_widget_scroll"; deactivating returns to the remembered state or
    // to the instrument options.
    void _ActivateVolumeSlider();    // 0xC168C0
    void _IncrementVolumeSlider();   // 0xC169F0
    void _DecrementVolumeSlider();   // 0xC16C20
    void _DeactivateVolumeSlider();  // 0xC16E50
    // Inlined; the binary keeps an uncalled copy. Name not in the reference
    // map.
    void _ResetSliderReturnState();  // 0xC16F90
    // The text slider (state 22, which the state table does not name).
    // Names not in the reference map.
    void _ActivateTextSlider();    // 0xC16FA0
    void _DeactivateTextSlider();  // 0xC170C0
    // Toggles no fail in the system options (in a SOMP session only the
    // session's leader may, and the session's value is toggled) and sends
    // the options to the session's client. Not reconstructed: during a
    // song the client gets the game's time (theGame, 0x1AEF770), which is
    // not modelled.
    void _ToggleNoFail();  // 0xC171E0
    // Only calls the user's GetPlatformUserId and GetPlatformName, the
    // remains of a stripped log. Inlined into the button handler; the binary
    // keeps an uncalled copy. Name not in the reference map; the evidence is
    // weak.
    void _PrintUserInfo();  // 0xC17380
    // Toggles the options' microphone overdrive deploy (the options' slots
    // 29-31). Name not in the reference map.
    void _ToggleMicOverdriveDeploy();  // 0xC173F0
    // "show_track_texture_menu": shows state 58, which the state table does
    // not name. Inlined into Handle; the binary keeps an uncalled copy. Name
    // not in the reference map.
    void _ShowTrackTextureMenu();  // 0xC17690
    // The track texture preferences: "toggle_show_local_track_texture",
    // "toggle_allow_remote_track_textures" (the SOMP session's flag, during
    // a session), "toggle_random_track_texture" and
    // "toggle_random_track_color" toggle the user's preference flags 0, 2
    // and 3 (RBBandUser) and the session's flag. Inlined into Handle; the
    // binary keeps uncalled copies. Names not in the reference map.
    void _ToggleShowLocalTrackTexture();  // 0xC176D0
    static void _ToggleAllowRemoteTrackTextures();  // 0xC17720
    void _ToggleRandomTrackTexture();  // 0xC17760
    void _ToggleRandomTrackColor();  // 0xC177B0
    // Puts the managed character on the user, records the user's band slot
    // for the character screens, leaves the options and goes to the player
    // portrait. Not reconstructed: it hands the slot to a static setter of
    // an object that is not identified (0xE7D2B0, a global at 0x1B39600).
    void _ConfirmCharManagementSession();  // 0xC17800
    // The character data of the managed character's owner: the user's
    // profile for "char_type_player", the other user's for
    // "char_type_other_user"; otherwise, or without a session, the
    // character error state and null.
    RBCharacterPersistentData* _GetManagementSessionCharacterDataOrFail();  // 0xC17960
    // Gives the user the character, marks it in use with the character
    // manager and refreshes the slots' character lists; the character
    // error state when it fails. Not reconstructed: it calls RBBandUser's
    // character setter (0x9AAA90) and RBCharManager members (0xFC7FC0,
    // 0xFC9750) that are not declared.
    bool _TryToSetActiveCharForUser(BandUser* user, Symbol name, Symbol type, String owner);  // 0xC17B20
    // Whether characters may be managed: outside a SOMP session. Inlined
    // into Handle's "allow_manage_char"; the binary keeps an uncalled copy.
    // Name not in the reference map; the evidence is weak.
    static bool _AllowManageChar();  // 0xC17C10
    // Starts managing the character, ending a session already open. Only
    // the user's own characters and another user's are managed; the
    // user's own need the user's character data.
    bool _StartCharManagementSession(Symbol name, Symbol type, String owner);  // 0xC17C30
    // Inlined into _StartCharManagementSession and Handle; the binary
    // keeps an uncalled copy.
    void _EndCharManagementSession();  // 0xC17EC0
    // The slot user's profile's character data; the character error state
    // and null without a user with a profile and a controller.
    RBCharacterPersistentData* _GetCharacterDataOnProfileOrFail();  // 0xC17F50
    // Whether a management session is open; the character error state
    // otherwise. The binary keeps an uncalled copy. Name not in the
    // reference map.
    bool _IsInCharManagementSessionOrFail();  // 0xC17FE0
    // The managed character, or the empty symbol without a session. The
    // binary keeps an uncalled copy.
    Symbol _GetManagementSessionChar();  // 0xC18020
    // The character data of the band user with the id `owner`; the
    // character error state and null without the user or its profile.
    RBCharacterPersistentData* _GetCharacterDataOnOtherProfileOrFail(String owner);  // 0xC18080
    // The character type of the type symbol ("char_type_player",
    // "char_type_prefab", "char_type_other_user"), with the other user's
    // id parsed from `owner`; false for another symbol.
    bool _GetCharTypeFromSymbols(
        const Symbol& type,
        const String& owner,
        RBBandUser::CharType& charType,
        UserGuid& ownerGuid) const;  // 0xC18180
    // Starts creating a character on the user's profile, or on another
    // user's ("start_char_create_other_user_session").
    void _StartCharCreate();                    // 0xC18350
    void _StartCharCreateOther(String owner);  // 0xC18440
    // "start_char_shop_session": the shop for the managed character.
    void _StartRockShop();  // 0xC18550
    // Enters the shop of the session's band: the campaign's band, the
    // profile's only band, the band select for several, or the no band
    // error. Name not in the reference map. Not reconstructed: it reads
    // RBCampaignMgr and the session band objects (0x1ADFE20, 0x1ADFE28),
    // which are not modelled.
    void _EnterShop();  // 0xC186A0
    // Whether the user can shop with the current character: a profile with
    // a band and characters, a current character and a saved character of
    // the profile's own. Name not in the reference map. Not reconstructed:
    // it reads RBBandUser's current character (0x9AA8E0), which is not
    // declared.
    bool _CanStartShopSession();  // 0xC18800
    // "start_generic_shop_session" and "start_track_tex_shop_session": the
    // shop for the user's current character. The binary keeps uncalled
    // copies of the two wrappers. Names not in the reference map.
    void _StartGenericShopSession();            // 0xC18960
    void _StartShopSession(bool trackTexShop);  // 0xC18970
    void _StartTrackTexShopSession();           // 0xC18A70
    // "start_char_edit_session": edits the managed character.
    void _StartCharEdit();  // 0xC18A80
    // Deletes the managed character from its owner's profile, replacing it
    // in the profile's bands with a prefab, takes it off the user and
    // returns to the character choice. Not reconstructed: it calls
    // RBBandUser's character members (0x9AA8C0, 0x9AB160) and RBCharManager
    // members (0xFC3B30, 0xFC81B0) that are not declared.
    void _DeleteManagementSessionChar();  // 0xC18BE0
    // "select_char": the character, then the options; the character error
    // state when it fails. Inlined into Handle; the binary keeps an
    // uncalled copy.
    void _SelectActiveChar(Symbol name, Symbol type, String owner);  // 0xC18FD0
    // Gives the slot's user the character, as _TryToSetActiveCharForUser.
    // Not reconstructed for the same reason.
    bool _SetActiveChar(Symbol name, Symbol type, String owner);  // 0xC19060
    // "select_band_char": the character, then the band member select's
    // ready state. Inlined into Handle; the binary keeps an uncalled copy.
    void _SelectBandChar(Symbol name, Symbol type, String owner);  // 0xC19170
    // Sends "refresh_char_data" with the slot to the list provider.
    void _RefreshCharList();  // 0xC19210
    // Marks the slot as needing the character preview scene. Inlined; the
    // binary keeps an uncalled copy.
    void _ActivateCharPreviewScene();  // 0xC19430
    // Clears the user's preview character.
    void _HideCharPreview();  // 0xC19440
    // The binary keeps an uncalled copy; RBOvershellCom's
    // UpdateCharPreviewSceneState inlines it.
    bool NeedsCharPreviewScene();  // 0xC19520
    // Shows the character as the user's preview character. Not
    // reconstructed: it calls RBBandUser's character members (0x9AA6D0,
    // 0x9AA110) and the character manager, which are not declared.
    bool _ShowCharPreview(Symbol name, Symbol type, String owner);  // 0xC19530
    // "show_preview_of_managed_char". The binary keeps an uncalled copy.
    bool _ShowPreviewOfManagedChar();  // 0xC19790
    // Shows the idle state: master, not the master or no master.
    void _CheckIfIdleMaster();  // 0xC19830
    // The binary keeps an uncalled copy.
    UIListCom* _GetUIList() const;  // 0xC198A0
    // How long the slot's user has waited since the last button pulse, or
    // -1 without a user; RBOvershellCom::GetSompMinWaitTime reads it. Name
    // not in the reference map; the evidence is weak.
    float GetSompWaitTime() const;  // 0xC198B0
    // Whether the slot is not a remote user's slot of the online overshell.
    // The binary keeps an uncalled copy. Name not in the reference map.
    bool _IsLocalSlot() const;  // 0xC19950
    // The SOMP status token the slot sends from the local layout: song
    // select, search, difficulty select, the song or its results, the
    // loading screen, and the green room (ready for the session's host).
    // Clears the other two tokens. Only with a session client. Name not in
    // the reference map.
    void _UpdateSompStatusToken();  // 0xC19980
    // Fills the record the slot sends to the SOMP session (the layout, the
    // state, the icons, the view and the tokens, and in the difficulty
    // select the list's rows and the text). Not reconstructed: the record
    // (0xD8 bytes and more) and the list provider's member (0xBFFD60) are
    // not modelled. Name not in the reference map.
    void _FillSompSlotRecord(void* record);  // 0xC19F80
    // Whether the meta state shows the difficulty select, and whether the
    // layout is it. Inlined; the binary keeps uncalled copies. Names not in
    // the reference map.
    static bool IsInDifficultySelect();                   // 0xC1A1A0
    static bool IsDifficultySelectLayout(RBUILayout layout);  // 0xC1A1B0
    // Whether a remote slot shows the record's own view: in the difficulty
    // select on both consoles, from the choose part state on, unless the
    // record's list view has a selection or its confirmation view has no
    // tokens. Called by RBOvershellRemoteSlotState::_GetView (0xC26D64).
    // Not reconstructed: the record is not modelled. Name not in the
    // reference map.
    bool _UsesSompRecordView(const void* record);  // 0xC1A1C0
    // Sets the three status tokens of an online slot (from the session's
    // member record; RBOvershellCom's 0xBEFA80 forwards them). Name not in
    // the reference map.
    void SetSompStatusTokens(Symbol status, Symbol status2, Symbol status3);  // 0xC1A240
    // The session's answer to the user's part query: on success in the
    // choose part state the parts are refreshed (OnSompPartsChanged), else
    // the user's part lists are set and the choose part state shown; a
    // failure gives up (_OnSompResponseRejected). Not reconstructed: it
    // sets the lists through RBBandUser's 0x9ABA70, which RBBandUser.h
    // does not declare. Name not in the reference map; the argument types
    // of the lists are not identified.
    void OnSompPartsResult(
        BandUser* user,
        const UserGuid& guid,
        bool success,
        const void* parts,
        const void* partStates);  // 0xC1A2A0
    // Whether a session answer is for the slot's local user: the slot is
    // not remote, the user and the id match; `current` tells whether the
    // user's request state (RBBandUser's 0x9AB960, -1 for any) and the
    // slot's state (-1 for any) are still the ones the request was made in.
    // Not reconstructed: RBBandUser.h does not declare the request state's
    // getter. Name not in the reference map.
    bool _IsSompResponseForUser(
        BandUser* user,
        const UserGuid& guid,
        int requestState,
        OvershellSlotStateID waitState,
        bool* current);  // 0xC1A3E0
    // A request of the slot's local user failed while the difficulty
    // select or the song shows: refreshes (0xC13460) and shows the online
    // error unless the UI transitions or the session ends. Name not in the
    // reference map.
    void _OnSompResponseRejected();  // 0xC1A580
    // The session changed the parts the user may choose: in the choose
    // part state the user's lists are replaced and the part list set up
    // again. Not reconstructed (RBBandUser's 0x9ABA70). Name not in the
    // reference map.
    void OnSompPartsChanged(
        BandUser* user,
        const UserGuid& guid,
        const void* parts,
        const void* partStates);  // 0xC1A630
    // The session withdrew the part query: in the song settings flow the
    // slot leaves the wait (0xC13460, the user's request state back to 4
    // through RBBandUser's 0x9AB970, 0xC0D3B0). Not reconstructed. Name not
    // in the reference map.
    void OnSompPartsCancelled(BandUser* user, const UserGuid& guid);  // 0xC1A700
    // The session's answers to the score type and difficulty requests:
    // the choose difficulty (or ready to play) state on success. Not
    // reconstructed: the values go through RBBandUser's 0x9ABC70 and
    // 0x9ABE50. Names not in the reference map.
    void OnSompScoreTypeResult(BandUser* user, const UserGuid& guid, bool success, ScoreType type);  // 0xC1A7D0
    void OnSompDifficultyResult(
        BandUser* user,
        const UserGuid& guid,
        bool success,
        Difficulty difficulty);  // 0xC1A8A0
    // The session's answer to the kick request: shown when it is for the
    // pending target. Name not in the reference map.
    void OnSompKickResult(bool success, Symbol name, Symbol id);  // 0xC1A970
    // The kick target's name and id, empty without one. Names not in the
    // reference map.
    Symbol GetKickTargetName() const;  // 0xC1AA80
    Symbol GetKickTargetId() const;    // 0xC1AAA0
    // The session's answers to the leader's return to the green room, the
    // return to the music library and the skip to the next song: back to
    // the joined state on success, else to the state before the request.
    // Names not in the reference map.
    void OnSompReturnToGreenRoomResult(BandUser* user, const UserGuid& guid, bool success);  // 0xC1AAC0
    // Shows mOnlineReturnState again. Inlined; the binary keeps an uncalled
    // copy. Name not in the reference map.
    void _RevertToOnlineReturnState();  // 0xC1ABA0
    void OnSompReturnToMusicLibraryResult(BandUser* user, const UserGuid& guid, bool success);  // 0xC1ABC0
    // The skip ends in the joined state whatever the answer.
    void OnSompSkipToNextSongResult(BandUser* user, const UserGuid& guid, bool success);  // 0xC1ACA0
    // Whether the slot's local user may make a session request: a client
    // (returned in `client`), the slot not remote, the state (-1 for any)
    // and the user's request state (-1 for any). Not reconstructed
    // (RBBandUser's 0x9AB960). Name not in the reference map.
    bool _GetSompRequestClient(int requestState, OvershellSlotStateID waitState, RBSompClient** client);  // 0xC1AD60
    // Tells the session the slot's local user chooses a difficulty. Name
    // not in the reference map.
    void _SendSompChoosingDifficulty();  // 0xC1AE50
    // Sends that, sets the user's request state to 8 (RBBandUser's
    // 0x9AB970) and shows the choose difficulty state. Not reconstructed.
    // Name not in the reference map.
    void _StartSompChooseDifficulty();  // 0xC1AF30
    // Sets the member the kick player flow targets, replacing any; always
    // true. Name not in the reference map.
    bool SetKickTarget(Symbol name, Symbol id);  // 0xC1AFA0
    void ClearKickTarget();  // 0xC1AFE0
    // Whether a kick target is set; shows the online error otherwise. Name
    // not in the reference map.
    bool _HasKickTarget();  // 0xC1B010
    // Empty. Name not in the reference map; the evidence is weak.
    void _OnKickPlayerCancelled() {}  // 0xC1B030
    // Asks the session client to kick the target and waits for the
    // answer. Without a client the online error shows first and the
    // request is still made. Name not in the reference map.
    void _KickPlayer();  // 0xC1B040
    // "overshell_somp_kick_player_confirmation" with the target's name,
    // with yes and no (focused). Name not in the reference map.
    void _ShowKickPlayerConfirmation();  // 0xC1B0D0
    // Sets the confirmation text and the list's actions. Inlined into the
    // confirmations; the binary keeps an uncalled copy. Name not in the
    // reference map.
    void _ShowConfirmActionsText(const char* text, const eastl::vector<Symbol>& actions, Symbol focus);  // 0xC1B4B0
    // "overshell_somp_kick_player_success" or "..._failure" with the
    // target's name, with continue. Name not in the reference map.
    void _ShowKickPlayerResult();  // 0xC1B520
    // Sends the list provider "setup_online_pick_emote_category" and
    // "setup_online_pick_emote" with the emote category. Names not in the
    // reference map.
    void _SetupEmoteCategoryList();  // 0xC1B860
    void _SetupEmoteList();          // 0xC1BA50
    // The binary keeps uncalled copies of the next four. Names not in the
    // reference map.
    int GetEmoteCategory() const;          // 0xC1BC40
    void _ClearListData();                 // 0xC1BC50
    void SetEmoteCategory(int category);   // 0xC1BC60
    void _ClearEmote();                    // 0xC1BC70
    // The pick emote category list's selection and cancel: a category with
    // emotes opens them (kState_OnlinePickEmote), one with subcategories
    // lists them; cancel goes up a level or back to the joined state.
    // Exports "on_select" or "on_cancel". Not reconstructed: the session
    // manager's emote tables (0xD3BB00, 0xD3BB10, 0xD36F20) and the index
    // conversions (0xA842D0, 0xA84280) are not modelled. Names not in the
    // reference map.
    DataNode _OnEmoteCategorySelect(int index);  // 0xC1BC90
    // Exports "on_select". The map's _ExportSelectMsg().
    void _ExportSelectMsg();  // 0xC1BDF0
    DataNode _OnEmoteCategoryCancel();  // 0xC1BE60
    // The pick emote list's selection: sends the emote (0x98A540) and waits
    // (kState_OnlinePickEmoteWait). Not reconstructed (as above). Name not
    // in the reference map.
    DataNode _OnEmoteSelect(int index);  // 0xC1BF90
    // The emote of the list's row, or null. Not reconstructed (as above).
    // Name not in the reference map.
    const void* _GetEmote(int index) const;  // 0xC1C130
    DataNode _OnEmoteCancel();  // 0xC1C1A0
    // The SOMP session's answer to the user's emote request: on success
    // the slot leaves the emote menus and queues the emote's token as a
    // literal message; a refused request shows the rate throttle or the
    // online error. Only for the slot's user and the pending request
    // (_IsSompRequestForSlot, 0xC1A3E0). Forwarded by RBOvershellCom
    // (0xBF20D0). Name not in the reference map; the evidence is weak.
    void _OnSompPickEmoteResult(BandUser* user, const UserGuid& guid, bool succeeded, bool throttled);  // 0xC1C2D0
    // Queues a message for "slot_message_token" and the confirm action
    // text; `literal` shows the token as text instead of localizing it.
    // Called by RBOvershellCom. Name not in the reference map.
    void QueueMessage(Symbol token, bool literal);  // 0xC1C500
    // The SOMP session's client when the slot's user may send requests
    // (in a session, with a user, and when `leaderOnly` the session
    // leader). The binary keeps an uncalled copy. Name not in the
    // reference map.
    bool _GetSompClient(bool leaderOnly, RBSompClient** client) const;  // 0xC1C5E0
    // Asks the SOMP session manager for the layout and leaves the options.
    // Name not in the reference map.
    void _GotoSompLayout(RBUILayout layout);  // 0xC1C680
    // Drops the slot's user out of the SOMP session: the drop out request,
    // the overshell going offline, a platform call with the user's id
    // (PlatformMgr slot 33, not modelled) and kLayoutSompHub. Not
    // reconstructed: PlatformMgr's slots 22-35 are not declared. Name not
    // in the reference map.
    void _DropOutOfSompSession();  // 0xC1C710
    // The session leader's requests to return to the green room and to the
    // music library, waiting for the answer; the online error otherwise.
    // Names not in the reference map.
    void _ReturnToGreenRoom();     // 0xC1C860
    void _ReturnToMusicLibrary();  // 0xC1C910
    // Show the online confirmations, remembering the state to return to.
    // Names not in the reference map.
    void _ShowQuitSessionConfirm();            // 0xC1C9C0
    void _ShowReturnToGreenRoomConfirm();      // 0xC1C9F0
    void _ShowReturnToMusicLibraryConfirm();   // 0xC1CA20
    // The online settings menu and its exit submenu: entering remembers
    // the state to return to, leaving returns there (the online error
    // when there is none). Names not in the reference map.
    void _ShowOnlineSettings();      // 0xC1CA50
    void _LeaveOnlineSettings();     // 0xC1CA80
    void _ReturnToOnlineSettings();  // 0xC1CAB0
    void _ShowOnlineExitSubmenu();   // 0xC1CAC0
    void _LeaveOnlineExitSubmenu();  // 0xC1CAF0
    // "overshell_online_state". Inlined into the accessor; the binary keeps
    // an uncalled copy. Name not in the reference map.
    OvershellOnlineState GetOvershellOnlineState() const;  // 0xC1CB20
    // "slot_is_somp_leader". Inlined into the accessor; the binary keeps an
    // uncalled copy. Name not in the reference map.
    bool IsUserSompLeader() const;  // 0xC1CB80
    // "show_somp_connection_spinner". Inlined into the accessor; the
    // binary keeps an uncalled copy. Name not in the reference map.
    bool ShowsSompConnectionSpinner() const;  // 0xC1CC00
    // Whether the slot is not one of the overshell's online slots. The
    // binary keeps an uncalled copy. Name not in the reference map.
    bool _IsNotOnlineSlot() const;  // 0xC1CC20
    // "this_overshell_slot_connection_state". Name not in the reference
    // map.
    OvershellSlotConnectionState GetConnectionState() const;  // 0xC1CC40
    // "slot_forced_view". Name not in the reference map.
    Symbol GetForcedView() const;  // 0xC1CD00
    // Whether a message shows or waits. Called by RBOvershellCom, the
    // remote slot state and 0x99E3ED. Name not in the reference map.
    bool HasMessage() const;  // 0xC1D130
    // When the shown message goes, in milliseconds. The binary keeps an
    // uncalled copy. Name not in the reference map.
    float GetMessageEndMs() const;  // 0xC1D150
    // Shows the next queued message for four seconds. Name not in the
    // reference map.
    void _ShowNextMessage();  // 0xC1D160
    // Set the confirm action text to the text, or to the token. Inlined;
    // the binary keeps uncalled copies. Names not in the reference map.
    void _SetConfirmActionText(const char* text);  // 0xC1D2B0
    void _SetConfirmActionToken(Symbol token);     // 0xC1D300
    // The overshell starts going online or offline: the slot's transition
    // animation is not done. Two copies of the same body. Called by
    // RBOvershellCom. Names not in the reference map.
    void BeginGoingOnline();  // 0xC1D350
    // The overshell went online: a slot outside the online slots drops a
    // local user and mirrors the remote user. Called by RBOvershellCom.
    // Name not in the reference map.
    void FinishGoingOnline();  // 0xC1D360
    void BeginGoingOffline();  // 0xC1D3E0
    // The overshell went offline: a remote user is dropped, and the mirror
    // state leaves for the joined default. Called by RBOvershellCom. Name
    // not in the reference map.
    void FinishGoingOffline();  // 0xC1D3F0
    // Whether the slot's transition animation is done; always outside the
    // pending states. Called by RBOvershellCom. Name not in the reference
    // map.
    bool IsOnlineTransitionDone() const;  // 0xC1D4B0
    // Whether the slot handles the user's input: the user's own slot while
    // the overshell is active. A user without a slot gets a handled
    // result.
    bool _CanHandleMsg(BandUser* user, DataNode& result) const;  // 0xC1D4F0
    // The state handles the input; cancel, start and the view modifier
    // become "on_cancel", "on_start" (with the user) and "on_view_modify"
    // when it does not. A press in the join view also exports
    // "button_pulse". The map's OnMsg(UIButtonDownMsg const&).
    DataNode OnMsg(const UIButtonDownMsg& msg);  // 0xC1D630
    DataNode OnMsg(const UIButtonUpMsg& msg);    // 0xC1DF40
    DataNode OnMsg(const UISelectMsg& msg);      // 0xC1E110
    // The list's focus moved: the state handles "list_focus". The map's
    // _OnListFocus(DataArray*); its return type is not recorded.
    void _OnListFocus(DataArray* msg);  // 0xC1E1D0
    // (setup_confirm_actions token ...): shows the token in the confirm
    // action text and hands the message to the list provider.
    void _OnSetupConfirmActions(DataArray* msg);  // 0xC1E2D0
    // (setup_tooltip button token).
    void _OnSetupTooltip(DataArray* msg);  // 0xC1E3B0
    // The account picker closed: leaves its event and, when a pad picked
    // a profile, gives the slot that profile.
    void _OnAccountPickerResult(int padNum, bool cancelled);  // 0xC1E3F0
    // The account picker closed for a joining user: joins the user, then
    // as _OnAccountPickerResult.
    void _OnAccountPickerJoinUser(int padNum, bool cancelled);  // 0xC1E540
    // Export "on_start" and "on_select_disabled" through the entity.
    void _ExportStartMsg();           // 0xC1E5A0
    void _ExportSelectDisabledMsg();  // 0xC1E610

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it with the other rb_meta classes' at 0xB88970.
    static void Init();
    // The class factory, emitted at 0xBABA30; the placement factory beside
    // it (0xBABA60) is InitComponentClass's.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0xC05FD0

    // "RBOvershellSlotCom".
    static Symbol sId;  // 0x1B03ED0
    // Also "RBOvershellSlotCom". Name not in the reference map.
    static Symbol sClassName;           // 0x1B03ED8
    static PropRegistry sPropRegistry;  // 0x1B03EE0
    static ComMetaData sMetaData;       // 0x1B03F80

    RuntimeData mRuntime;
    // Field names are not in the reference map.
    // "list": the object with the slot's RBOvershellListProviderCom and
    // UIListCom.
    GameObjectId mList;
    // "confirm_action_text": the object with the RndTextCom the slot's
    // error and confirmation texts go to.
    GameObjectId mConfirmActionText;
    // "height": "base_height" and "max_height".
    float mBaseHeight;
    float mMaxHeight;
    // "state_blenders".
    PropArray<GameObjectId> mStateBlenders;
};

static_assert(sizeof(RBOvershellSlotCom::QueuedMessage) == 16);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mInputCom) == 8);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mStateBlenderComs) == 40);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mListProvider) == 72);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mState) == 96);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mStateMgr) == 112);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mOverrideFlowReturnState) == 120);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mCurrentView) == 128);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mBandSlot) == 144);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mSlotName) == 160);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mValidControllers) == 200);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mPotentialUsers) == 232);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mCharManagementSession) == 264);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mManagedCharOwner) == 288);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mOverrideFlow) == 304);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mButtonDown) == 308);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mPreviewTrackType) == 324);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mNumHarmonyMicsToDisplay) == 328);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mTooltipShowing) == 334);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mTooltipButton) == 336);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mTooltipToken) == 344);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mMicMonitorsShowing) == 352);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mMicEnergy) == 356);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mNeedsCharPreviewScene) == 368);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mAccountPickerUser) == 376);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mListFocusSink) == 384);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mAccountPickerSink) == 424);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mCymbalTogglePending) == 464);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mLastSompUpdateMs) == 476);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mKickTargetName) == 488);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mLastInputMs) == 508);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mShowingMessage) == 512);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mMessageToken) == 520);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mMessageChanged) == 528);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mMessageQueue) == 536);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mEmoteCategory) == 624);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mConnectionState) == 632);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mOnlineReturnState) == 636);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mSompTransitionDone) == 648);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mPendingFocus) == 656);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mPendingFocusReceiver) == 664);
static_assert(offsetof(RBOvershellSlotCom::RuntimeData, mSliderReturnState) == 672);
static_assert(sizeof(RBOvershellSlotCom::RuntimeData) == 680);
static_assert(offsetof(RBOvershellSlotCom, mRuntime) == 24);
static_assert(offsetof(RBOvershellSlotCom, mList) == 704);
static_assert(offsetof(RBOvershellSlotCom, mBaseHeight) == 712);
static_assert(offsetof(RBOvershellSlotCom, mStateBlenders) == 720);
static_assert(sizeof(RBOvershellSlotCom) == 760);
