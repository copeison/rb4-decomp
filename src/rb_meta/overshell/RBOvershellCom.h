#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropRegistry.h"
#include "math/random/Rand.h"
#include "os/users/UserGuid.h"
#include "rb_game/core/Defines.h"
#include "rb_meta/overshell/RBOvershellDefines.h"
#include "rb_meta/state/RBMetaStateCom.h"
#include "utl/containers/Set.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataNode.h"
#include "utl/messages/Message.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

class BandUser;
class DataArray;
class GameAboutToBeDestroyedMsg;
class GameCreatedMsg;
class GameEndedMsg;
class GameMicsChangedMsg;
class GameModeChangedMsg;
class GameStartedMsg;
class JoypadConnectionMsg;
class RBOvershellSlotCom;
class RBProfile;
class RBUIInputCom;
class SystemOverlayStatusChangedMsg;

// The online state of the overshell's slots in a session of the SOMP mode
// (the slot component's "overshell_online_state" property). Newer than the
// map; the type and enumerator names are inferred from the allowed values'
// labels (GetOvershellOnlineStateAllowedValues).
enum OvershellOnlineState : int {
    kOvershellOnlineStateInvalid = -1,
    kOvershellOnlineStateOffline = 0,
    kOvershellOnlineStatePendingOffline = 1,
    kOvershellOnlineStateOnline = 2,
    kOvershellOnlineStatePendingOnline = 3,
};

// The state's label, "UNKNOWN_STATE" out of range. Newer than the map; name
// not in the reference map. The binary keeps an uncalled copy.
const char* OvershellOnlineStateToString(OvershellOnlineState state);  // 0xBE6F60
// The states with their labels, for RBOvershellSlotCom's
// "overshell_online_state". Newer than the map; name not in the reference
// map.
eastl::vector<AllowedValue<int>> GetOvershellOnlineStateAllowedValues();  // 0xBE6F80

// "override_ended", which the overshell exports when an override flow ends:
// (target "override_ended" flow cancelled). Inline in the map's build; the
// constructor (0xBF6610) and the deleting destructor (0xBF6840) are emitted
// here, the complete destructor (0xBEAAC0) jumps to Message<2>'s
// (0x3E5B0), and the vtable is at 0x1971B40.
class OvershellOverrideEndedMsg : public Message<2> {
public:
    OvershellOverrideEndedMsg(OvershellOverrideFlow flow, bool cancelled);  // 0xBF6610
    ~OvershellOverrideEndedMsg() override;

    // The message type, built on first use (0x1B02FE0).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("override_ended");
        }
        return t;
    }
};

// The component that runs the overshell scene ("Total control of the
// overshell scene."; rb_meta/RBOvershellCom.o, 0xBE6F60-0xBF727F with the
// static initializer at 0xBF7180). It owns the four RBOvershellSlotComs of
// the scene ("overshell_slots"), joins the band's users into them, resolves
// the slots' states (choosing parts, readying, controller loss), runs the
// override flows that take every slot through one screen (song settings,
// band member select, idle), and keeps the slots' online state in a SOMP
// session. The constructor publishes it as theOvershell. The vtable at
// 0x19719E8 has 41 slots. The object is 1064 bytes.
class RBOvershellCom : public Component {
public:
    // A queued Begin- or EndOverrideFlow that could not run yet
    // (_CanChangeOverrideFlow). Name and field names not in the reference
    // map.
    struct OverrideFlowRequest {
        // Begin or end.
        bool mBegin;
        OvershellOverrideFlow mFlow;
        // EndOverrideFlow's flag.
        bool mCancelled;
    };

    // The run-time state, which the imprint's copy starts afresh. The map
    // names the type; the field names are not in the reference map.
    struct RuntimeData {
        RuntimeData();   // 0xBE72D0
        ~RuntimeData();  // 0xBE78E0

        // Set until _FindSlots runs from the edit mode's poll.
        bool mNeedsFindSlots;
        // Set while the system overlay shows, and when it closed until
        // _Poll handles it.
        bool mSystemOverlayOpen;
        bool mSystemOverlayClosed;
        // The slot components by GameSlot, from "overshell_slots".
        eastl::vector<RBOvershellSlotCom*> mSlots;
        // The slots in the order users join them: guitar, second guitar,
        // vocals, drums.
        eastl::vector<unsigned long> mJoinOrder;
        // The overshell object's RBUIInputCom, given to the slots.
        RBUIInputCom* mInputCom;
        // Set by "lost_connection_to_server"; the next poll drops the users
        // who are not signed in.
        bool mLostConnectionToServer;
        // The override flow in progress.
        OvershellOverrideFlow mOverrideFlow;
        eastl::vector<OverrideFlowRequest> mOverrideFlowRequests;
        // "active_status".
        OvershellActiveStatus mActiveStatus;
        bool mInVenueMetaMode;
        bool mSongOptionsRequired;
        bool mUseExtendedMicArrows;
        bool mAllowsButtonPulse;
        // Users waiting for a slot until the overshell takes users again.
        eastl::vector<BandUser*> mQueuedJoinUsers;
        // Draws the slot whose user must pick another part; seeded with
        // mRandomSeed so that the draw holds for an override flow.
        Rand mPartRand;
        // Drawn from gRand when an override flow begins.
        unsigned long mRandomSeed;
        // The idle master: the user who owns the idle flow and their slot.
        UserGuid mIdleMasterGuid;
        int mIdleMasterPadNum;
        const RBProfile* mIdleMasterProfile;
        unsigned long mIdleMasterSlot;
        // "gig_show_difficulty_song_ms" plus "gig_grace_difficulty_song_ms"
        // from gig/config.dta; CheckAndAutoPauseGame compares the game's
        // time with it. Left uninitialized by the constructor.
        int mDifficultySelectMs;
        // The last _UpdateGameplayBlocking values.
        bool mGameplayBlocking;
        bool mSompLayoutTransitionBlocking;
        // The slots' online state in a SOMP session, and the slots taking
        // part in it.
        OvershellOnlineState mOnlineState;
        eastl::set<unsigned long> mOnlineSlots;
        // Set when the online state changes; the SOMP poll clears it.
        bool mOnlineStateChanged;
        // Set while "somp_must_reconnect_original_controller" shows.
        bool mReconnectDialogShowing;
        // Set by the SOMP join flows of other objects (0xB5C86F); IsFull
        // reports the overshell full meanwhile. The evidence for the name
        // is weak.
        bool mJoinBlocked;
        // When the pending online state started, in milliseconds.
        float mOnlineTransitionStartMs;
        // When the local user's SOMP drop-in started (_PollSompDropIn), or
        // -1. The evidence for the name is weak.
        float mSompDropInStartMs;
        // Set by another object (0xD79EEC) while it moves a user to another
        // slot; the pending online states do not hold the slots meanwhile.
        // The evidence for the name is weak.
        bool mMovingUser;
        // Set when the SOMP session changes the layout
        // (OnSompLayoutChanged); the override flow is
        // kOverrideFlowSompTransition until the timer runs out.
        bool mSompLayoutTransition;
        float mSompLayoutTransitionTime;
        // Set by QueueUpdateAll; _Poll runs UpdateAll once.
        bool mNeedsUpdateAll;
        // Set by _OnLayoutTransitionDone; _PostPoll resets the scene's draw
        // registrations.
        bool mNeedsDrawReset;
        // The layout the joypad lights were last set for.
        RBUILayout mLightLayout;
        // The subscriptions, made by _Enter (and OnMsg(GameCreatedMsg)).
        MsgSource::EventSinkElem mTransitionDoneSink;
        MsgSource::EventSinkElem mGameMicsChangedSink;
        MsgSource::EventSinkElem mDebugControllerTypesChangedSink;
        MsgSource::EventSinkElem mProfileAboutToBeRemovedSink;
        MsgSource::EventSinkElem mProfileFailedToSaveLoadSink;
        MsgSource::EventSinkElem mPendingProfilesResolvedSink;
        MsgSource::EventSinkElem mGameCreatedSink;
        MsgSource::EventSinkElem mGameStartedSink;
        MsgSource::EventSinkElem mGameEndedSink;
        MsgSource::EventSinkElem mGameAboutToBeDestroyedSink;
        MsgSource::EventSinkElem mFailScreenPresentedSink;
        MsgSource::EventSinkElem mSetSongOptionsRequiredSink;
        MsgSource::EventSinkElem mErrorZeroPlayersSink;
        MsgSource::EventSinkElem mSystemOverlayStatusChangedSink;
        MsgSource::EventSinkElem mGameModeChangedSink;
        MsgSource::EventSinkElem mJoypadSink;
        MsgSource::EventSinkElem mLostConnectionToServerSink;
    };

    RBOvershellCom();  // 0xBE8810
    // Slots 0-1: 0xBE88D0, 0xBE8970. Clears theOvershell.
    ~RBOvershellCom() override;

    // Slot 2: the overshell's messages and script calls.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xBF4700
    Symbol GetId() const override;          // slot 4: 0xBF6370
    Symbol GetClassName() const override;   // slot 5: 0xBF6380
    int CurrentRev() const override;        // slot 7: 0xBF6390
    bool IsA(Symbol type) const override;   // slot 8: 0xBF63B0
    Component* AsComponent() override;      // slot 9: 0xBF63E0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0xBF63F0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0xBF6580
    ComMetaData& _GetMetaData() override;       // slot 23: 0xBF6590
    // Slot 29: finds the RBUIInputCom and reads the difficulty select time
    // from gig/config.dta. The map's _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;  // 0xBF2860
    // Slot 31: finds the slots and subscribes to the events. The map's
    // _Enter(ObjPtr const&).
    void _Enter() override;  // 0xBF2A40
    // Slot 33. The map's _Poll(ObjPtr const&).
    void _Poll() override;  // 0xBF36A0
    // Slot 38. The map's _EditPoll(ObjPtr const&).
    void _EditPoll() override;  // 0xBF3C70
    // Slot 39 (0xBF65A0). Name not in the reference map.
    bool _HasPostPoll() const override {
        return true;
    }
    // Slot 40: resets the scene's draw registrations after a layout
    // transition. Name not in the reference map.
    void _PostPoll() override;  // 0xBF3CA0

    // The slot of the game slot, or null. Inlined into its users; the
    // binary keeps an uncalled copy.
    RBOvershellSlotCom* GetSlotComFromGameSlot(GameSlot slot) const;  // 0xBE8A10
    // The slot holding the user, else the slot the user waits to join.
    RBOvershellSlotCom* GetSlotFromUser(BandUser* user) const;  // 0xBE8A80
    // The slot at the index, or null.
    RBOvershellSlotCom* GetSlot(unsigned long index) const;  // 0xBE8B10
    // Sets the active status and the venue meta mode, refreshes the slots
    // and joins the queued users. The map's
    // SetActiveStatus(OvershellActiveStatus, bool).
    void SetActiveStatus(OvershellActiveStatus status, bool venueMetaMode);  // 0xBE8B40
    void SetSongOptionsRequired(bool required);  // 0xBE8E20
    // Marks the song options required and tries the drop-ins when the
    // game starts. The binary keeps an uncalled copy.
    void _SetInGame(bool inGame);  // 0xBE8E30
    // Whether the overshell takes users now: active, not overridden, and in
    // a SOMP session not in a pending online transition. Name not in the
    // reference map.
    bool _CanJoinUsers() const;  // 0xBE8ED0
    // Joins the queued users the slots take. Inlined into SetActiveStatus;
    // the binary keeps an uncalled copy.
    void _AddQueuedJoinUsers();  // 0xBE8F40
    // Assigns the joinable users to slots, updates and resolves every
    // slot's state, and handles the controller disconnects.
    void UpdateAll();  // 0xBE8FE0
    // The active status, and through `inVenueMetaMode` whether the song
    // status is the venue meta mode. Name not in the reference map. The
    // binary keeps an uncalled copy.
    OvershellActiveStatus GetActiveStatus(bool* inVenueMetaMode) const;  // 0xBE9110
    void EnterVenueMetaMode();  // 0xBE9130
    void ExitVenueMetaMode();   // 0xBE9150
    // Takes every slot out of its options.
    void LeaveOptions();  // 0xBE91F0
    // Sets the status the meta state's layout calls for: in song in the
    // game layout, inactive while loading, in meta otherwise. Inlined; the
    // binary keeps an uncalled copy.
    void _ChooseActiveStatusFromMetaState();  // 0xBE9250
    void SetUseExtendedMicArrows(bool use);  // 0xBE9280
    void SetAllowsButtonPulse(bool allows);  // 0xBE9290
    // Records the idle master, and points the meta state's focus slot at
    // the slot.
    void SetIdleMasterState(
        const UserGuid& guid,
        int padNum,
        const RBProfile* profile,
        const unsigned long& slot);  // 0xBE92A0
    // The binary keeps an uncalled copy.
    void ClearIdleMaster();  // 0xBE92E0
    // Whether a slot's user is the idle master the idle flow began with.
    bool IsAnySlotOriginalIdleMaster() const;                   // 0xBE9370
    bool SlotIsOriginalIdleMaster(const unsigned long& slot) const;  // 0xBE9430
    // Whether a slot's user may be the idle master: the same user, pad or
    // profile.
    bool IsAnySlotPossibleIdleMaster() const;                   // 0xBE9470
    bool SlotIsPossibleIdleMaster(const unsigned long& slot) const;  // 0xBE9510
    // Shows every slot's idle state again during the idle flow.
    void RedetectSlotIdleMaster();  // 0xBE95F0
    // Adds the user to the slot that waits for them, or queues them while
    // the overshell takes no users.
    void AttemptToAddUser(BandUser* user);  // 0xBE9640
    void _QueueUserToJoin(BandUser* user);  // 0xBE9860
    // Whether no more users can join: as many users as the game mode
    // allows, or the SOMP checks.
    bool IsFull() const;  // 0xBE99B0
    OvershellOnlineState GetOnlineState() const {  // 0xBE9AC0
        return mRuntime.mOnlineState;
    }
    // Checks a joining user against the SOMP session's local user. False
    // when the user may not join; `switchProfile` and `profile` then tell
    // the caller to take the local user's profile. Name not in the
    // reference map.
    bool _CheckSompJoin(BandUser* user, bool* switchProfile, RBProfile** profile);  // 0xBE9AD0
    // Drops the users of the slots that require the song options. The
    // binary keeps an uncalled copy.
    void RemoveUsersRequiringSongOptions();  // 0xBE9EE0
    // Returns the overshell to the meta screens: the meta status, no
    // override flow, every slot out of its options, and offline. Name not
    // in the reference map.
    void Reset();  // 0xBE9F70
    // Starts the override flow, or queues it while a flow cannot start.
    void BeginOverrideFlow(OvershellOverrideFlow flow);  // 0xBEA690
    // Ends the override flow if it is the current one, or queues the end.
    void EndOverrideFlow(OvershellOverrideFlow flow, bool cancelled);  // 0xBEA150
    // Leaves the online state; a public entry of _GoOffline. Name not in
    // the reference map.
    void GoOffline();  // 0xBEA1E0
    // Runs the queued override flow requests while they can run. Name not
    // in the reference map.
    void _ProcessOverrideFlowRequests();  // 0xBEA1F0
    // Runs one request. Inlined; the binary keeps uncalled copies of the
    // two halves. Names not in the reference map.
    bool _CanProcessOverrideFlowRequest(const OverrideFlowRequest& request) const;  // 0xBEA330
    void _ProcessOverrideFlowRequest(const OverrideFlowRequest& request);  // 0xBEA340
    // Whether a begin, or an end, is queued. Inlined; the binary keeps
    // uncalled copies. Names not in the reference map.
    bool _HasQueuedBeginOverrideFlow() const;  // 0xBEA3D0
    bool _HasQueuedEndOverrideFlow() const;    // 0xBEA400
    // The body of BeginOverrideFlow. Inlined; the binary keeps an uncalled
    // copy. Name not in the reference map.
    void _BeginOverrideFlow(OvershellOverrideFlow flow);  // 0xBEA430
    // The body of EndOverrideFlow: exports "override_ended". Name not in
    // the reference map.
    void _EndOverrideFlow(OvershellOverrideFlow flow, bool cancelled);  // 0xBEA4A0
    // Whether the override flow may change now. Name not in the reference
    // map.
    bool _CanChangeOverrideFlow(bool beginning) const;  // 0xBEA5A0
    // The override flow in effect: kOverrideFlowDisabled while inactive or
    // in the blocking gameplay states, kOverrideFlowSompTransition while a
    // SOMP layout transition lasts. Name not in the reference map.
    OvershellOverrideFlow GetOverrideFlow() const;  // 0xBEA880
    // Queues an end. Name not in the reference map.
    void _QueueEndOverrideFlow(OvershellOverrideFlow flow, bool cancelled);  // 0xBEA950
    // Exports the message through the overshell's entity.
    void _Export(DataArray* msg);  // 0xBEAAB0
    // Whether the meta state's gameplay state keeps the overshell
    // disabled. Inlined; the binary keeps an uncalled copy. Name not in the
    // reference map.
    bool _IsGameplayBlocking() const;  // 0xBEAAD0
    // Whether a SOMP layout transition lasts. Inlined; the binary keeps an
    // uncalled copy. Name not in the reference map.
    bool _IsInSompLayoutTransition() const;  // 0xBEAB70
    // Ends the override flow once the overshell is disabled. Name not in
    // the reference map.
    void _EndBlockedOverrideFlow();  // 0xBEAB90
    // Refreshes the slots when the blocking states change. Name not in the
    // reference map.
    void _UpdateGameplayBlocking();  // 0xBEADA0
    // Whether the overshell should show: active and not disabled.
    // Inlined into "should_be_showing"; the binary keeps an uncalled copy.
    // Name not in the reference map.
    bool ShouldBeShowing() const;  // 0xBEAEB0
    // Whether the online state is settled enough to update the slots: not
    // pending, unless a user is being moved, and no UI transition. Inlined;
    // the binary keeps uncalled copies. Names not in the reference map.
    bool _IsOnlineStateSettled() const;  // 0xBEDC70
    bool _CanUpdateSlots() const;        // 0xBEAED0
    // Whether the user's slot is in the idle flow, and whether it lets the
    // input through to the shell. Inlined; the binary keeps uncalled
    // copies. Names not in the reference map.
    bool IsUserInIdleFlow(BandUser* user) const;                // 0xBEAF20
    bool IsUserSlotAllowingInputToShell(BandUser* user) const;  // 0xBEAFC0
    // Whether the user's input goes to the shell rather than to the
    // overshell: true unless the slot that holds the user, or has the user
    // waiting to join, is in the idle flow and keeps the input. The map has
    // the per-slot IsAnySlotAllowingInputToShell(); this per-user form is
    // newer. Name not in the reference map; the evidence is weak.
    bool IsUserAllowingInputToShell(BandUser* user);  // 0xBEB060
    void _ResolveReadyToPlayStates();  // 0xBEB1C0
    void _ResolvePartWaitStates();     // 0xBEB590
    // Shows the character preview scene while a slot needs it.
    void UpdateCharPreviewSceneState();  // 0xBEBFB0
    // The users in the slots. Name not in the reference map. The binary
    // keeps an uncalled copy.
    int GetNumUsers() const;  // 0xBEC020
    // Whether the SOMP session's local user must reconnect the controller
    // they joined with. Name not in the reference map.
    bool _NeedsSompControllerReconnect() const;  // 0xBEC080
    bool InSong() const;  // 0xBEC1E0
    // The binary keeps uncalled copies of the next five.
    bool AllSlotsReadyToPlay() const;  // 0xBEC1F0
    bool ShouldPause() const;          // 0xBEC240
    bool AllSlotsHaveChosenBandMember() const;  // 0xBEC2F0
    bool HasAnyUsers() const;  // 0xBEC3C0
    // Starts the drop-in flow of the users who play no track or are not
    // participating. Name not in the reference map.
    void _StartDropInFlows();  // 0xBEC410
    bool _IsInGame() const;    // 0xBEC4C0
    void _RefreshJoinableUsers();  // 0xBEC4D0
    void _ResolveSlotStates();     // 0xBECA70
    // Hands the slots whose user's controller is gone to
    // _HandleControllerDisconnect. Inlined into UpdateAll; the binary keeps
    // an uncalled copy.
    void _CheckForControllerDisconnects();  // 0xBECB80
    // Asks the next poll to run UpdateAll. Name not in the reference map.
    void QueueUpdateAll();  // 0xBECC20
    // The binary keeps uncalled copies of the next five.
    int _GetNumJoinedUsers(ControllerType type) const;  // 0xBECC30
    bool IsAnySlotAllowingInputToShell() const;          // 0xBECC90
    bool IsAnyLocalSlotAllowingInputToShell() const;     // 0xBECCF0
    bool AreAllLocalSlotsAllowingInputToShell() const;   // 0xBECD60
    bool IsAnySlotJoinable() const;                      // 0xBECDE0
    void _ResolveChooseProfileStates();  // 0xBECE40
    // Resolves a slot whose controller is gone: back to its profile and
    // drop-in flow, or out of the slot. Name not in the reference map.
    void _ResolveControllerDisconnectedState(RBOvershellSlotCom* slot);  // 0xBECED0
    // Puts the slot into the override flow in progress, or takes it out of
    // a finished one. True when the slot's state stands. Name not in the
    // reference map.
    bool _ResolveOverrideFlowState(RBOvershellSlotCom* slot, BandUser* user);  // 0xBECFD0
    // Whether the online state is pending offline or online. Name not in
    // the reference map; the evidence is weak.
    bool IsOnline() const;  // 0xBED090
    // _ResolveControllerDisconnectedState's reconnection. Inlined; the
    // binary keeps an uncalled copy. Name not in the reference map.
    void _ReconnectSlot(RBOvershellSlotCom* slot);  // 0xBED0A0
    // Whether the slot's user stays in the game. Inlined; the binary keeps
    // an uncalled copy. Name not in the reference map.
    bool _ShouldKeepSlotUser(RBOvershellSlotCom* slot) const;  // 0xBED100
    // Shows the reconnect dialog when needed. Inlined into _Poll; the
    // binary keeps an uncalled copy. Name not in the reference map.
    void _CheckForSompControllerReconnect();  // 0xBED160
    // Shows "somp_must_reconnect_original_controller". Name not in the
    // reference map.
    void _ShowReconnectControllerDialog();  // 0xBED1A0
    // The binary keeps an uncalled copy. Name not in the reference map.
    bool IsReconnectDialogShowing() const {  // 0xBED460
        return mRuntime.mReconnectDialogShowing;
    }
    // The reconnect dialog closed: leaving it before the controller came
    // back drops the session's local user. Name not in the reference map.
    void _OnReconnectControllerDialogHidden(int button);  // 0xBED470
    // Reports the local user's drop out with the reason and takes every
    // slot out of its options. Inlined; the binary keeps an uncalled copy.
    // Name not in the reference map.
    void _DropOutOfSompSession(Symbol reason);  // 0xBED5B0
    // Whether the slot's state is not the one the override flow in
    // progress calls for. Name not in the reference map.
    bool _IsSlotOutOfOverrideFlow(RBOvershellSlotCom* slot);  // 0xBED650
    // Puts the slot into the override flow in progress. Name not in the
    // reference map.
    void _StartSlotOverrideFlow(RBOvershellSlotCom* slot, OvershellSlotStateID returnState);  // 0xBED750
    // Drops the users not signed in after a lost connection, and syncs the
    // SOMP session's remote users. Name not in the reference map.
    void _PollRemoteUsers();  // 0xBED950
    // Starts going offline: pending offline from online, else offline at
    // once. Name not in the reference map.
    void _GoOffline();  // 0xBEDAF0
    // _SyncRemoteUsers (0xBEDCB0), _UpdateRemoteUsers (0xBEE680) and
    // _UpdateRemoteUser (0xBEE810) copy the SOMP session's remote slot
    // records into the remote users. Not declared: the records are not
    // modelled. Names not in the reference map.
    bool IsPendingOnline() const;   // 0xBEE7F0
    bool IsPendingOffline() const;  // 0xBEE800
    // Lets a changed remote user drop in to the game in progress. Name not
    // in the reference map.
    void _DropInRemoteUser(BandUser* user);  // 0xBEEAB0
    // Drops users until one is left, those not flagged first, before the
    // SOMP session takes its local user. Name not in the reference map.
    void PrepareForSompLocalUser();  // 0xBEEBB0
    // Drops users the game mode has no room for. Name not in the
    // reference map.
    void _OnGameModeChanged();  // 0xBEEDA0
    // Updates the slot. The binary keeps an uncalled copy.
    void _Update(RBOvershellSlotCom* slot);  // 0xBEEE60
    void _OnProfileAboutToBeRemoved(RBProfile* profile);  // 0xBEEE70
    // Sends "refresh_char_list" to every slot.
    void RefreshSlotCharLists();  // 0xBEEF00
    void _OnProfileFailedToSaveLoad(RBProfile* profile);  // 0xBEF050
    // The binary keeps an uncalled copy; the second half is a separate
    // copy, whose name is not in the reference map.
    void _OnPendingProfilesResolved();  // 0xBEF0B0
    void _SetupSlotCharacters();        // 0xBEF150
    // Pauses the game while a slot's state calls for it.
    bool CheckAndAutoPauseGame();    // 0xBEF1E0
    void CheckAndAutoUnpauseGame();  // 0xBEF2B0
    // Ends the slot's override flow and shows its controller disconnected
    // state, pausing the game. Name not in the reference map.
    void _HandleControllerDisconnect(RBOvershellSlotCom* slot);  // 0xBEF300
    // Drops the remote users whose controller is gone, with the SOMP
    // error. Name not in the reference map.
    void CheckForSompControllerDisconnects();  // 0xBEF460
    // Drops the users whose controller is gone. The binary keeps an
    // uncalled copy. Name not in the reference map.
    void _RemoveDisconnectedUsers();  // 0xBEF6C0
    // Whether the slot is online: always outside the online states.
    // Names not in the reference map.
    bool IsSlotOnline(unsigned long slot) const;   // 0xBEF710
    bool IsOffline() const;                        // 0xBEF790
    bool _IsOnlineSlot(unsigned long slot) const;  // 0xBEF7B0
    bool IsSlotOffline(unsigned long slot) const;  // 0xBEF820
    // Replaces the slot in the online slots. Name not in the reference
    // map.
    void MoveOnlineSlot(unsigned long from, unsigned long to);  // 0xBEF8A0
    // Hands the remote user's settings to an online slot. Inlined; the
    // binary keeps an uncalled copy. Name not in the reference map.
    // 0xBEFA80; not declared, its arguments are not identified.
    // Goes pending online for the slots. Names not in the reference map.
    void _GoOnline(const eastl::set<unsigned long>& slots);  // 0xBEFB10
    void _ResetOnlineSlots();                              // 0xBEFD70
    void GoOnline(const eastl::vector<BandUser*>& users);  // 0xBEFE80
    void GoOnline(BandUser* user);                         // 0xBF0010
    void _FinishGoingOnline();                             // 0xBF02E0
    void _ResetOfflineSlots();                             // 0xBF0230
    // Goes back offline before the online state was reached.
    void CancelGoingOnline();  // 0xBF03B0
    void _ResetGoingOfflineSlots();  // 0xBF04A0
    void _FinishGoingOffline();      // 0xBF05B0
    void _ResetOfflineSlotsAfterOffline();  // 0xBF06F0
    // Whether the slot is not one of the online slots. Name not in the
    // reference map.
    bool IsNotOnlineSlot(unsigned long slot) const;  // 0xBF07A0
    // Checks of the slots during a pending state. Inlined; the binary keeps
    // uncalled copies. Names not in the reference map.
    bool _AreOnlineSlotsReady() const;          // 0xBF0810
    bool _AreOnlineSlotUsersReady() const;      // 0xBF08F0
    bool _IsSlotUserReady(RBOvershellSlotCom* slot) const;  // 0xBF09F0
    bool _IsAnyOnlineSlotUserReady() const;     // 0xBF0A30
    static bool _AreRemoteSlotsConnected();     // 0xBF0B40
    void _UpdatePendingOnlineState();           // 0xBF0BA0
    // Leaves the pending states once the slots are ready or the time ran
    // out. Names not in the reference map.
    void _UpdatePendingOnline();   // 0xBF0BC0
    void _UpdatePendingOffline();  // 0xBF0F80
    bool _HasOnlineTransitionTimedOut(bool ready, bool expired) const;  // 0xBF11F0
    // Goes offline once no online slot's user is ready. Name not in the
    // reference map.
    void _UpdateOnline();  // 0xBF1290
    // Times the session's local drop in. Name not in the reference map.
    void _PollSompDropIn();  // 0xBF13C0
    // Counts down the SOMP layout transition. Inlined into _Poll; the
    // binary keeps an uncalled copy. Name not in the reference map.
    void _PollSompLayoutTransition();  // 0xBF1540
    bool IsInSompLayoutTransition() const {  // 0xBF15E0
        return mRuntime.mSompLayoutTransition;
    }
    // The SOMP client's per-slot messages are forwarded to the online slots
    // by 0xBF15F0, 0xBF1720, 0xBF1840, 0xBF1950, 0xBF1A70, 0xBF1B90,
    // 0xBF1CA0, 0xBF1DA0, 0xBF1EB0, 0xBF1FC0, 0xBF20D0 and 0xBF21F0. Not
    // declared: their message records are not modelled.
    // The SOMP session changes the layout. Name not in the reference map.
    void OnSompLayoutChanged(RBUILayout layout);  // 0xBF2610
    // A kick request of the SOMP client (0xBF2670) is not declared either.
    // Runs a queued UpdateAll. Inlined into _Poll; the binary keeps an
    // uncalled copy. Name not in the reference map.
    void _PollQueuedUpdateAll();  // 0xBF2830
    // Points the slots at the "overshell_slots" objects, gives them their
    // game slots, controller types and the input component, and orders
    // the joins.
    void _FindSlots();  // 0xBF33D0
    // Handles the closed system overlay once the platform reports it gone:
    // refreshes the slots' profiles and shows the in-song menu of the first
    // slot whose user is joining. Name not in the reference map.
    void _PollSystemOverlay();  // 0xBF3A90
    void _OnLayoutTransitionDone();  // 0xBF3D90
    DataNode OnMsg(const JoypadConnectionMsg& msg);       // 0xBF3E10
    DataNode OnMsg(const GameModeChangedMsg& msg);        // 0xBF3E40
    DataNode OnMsg(const GameMicsChangedMsg& msg);        // 0xBF3E70
    DataNode OnMsg(const GameCreatedMsg& msg);            // 0xBF3F10
    DataNode OnMsg(const GameStartedMsg& msg);            // 0xBF3FB0
    DataNode OnMsg(const GameEndedMsg& msg);              // 0xBF4020
    DataNode OnMsg(const GameAboutToBeDestroyedMsg& msg); // 0xBF40B0
    DataNode OnMsg(const SystemOverlayStatusChangedMsg& msg);  // 0xBF4140
    void _OnSystemOverlayOpened();  // 0xBF4190
    void _OnSystemOverlayClosed();  // 0xBF41A0
    // Refreshes the slot's profile after the system overlay. Name not in
    // the reference map.
    void _RefreshSlotProfile(RBOvershellSlotCom* slot);  // 0xBF41B0
    // Shows "error_zero_players" while no band user takes part. Name not in
    // the reference map.
    void _ShowErrorZeroPlayersDialog();  // 0xBF42D0
    // The shortest time the online slots' users waited, or the default.
    // Name not in the reference map.
    float GetSompMinWaitTime() const;  // 0xBF45C0

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it with the other rb_meta classes' at 0xB87E00.
    static void Init();
    // The class factory, emitted at 0xBAB940; the placement factory beside
    // it (0xBAB970) is InitComponentClass's.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0xBE7EE0

    // "RBOvershellCom".
    static Symbol sId;  // 0x1B029E0
    // Also "RBOvershellCom". Name not in the reference map.
    static Symbol sClassName;           // 0x1B029E8
    static PropRegistry sPropRegistry;  // 0x1B029F0
    static ComMetaData sMetaData;       // 0x1B02A90

    RuntimeData mRuntime;
    // "overshell_slots": the four slot objects. Field name not in the
    // reference map.
    PropArray<GameObjectId> mOvershellSlots;
};

static_assert(sizeof(RBOvershellCom::OverrideFlowRequest) == 12);
static_assert(offsetof(RBOvershellCom::RuntimeData, mSlots) == 8);
static_assert(offsetof(RBOvershellCom::RuntimeData, mJoinOrder) == 40);
static_assert(offsetof(RBOvershellCom::RuntimeData, mInputCom) == 72);
static_assert(offsetof(RBOvershellCom::RuntimeData, mLostConnectionToServer) == 80);
static_assert(offsetof(RBOvershellCom::RuntimeData, mOverrideFlow) == 84);
static_assert(offsetof(RBOvershellCom::RuntimeData, mOverrideFlowRequests) == 88);
static_assert(offsetof(RBOvershellCom::RuntimeData, mActiveStatus) == 120);
static_assert(offsetof(RBOvershellCom::RuntimeData, mAllowsButtonPulse) == 127);
static_assert(offsetof(RBOvershellCom::RuntimeData, mQueuedJoinUsers) == 128);
static_assert(offsetof(RBOvershellCom::RuntimeData, mPartRand) == 160);
static_assert(offsetof(RBOvershellCom::RuntimeData, mRandomSeed) == 168);
static_assert(offsetof(RBOvershellCom::RuntimeData, mIdleMasterGuid) == 176);
static_assert(offsetof(RBOvershellCom::RuntimeData, mIdleMasterSlot) == 208);
static_assert(offsetof(RBOvershellCom::RuntimeData, mDifficultySelectMs) == 216);
static_assert(offsetof(RBOvershellCom::RuntimeData, mOnlineState) == 224);
static_assert(offsetof(RBOvershellCom::RuntimeData, mOnlineSlots) == 232);
static_assert(offsetof(RBOvershellCom::RuntimeData, mOnlineStateChanged) == 288);
static_assert(offsetof(RBOvershellCom::RuntimeData, mOnlineTransitionStartMs) == 292);
static_assert(offsetof(RBOvershellCom::RuntimeData, mMovingUser) == 300);
static_assert(offsetof(RBOvershellCom::RuntimeData, mSompLayoutTransitionTime) == 304);
static_assert(offsetof(RBOvershellCom::RuntimeData, mNeedsUpdateAll) == 308);
static_assert(offsetof(RBOvershellCom::RuntimeData, mLightLayout) == 312);
static_assert(offsetof(RBOvershellCom::RuntimeData, mTransitionDoneSink) == 320);
static_assert(offsetof(RBOvershellCom::RuntimeData, mLostConnectionToServerSink) == 960);
static_assert(sizeof(RBOvershellCom::RuntimeData) == 1000);
static_assert(offsetof(RBOvershellCom, mRuntime) == 24);
static_assert(offsetof(RBOvershellCom, mOvershellSlots) == 1024);
static_assert(sizeof(RBOvershellCom) == 1064);

// The overshell component, which its constructor sets and its destructor
// clears. Name not in the reference map.
extern RBOvershellCom* theOvershell;  // 0x1B02C38
