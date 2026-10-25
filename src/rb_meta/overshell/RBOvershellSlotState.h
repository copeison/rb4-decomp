#pragma once

#include <cstddef>

#include "rb_beatmatch/tracks/TrackType.h"
#include "rb_meta/overshell/RBOvershellDefines.h"
#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

class RBOvershellSlotCom;
class RemoteBandUser;
class RndTextCom;
class UIListData;

// One state of an overshell slot (rb_meta/RBOvershellSlotState.o,
// 0xC24B00-0xC2906F with the static initializer at 0xC29030). Each state is
// an entry of ui/overshell/slot_states/slot_states.dta: (state_id handlers...)
// whose handler arrays answer the state's script messages, run with the
// slot as the object and $overshell set to the overshell. Most queries send
// the state a message ("allows_input_to_shell", "idle_flow", ...) and read
// the handler's answer, falling back to a default when the state does not
// handle it. The map's build declares only the destructors and Handle
// virtual (its vtable has three slots); this build makes every query
// virtual, and the vtable at 0x1972468 has 49 slots. The object is 88
// bytes. Field names are not in the reference map.
class RBOvershellSlotState : public MsgSink {
public:
    // Reads the state's id from the entry's first node.
    RBOvershellSlotState(DataArray* data, RBOvershellSlotCom* slot);  // 0xC24B00
    // Slots 0-1: 0xC24BC0, 0xC24C20.
    ~RBOvershellSlotState() override;
    // Slot 2: HandleMsg; the message's type is read for the release
    // build's stripped checks.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xC26950
    // Slot 4: the slot's own view when it has one, else the state's
    // "view" answer. Not reconstructed: it calls an RBOvershellSlotCom
    // member (0xC1CD00) that is not declared yet.
    virtual Symbol GetView();  // 0xC24C90
    // Slot 5: sends "update_view".
    virtual void UpdateView();  // 0xC24DE0
    // Slots 6-12: the state's answers, false when unhandled.
    virtual bool AllowsInputToShell();     // 0xC24EA0
    virtual bool PreventsOverride();       // 0xC24F80
    virtual bool PreventsDropOut();        // 0xC25060
    virtual bool RequiresOnlineSession();  // 0xC25140
    virtual bool RetractedPosition();      // 0xC25220
    virtual bool IsReadyToPlay();          // 0xC26060
    virtual bool ShowsExtendedMicArrows(); // 0xC25300
    // Slot 13: "allows_uilist_input", true when unhandled.
    virtual bool AllowsUIListInput();  // 0xC253E0
    // Slots 14-15: true when unhandled; never while the list takes no
    // input. Names not in the reference map.
    virtual bool AllowsUIListScrolling();  // 0xC254C0
    virtual bool AllowsUIListSelection();  // 0xC255B0
    // Slot 16: false when unhandled; never in the retracted position. Name
    // not in the reference map.
    virtual bool ForceUIListFocus();  // 0xC256A0
    // Slots 17-18: false when unhandled. Names not in the reference map.
    virtual bool ShowOnlineSpinner();  // 0xC25790
    virtual bool ServerWait();         // 0xC25870

    // Slots 19-36: what the slot shows of a remote user, from the record
    // the remote console sends (RBOvershellRemoteSlotState). The slot asks
    // HasX before GetX and otherwise uses its own value. A local state has
    // none of them. Names not in the reference map; the slot's fields that
    // the values replace are the evidence, which is weak for every name.
    // Slots 19-20: the player's name; the slot falls back to its user's.
    virtual bool HasPlayerName();     // 0xC26310
    virtual Symbol GetPlayerName();   // 0xC26320
    // Slots 21-22: the character; the slot keeps it with the user's
    // character records.
    virtual bool HasCharacter();     // 0xC26330
    virtual Symbol GetCharacter();   // 0xC26340
    // Slots 23-24: the part, kTrackNone without one.
    virtual bool HasTrackType();         // 0xC26350
    virtual TrackType GetTrackType();    // 0xC26360
    // Slots 25-26: the number of harmony mics to display (the slot's
    // value comes from the song's metadata).
    virtual bool HasNumHarmonyMics();  // 0xC26370
    virtual int GetNumHarmonyMics();   // 0xC26380
    // Slots 27-28: whether the vocal freestyle (improv) icon shows.
    virtual bool HasVocalFreestyle();  // 0xC26390
    virtual bool GetVocalFreestyle();  // 0xC263A0
    // Slots 29-30: whether the part plays pro drums (the slot's value is
    // the score type kScoreRealDrum).
    virtual bool HasProDrums();  // 0xC263B0
    virtual bool GetProDrums();  // 0xC263C0
    // Slots 31-32: whether a new record arrived since the slot last read
    // it, and marking it read.
    virtual bool IsRecordNew();      // 0xC263D0
    virtual void ClearRecordNew();   // 0xC263E0
    // Slots 33-34: whether the state fills the slot's list, and filling it
    // (a local state only clears it).
    virtual bool HasListData();                   // 0xC263F0
    virtual void FillListData(UIListData* data);  // 0xC26400
    // Slot 35: the list's selected row, -1 for none.
    virtual long GetListSelection();  // 0xC26410
    // Slot 36: sets the slot's text component (a local state clears it).
    virtual void UpdateText(RndTextCom* text);  // 0xC26420

    // Slots 37-46: whether the slot is in a flow, false when unhandled.
    virtual bool InSongSettingsFlow();  // 0xC25950
    // Slot 38: in the song settings flow, "part_unresolved".
    virtual bool IsPartUnresolved();         // 0xC25DB0
    virtual bool InRegisterOnlineFlow();     // 0xC25EA0
    virtual bool InChooseCharFlow();         // 0xC25F80
    virtual bool InChooseBandMembersFlow();  // 0xC25A30
    virtual bool ChooseBandMembersReady();   // 0xC25B10
    virtual bool InIdleFlow();               // 0xC25BF0
    // Slot 44: "idle_flow_master".
    virtual bool IsIdleMaster();  // 0xC25CD0
    // Slots 45-46. Names not in the reference map.
    virtual bool InDisabledFlow();        // 0xC26140
    virtual bool InSompTransitionFlow();  // 0xC26220
    // Slot 47: whether the state shows a remote user. Name not in the
    // reference map.
    virtual bool IsRemote();  // 0xC26300
    // Slot 48: sets $overshell to the slot's overshell and runs the
    // handler named by the message's type with the slot as the object;
    // unhandled without one. Not reconstructed: it reads the slot's
    // overshell and passes the slot as its Component, and
    // RBOvershellSlotCom is not declared yet.
    virtual DataNode HandleMsg(DataArray* msg);  // 0xC26680

    OvershellSlotStateID GetStateID() const;  // 0xC24C80
    // Sets a value the state's handlers read.
    void SetProperty(Symbol name, const DataNode& value);  // 0xC26450
    // The value, or an unhandled node when it was never set. The map does
    // not record the return type.
    const DataNode& GetProperty(Symbol name);  // 0xC26590

    // The entry of slot_states.dta.
    DataArrayPtr mData;
    OvershellSlotStateID mStateID;
    RBOvershellSlotCom* mSlotCom;
    // The values SetProperty sets.
    eastl::map<Symbol, DataNode> mProperties;
};

static_assert(offsetof(RBOvershellSlotState, mData) == 8);
static_assert(offsetof(RBOvershellSlotState, mStateID) == 16);
static_assert(offsetof(RBOvershellSlotState, mSlotCom) == 24);
static_assert(offsetof(RBOvershellSlotState, mProperties) == 32);
static_assert(sizeof(RBOvershellSlotState) == 88);

// The state of a slot that shows a remote user of a SOMP session: the
// entries of slot_states.dta with (remote TRUE). Its answers come from the
// session record the remote console sends (RemoteBandUser's
// mSessionRecord) instead of the handlers. Newer than the map; the class
// name is not in the reference map. Its vtable at 0x19725F0 has 49 slots;
// the complete destructor is folded with RBOvershellSlotState's. Most of it
// is not reconstructed: it reads the slot's user (an inlined
// RBOvershellSlotCom member, through the slot's index at 0xA8) and the
// session record, which RemoteBandUser does not model.
class RBOvershellRemoteSlotState : public RBOvershellSlotState {
public:
    RBOvershellRemoteSlotState(DataArray* data, RBOvershellSlotCom* slot);  // 0xC269A0
    // Slots 0-1: 0xC24BC0 (folded), 0xC26A70.
    ~RBOvershellRemoteSlotState() override;
    // Slot 2: RBOvershellSlotState's, inlined.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xC27850
    // Slot 4: _GetView, merged into a tail call.
    Symbol GetView() override;  // 0xC26B90
    // Slots 6-9: a remote user's slot never takes the local input and
    // always holds the slot online.
    bool AllowsInputToShell() override;     // 0xC26DC0
    bool PreventsOverride() override;       // 0xC26DD0
    bool PreventsDropOut() override;        // 0xC26DE0
    bool RequiresOnlineSession() override;  // 0xC26DF0
    // Slot 10: retracted unless _ShowsPlayerView.
    bool RetractedPosition() override;  // 0xC26E00
    bool ShowsExtendedMicArrows() override;  // 0xC26FE0
    // Slot 13: _ShowsPlayerView, merged into a tail call.
    bool AllowsUIListInput() override;  // 0xC26FF0
    // Slot 17: while the slot's online state is pending. Not
    // reconstructed (RBOvershellSlotCom).
    bool ShowOnlineSpinner() override;  // 0xC27000
    // Slots 19-36: the session record's values. The getters are not
    // reconstructed (the slot's user and the record).
    bool HasPlayerName() override;  // 0xC270C0
    // The record's name, else the slot's user's platform name.
    Symbol GetPlayerName() override;  // 0xC270D0
    bool HasCharacter() override;  // 0xC27190
    Symbol GetCharacter() override;  // 0xC271A0
    bool HasTrackType() override;  // 0xC27200
    // The record's part, else the slot's user's.
    TrackType GetTrackType() override;  // 0xC27210
    bool HasNumHarmonyMics() override;  // 0xC27290
    int GetNumHarmonyMics() override;   // 0xC272A0
    bool HasVocalFreestyle() override;  // 0xC272F0
    bool GetVocalFreestyle() override;  // 0xC27300
    bool HasProDrums() override;  // 0xC27350
    bool GetProDrums() override;  // 0xC27360
    // RemoteBandUser::mSessionRecordStamped and ClearSessionRecordStamp.
    bool IsRecordNew() override;     // 0xC273B0
    void ClearRecordNew() override;  // 0xC27400
    bool HasListData() override;  // 0xC27460
    // Slot 34: _FillListData, merged into a tail call.
    void FillListData(UIListData* data) override;  // 0xC27470
    long GetListSelection() override;  // 0xC276D0
    // Slot 36: unless the slot (0xC1D130) keeps its own text, the record's
    // token and text.
    void UpdateText(RndTextCom* text) override;  // 0xC27790
    bool IsRemote() override;  // 0xC27840

    // The rest is not reconstructed. Names not in the reference map.
    // Whether the slot has a user. Not called.
    bool HasUser();  // 0xC26AD0
    // The slot's user when it is remote, else null. Not called.
    RemoteBandUser* GetRemoteUser();  // 0xC26AF0
    // The remote user's session record, else null. Not called.
    const void* GetSessionRecord();  // 0xC26B40
    // "not_master" in the disabled override flow, else the slot's own view
    // (0xC1CD00) when it has one, else the record's view when the slot
    // accepts the record (0xC1A1C0), else "no_instrument" without a user,
    // "not_master" in the idle flow and "joined_default" otherwise.
    Symbol _GetView();  // 0xC26BA0
    // Fills the list's rows (columns 0, 7 and 2) from the record's up to
    // eight entries while _ShowsPlayerView.
    void _FillListData(UIListData* data);  // 0xC27480
    // GetListSelection's body again. Not called.
    long _GetListSelection();  // 0xC27730

    // Whether the view shows the player: none of "no_instrument",
    // "joined_default", "not_master" and "join". Name not in the reference
    // map.
    bool _ShowsPlayerView();  // 0xC26E10
};

static_assert(sizeof(RBOvershellRemoteSlotState) == 88);

// A slot's states and the times of the transitions between them. Not
// polymorphic; each RBOvershellSlotCom owns one. The object is 96 bytes.
// Field names are not in the reference map.
class RBOvershellSlotStateMgr {
public:
    RBOvershellSlotStateMgr();   // 0xC278C0
    ~RBOvershellSlotStateMgr();  // 0xC28360

    // Creates the states of slot_states.dta for the slot and reads
    // state_transitions.dta: (from (to seconds)...).
    void Init(RBOvershellSlotCom* slot);  // 0xC27980
    // The state with the id, or null.
    RBOvershellSlotState* GetSlotState(OvershellSlotStateID state);  // 0xC28410
    // The transition's time; 0 from no state, 0.3 when not listed.
    float GetTransitionTime(const Symbol& from, const Symbol& to);  // 0xC28450

    eastl::vector<RBOvershellSlotState*> mStates;
    // slot_states.dta, which the states share.
    DataArrayPtr mStatesData;
    eastl::map<Symbol, eastl::map<Symbol, float>> mTransitionTimes;
};

static_assert(offsetof(RBOvershellSlotStateMgr, mStatesData) == 32);
static_assert(offsetof(RBOvershellSlotStateMgr, mTransitionTimes) == 40);
static_assert(sizeof(RBOvershellSlotStateMgr) == 96);
