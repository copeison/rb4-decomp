// rb_meta/RBOvershellSlotState.o (0xC24B00 to 0xC2906F). The compiler's
// code: DataNode's destructor for GetProperty's static (0xC24D80), the
// EASTL tree helpers of the transition maps (0xC28560-0xC29018: the
// subtrees' destruction and copy, the inserts behind operator[] and the
// map assignment) and SetProperty's insert (0xC28610); the property map's
// subtree destruction is a shared instance (0x942090), and Message<0>'s
// constructor and destructor are the binary's copies (0x8D9A0, 0x8DBA0).
// The static initializer is at 0xC29030; the shared header statics it sets
// (0x1B05458-0x1B05480) belong to headers that are not modelled, apart from
// the null object id.
#include "rb_meta/overshell/RBOvershellSlotState.h"

#include "entity/core/GameObject.h"
#include "render/fonts/RndTextCom.h"
#include "ui/lists/UIListData.h"
#include "utl/data/DataFile.h"
#include "utl/messages/Message.h"

namespace {

// The id of no object, at 0x1B05458, which the static initializer sets
// with the shared header's statics. Name not in the reference map.
[[maybe_unused]] GameObjectId gNullObjectId = {0xFFFFFFFFU};

// Sends the state a message of the type, without arguments, and returns
// the handler's answer. Inlined into each query. Name not in the reference
// map.
DataNode SendStateMsg(RBOvershellSlotState* state, const char* type) {
    const Symbol messageType(type);
    Message<0> msg(messageType);
    return state->HandleMsg(msg.mData);
}

// The answer to the query as a flag, or the default when the state does
// not handle it. Inlined into each query. Name not in the reference map.
bool GetStateFlag(RBOvershellSlotState* state, const char* type, bool unhandled) {
    const DataNode result = SendStateMsg(state, type);
    if (result.Type() == kDataUnhandled) {
        return unhandled;
    }
    return result.Int(nullptr) != 0;
}

}  // namespace

// Reconstructed from eboot.elf at 0xC24B00.
RBOvershellSlotState::RBOvershellSlotState(DataArray* data, RBOvershellSlotCom* slot)
    : mData(data), mSlotCom(slot) {
    mStateID = static_cast<OvershellSlotStateID>(mData->Node(0).Int(mData));
}

// Reconstructed from eboot.elf at 0xC24BC0. The deleting destructor is at
// 0xC24C20.
RBOvershellSlotState::~RBOvershellSlotState() {}

// Reconstructed from eboot.elf at 0xC24C80.
OvershellSlotStateID RBOvershellSlotState::GetStateID() const {
    return mStateID;
}

// Reconstructed from eboot.elf at 0xC24DE0.
void RBOvershellSlotState::UpdateView() {
    SendStateMsg(this, "update_view");
}

// Reconstructed from eboot.elf at 0xC24EA0.
bool RBOvershellSlotState::AllowsInputToShell() {
    return GetStateFlag(this, "allows_input_to_shell", false);
}

// Reconstructed from eboot.elf at 0xC24F80.
bool RBOvershellSlotState::PreventsOverride() {
    return GetStateFlag(this, "prevents_override", false);
}

// Reconstructed from eboot.elf at 0xC25060.
bool RBOvershellSlotState::PreventsDropOut() {
    return GetStateFlag(this, "prevents_drop_out", false);
}

// Reconstructed from eboot.elf at 0xC25140.
bool RBOvershellSlotState::RequiresOnlineSession() {
    return GetStateFlag(this, "requires_online_session", false);
}

// Reconstructed from eboot.elf at 0xC25220.
bool RBOvershellSlotState::RetractedPosition() {
    return GetStateFlag(this, "retracted_position", false);
}

// Reconstructed from eboot.elf at 0xC25300.
bool RBOvershellSlotState::ShowsExtendedMicArrows() {
    return GetStateFlag(this, "shows_extended_mic_arrows", false);
}

// Reconstructed from eboot.elf at 0xC253E0.
bool RBOvershellSlotState::AllowsUIListInput() {
    return GetStateFlag(this, "allows_uilist_input", true);
}

// Reconstructed from eboot.elf at 0xC254C0.
bool RBOvershellSlotState::AllowsUIListScrolling() {
    bool allows = GetStateFlag(this, "allows_uilist_scrolling", true);
    if (allows && !AllowsUIListInput()) {
        // The release build keeps only the warning's argument.
        static_cast<void>(OvershellSlotStateToString(mStateID));
        allows = false;
    }
    return allows;
}

// Reconstructed from eboot.elf at 0xC255B0.
bool RBOvershellSlotState::AllowsUIListSelection() {
    bool allows = GetStateFlag(this, "allows_uilist_selection", true);
    if (allows && !AllowsUIListInput()) {
        static_cast<void>(OvershellSlotStateToString(mStateID));
        allows = false;
    }
    return allows;
}

// Reconstructed from eboot.elf at 0xC256A0.
bool RBOvershellSlotState::ForceUIListFocus() {
    bool force = GetStateFlag(this, "force_uilist_focus", false);
    if (force && RetractedPosition()) {
        static_cast<void>(OvershellSlotStateToString(mStateID));
        force = false;
    }
    return force;
}

// Reconstructed from eboot.elf at 0xC25790.
bool RBOvershellSlotState::ShowOnlineSpinner() {
    return GetStateFlag(this, "show_online_spinner", false);
}

// Reconstructed from eboot.elf at 0xC25870.
bool RBOvershellSlotState::ServerWait() {
    return GetStateFlag(this, "server_wait", false);
}

// Reconstructed from eboot.elf at 0xC25950.
bool RBOvershellSlotState::InSongSettingsFlow() {
    return GetStateFlag(this, "song_settings_flow", false);
}

// Reconstructed from eboot.elf at 0xC25A30.
bool RBOvershellSlotState::InChooseBandMembersFlow() {
    return GetStateFlag(this, "choose_band_members_flow", false);
}

// Reconstructed from eboot.elf at 0xC25B10.
bool RBOvershellSlotState::ChooseBandMembersReady() {
    return GetStateFlag(this, "choose_band_members_ready", false);
}

// Reconstructed from eboot.elf at 0xC25BF0.
bool RBOvershellSlotState::InIdleFlow() {
    return GetStateFlag(this, "idle_flow", false);
}

// Reconstructed from eboot.elf at 0xC25CD0.
bool RBOvershellSlotState::IsIdleMaster() {
    return GetStateFlag(this, "idle_flow_master", false);
}

// Reconstructed from eboot.elf at 0xC25DB0.
bool RBOvershellSlotState::IsPartUnresolved() {
    if (!InSongSettingsFlow()) {
        return false;
    }
    return GetStateFlag(this, "part_unresolved", false);
}

// Reconstructed from eboot.elf at 0xC25EA0.
bool RBOvershellSlotState::InRegisterOnlineFlow() {
    return GetStateFlag(this, "register_online_flow", false);
}

// Reconstructed from eboot.elf at 0xC25F80.
bool RBOvershellSlotState::InChooseCharFlow() {
    return GetStateFlag(this, "choose_char_flow", false);
}

// Reconstructed from eboot.elf at 0xC26060.
bool RBOvershellSlotState::IsReadyToPlay() {
    return GetStateFlag(this, "ready_to_play", false);
}

// Reconstructed from eboot.elf at 0xC26140.
bool RBOvershellSlotState::InDisabledFlow() {
    return GetStateFlag(this, "disabled_flow", false);
}

// Reconstructed from eboot.elf at 0xC26220.
bool RBOvershellSlotState::InSompTransitionFlow() {
    return GetStateFlag(this, "somp_transition_flow", false);
}

// Reconstructed from eboot.elf at 0xC26300.
bool RBOvershellSlotState::IsRemote() {
    return false;
}

// Reconstructed from eboot.elf at 0xC26310.
bool RBOvershellSlotState::HasPlayerName() {
    return false;
}

// Reconstructed from eboot.elf at 0xC26320.
Symbol RBOvershellSlotState::GetPlayerName() {
    return Symbol(gNullStr);
}

// Reconstructed from eboot.elf at 0xC26330.
bool RBOvershellSlotState::HasCharacter() {
    return false;
}

// Reconstructed from eboot.elf at 0xC26340.
Symbol RBOvershellSlotState::GetCharacter() {
    return Symbol(gNullStr);
}

// Reconstructed from eboot.elf at 0xC26350.
bool RBOvershellSlotState::HasTrackType() {
    return false;
}

// Reconstructed from eboot.elf at 0xC26360.
TrackType RBOvershellSlotState::GetTrackType() {
    return kTrackNone;
}

// Reconstructed from eboot.elf at 0xC26370.
bool RBOvershellSlotState::HasNumHarmonyMics() {
    return false;
}

// Reconstructed from eboot.elf at 0xC26380.
int RBOvershellSlotState::GetNumHarmonyMics() {
    return 0;
}

// Reconstructed from eboot.elf at 0xC26390.
bool RBOvershellSlotState::HasVocalFreestyle() {
    return false;
}

// Reconstructed from eboot.elf at 0xC263A0.
bool RBOvershellSlotState::GetVocalFreestyle() {
    return false;
}

// Reconstructed from eboot.elf at 0xC263B0.
bool RBOvershellSlotState::HasProDrums() {
    return false;
}

// Reconstructed from eboot.elf at 0xC263C0.
bool RBOvershellSlotState::GetProDrums() {
    return false;
}

// Reconstructed from eboot.elf at 0xC263D0.
bool RBOvershellSlotState::IsRecordNew() {
    return false;
}

// Reconstructed from eboot.elf at 0xC263E0.
void RBOvershellSlotState::ClearRecordNew() {}

// Reconstructed from eboot.elf at 0xC263F0.
bool RBOvershellSlotState::HasListData() {
    return false;
}

// Reconstructed from eboot.elf at 0xC26400.
void RBOvershellSlotState::FillListData(UIListData* data) {
    data->Clear();
}

// Reconstructed from eboot.elf at 0xC26410.
long RBOvershellSlotState::GetListSelection() {
    return -1;
}

// Reconstructed from eboot.elf at 0xC26420.
void RBOvershellSlotState::UpdateText(RndTextCom* text) {
    text->SetTextToken(Symbol(gNullStr));
    text->SetTextUtf8(gNullStr);
}

// Reconstructed from eboot.elf at 0xC26450.
void RBOvershellSlotState::SetProperty(Symbol name, const DataNode& value) {
    mProperties[name] = value;
}

// Reconstructed from eboot.elf at 0xC26590.
const DataNode& RBOvershellSlotState::GetProperty(Symbol name) {
    const auto found = mProperties.find(name);
    if (found != mProperties.end()) {
        return found->second;
    }
    // 0x1B054A8.
    static DataNode sUnset;
    return sUnset;
}

// Reconstructed from eboot.elf at 0xC26950.
DataNode RBOvershellSlotState::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    // The message's type, read for the release build's stripped checks.
    static_cast<void>(msg->Sym(1));
    return HandleMsg(msg);
}

// Reconstructed from eboot.elf at 0xC269A0.
RBOvershellRemoteSlotState::RBOvershellRemoteSlotState(DataArray* data, RBOvershellSlotCom* slot)
    : RBOvershellSlotState(data, slot) {}

// Reconstructed from eboot.elf at 0xC26A70, the deleting destructor; the
// complete destructor is folded with RBOvershellSlotState's (0xC24BC0).
RBOvershellRemoteSlotState::~RBOvershellRemoteSlotState() {}

// Reconstructed from eboot.elf at 0xC26DC0.
bool RBOvershellRemoteSlotState::AllowsInputToShell() {
    return false;
}

// Reconstructed from eboot.elf at 0xC26DD0.
bool RBOvershellRemoteSlotState::PreventsOverride() {
    return true;
}

// Reconstructed from eboot.elf at 0xC26DE0.
bool RBOvershellRemoteSlotState::PreventsDropOut() {
    return true;
}

// Reconstructed from eboot.elf at 0xC26DF0.
bool RBOvershellRemoteSlotState::RequiresOnlineSession() {
    return true;
}

// Reconstructed from eboot.elf at 0xC26E00.
bool RBOvershellRemoteSlotState::RetractedPosition() {
    return !_ShowsPlayerView();
}

// Reconstructed from eboot.elf at 0xC26E10.
bool RBOvershellRemoteSlotState::_ShowsPlayerView() {
    const Symbol view = _GetView();
    // 0x1B05500, 0x1B05510, 0x1B05520 and 0x1B05530.
    static Symbol sNoInstrument;
    if (sNoInstrument == Symbol()) {
        sNoInstrument = Symbol("no_instrument");
    }
    static Symbol sJoinedDefault;
    if (sJoinedDefault == Symbol()) {
        sJoinedDefault = Symbol("joined_default");
    }
    static Symbol sNotMaster;
    if (sNotMaster == Symbol()) {
        sNotMaster = Symbol("not_master");
    }
    static Symbol sJoin;
    if (sJoin == Symbol()) {
        sJoin = Symbol("join");
    }
    return view != sNoInstrument && view != sJoinedDefault && view != sNotMaster && view != sJoin;
}

// Reconstructed from eboot.elf at 0xC26FE0.
bool RBOvershellRemoteSlotState::ShowsExtendedMicArrows() {
    return false;
}

// Reconstructed from eboot.elf at 0xC26FF0.
bool RBOvershellRemoteSlotState::AllowsUIListInput() {
    return _ShowsPlayerView();
}

// Reconstructed from eboot.elf at 0xC270C0.
bool RBOvershellRemoteSlotState::HasPlayerName() {
    return true;
}

// Reconstructed from eboot.elf at 0xC27190.
bool RBOvershellRemoteSlotState::HasCharacter() {
    return true;
}

// Reconstructed from eboot.elf at 0xC27200.
bool RBOvershellRemoteSlotState::HasTrackType() {
    return true;
}

// Reconstructed from eboot.elf at 0xC27290.
bool RBOvershellRemoteSlotState::HasNumHarmonyMics() {
    return true;
}

// Reconstructed from eboot.elf at 0xC272F0.
bool RBOvershellRemoteSlotState::HasVocalFreestyle() {
    return true;
}

// Reconstructed from eboot.elf at 0xC27350.
bool RBOvershellRemoteSlotState::HasProDrums() {
    return true;
}

// Reconstructed from eboot.elf at 0xC27460.
bool RBOvershellRemoteSlotState::HasListData() {
    return true;
}

// Reconstructed from eboot.elf at 0xC27840.
bool RBOvershellRemoteSlotState::IsRemote() {
    return true;
}

// Reconstructed from eboot.elf at 0xC27850.
DataNode RBOvershellRemoteSlotState::Handle(DataArray* msg, bool warn) {
    static_cast<void>(msg->Sym(1));
    const DataNode result = RBOvershellSlotState::Handle(msg, warn);
    if (result.Type() != kDataUnhandled) {
        return result;
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0xC278C0.
RBOvershellSlotStateMgr::RBOvershellSlotStateMgr() {}

// Reconstructed from eboot.elf at 0xC27980.
void RBOvershellSlotStateMgr::Init(RBOvershellSlotCom* slot) {
    // The file's array replaces the old one; the temporary releases its
    // reference after the assignment.
    mStatesData = static_cast<DataArray*>(
        DataReadFile("ui/overshell/slot_states/slot_states.dta", true));
    for (int index = 0; index < mStatesData->Size(); ++index) {
        if (mStatesData->Node(index).Type() != kDataArray) {
            continue;
        }
        const DataArrayPtr stateData = mStatesData->Node(index).Array(mStatesData);
        // 0x1B05540.
        static Symbol sRemote;
        if (sRemote == Symbol()) {
            sRemote = Symbol("remote");
        }
        bool remote = false;
        stateData->FindData(sRemote, remote, false);
        if (remote) {
            mStates.push_back(new RBOvershellRemoteSlotState(stateData, slot));
        } else {
            mStates.push_back(
                new RBOvershellSlotState(mStatesData->Node(index).Array(mStatesData), slot));
        }
    }

    const DataArrayPtr transitions =
        DataReadFile("ui/overshell/slot_states/state_transitions.dta", true);
    for (int index = 0; index < transitions->Size(); ++index) {
        if (transitions->Node(index).Type() != kDataArray) {
            continue;
        }
        const DataArrayPtr from = transitions->Node(index).Array(transitions);
        if (from->Node(0).Type() != kDataSymbol) {
            continue;
        }
        const Symbol fromState = from->Sym(0);
        mTransitionTimes[fromState] = eastl::map<Symbol, float>();
        for (int toIndex = 1; toIndex < from->Size(); ++toIndex) {
            if (from->Node(toIndex).Type() != kDataArray) {
                continue;
            }
            const DataArrayPtr to = from->Node(toIndex).Array(from);
            if (to->Node(0).Type() == kDataSymbol && to->Node(1).Type() == kDataFloat) {
                const Symbol toState = to->Sym(0);
                const float time = to->Float(1);
                mTransitionTimes[fromState][toState] = time;
            }
        }
    }
}

// Reconstructed from eboot.elf at 0xC28360.
RBOvershellSlotStateMgr::~RBOvershellSlotStateMgr() {
    for (RBOvershellSlotState** state = mStates.begin(); state != mStates.end(); ++state) {
        delete *state;
    }
    mStates.clear();
}

// Reconstructed from eboot.elf at 0xC28410.
RBOvershellSlotState* RBOvershellSlotStateMgr::GetSlotState(OvershellSlotStateID state) {
    for (unsigned long index = 0; index < mStates.size(); ++index) {
        if (mStates[index]->GetStateID() == state) {
            return mStates[index];
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0xC28450.
float RBOvershellSlotStateMgr::GetTransitionTime(const Symbol& from, const Symbol& to) {
    if (from.Str() == gNullStr) {
        return 0.0f;
    }
    // The default time (0x12FDC80).
    const float defaultTime = 0.3f;
    const auto fromTimes = mTransitionTimes.find(from);
    if (fromTimes == mTransitionTimes.end()) {
        return defaultTime;
    }
    const auto time = fromTimes->second.find(to);
    if (time == fromTimes->second.end()) {
        return defaultTime;
    }
    return time->second;
}
