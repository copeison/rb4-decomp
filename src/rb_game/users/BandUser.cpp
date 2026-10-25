#include "rb_game/users/BandUser.h"

#include "net/OnlineID.h"
#include "os/joypads/Joypad.h"
#include "os/system/System.h"
#include "rb_game/options/GameplayOptions.h"
#include "rb_game/players/RBPlayer.h"
#include "rb_game/users/BandUserMgr.h"
#include "rb_meta/profiles/RBProfile.h"
#include "utl/data/DataArray.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

// Reconstructed from eboot.elf at 0xA3ED10.
BandUser::BandUser()
    : mDifficulty(DefaultDifficulty()),
      mTrackType(kTrackNone),
      mControllerType(kControllerNone),
      mPreferredScoreType(kScoreBand),
      mHasButtonGuitar(false),
      mHas22FretGuitar(false),
      mPlayer(nullptr),
      mOvershellSlotState(static_cast<OvershellSlotStateID>(4)),
      mAutoplay(false),
      mJustDisconnected(false),
      mDifficultyChosen(false),
      mParticipating(false) {}

// Reconstructed from eboot.elf at 0xA3ED60. The release body also interns
// "none" in a static it never reads.
void BandUser::Reset() {
    static Symbol none;
    if (none == Symbol()) {
        none = Symbol("none");
    }
    mOvershellSlotState = static_cast<OvershellSlotStateID>(4);
    mDifficulty = DefaultDifficulty();
    mDifficultyChosen = false;
    mTrackType = kTrackNone;
    mControllerType = kControllerNone;
    mAutoplay = false;
    mHasButtonGuitar = false;
    mHas22FretGuitar = false;
    mParticipating = false;
    _Reset();
}

// Reconstructed from eboot.elf at 0xA3EE40.
Difficulty BandUser::GetDifficulty() const {
    return mDifficulty;
}

// Reconstructed from eboot.elf at 0xA3EE50.
Symbol BandUser::GetDifficultySym() const {
    return DifficultyToSym(mDifficulty);
}

// Reconstructed from eboot.elf at 0xA3EE60.
void BandUser::SetDifficulty(Difficulty difficulty, bool force) {
    const Difficulty old = mDifficulty;
    mDifficulty = difficulty;
    mDifficultyChosen = true;
    if (mPlayer != nullptr && (old != difficulty || force)) {
        mPlayer->ChangeDifficulty(difficulty);
    }
}

// Reconstructed from eboot.elf at 0xA3EE90. The release build keeps the
// arguments of a log line it no longer prints.
void BandUser::ResetDifficulty() {
    mDifficulty = DefaultDifficulty();
    mDifficultyChosen = false;
    if (mPlayer != nullptr) {
        static_cast<void>(GetPlatformUserId());
        static_cast<void>(GetPlatformName());
        static_cast<void>(GetOnlineIDString());
    }
}

// Reconstructed from eboot.elf at 0xA3EF10.
String BandUser::GetOnlineIDString() const {
    OnlineID id;
    GetOnlineID(id);
    return String(id.ToString());
}

// Reconstructed from eboot.elf at 0xA3EFF0.
bool BandUser::IsFullyInGame() const {
    return mDifficultyChosen && mTrackType < kTrackNone;
}

// Reconstructed from eboot.elf at 0xA3F010.
void BandUser::SetDifficulty(Symbol difficulty, bool force) {
    SetDifficulty(SymToDifficulty(difficulty), force);
}

// Reconstructed from eboot.elf at 0xA3F060.
TrackType BandUser::GetTrackType() const {
    return mTrackType;
}

// Reconstructed from eboot.elf at 0xA3F070.
Symbol BandUser::GetTrackSym() const {
    return TrackTypeToSym(mTrackType);
}

// Reconstructed from eboot.elf at 0xA3F080. The release build keeps the
// arguments of a log line it no longer prints.
void BandUser::SetTrackType(TrackType type) {
    const TrackType old = mTrackType;
    mTrackType = type;
    _ApplyTrackVolumeSettings();
    if (old != type && mPlayer != nullptr) {
        static_cast<void>(TrackTypeToSym(old));
        static_cast<void>(TrackTypeToSym(type));
        static_cast<void>(GetPlatformUserId());
        static_cast<void>(GetPlatformName());
        static_cast<void>(GetOnlineIDString());
    }
}

// Reconstructed from eboot.elf at 0xA3F120.
void BandUser::SetTrackType(Symbol type) {
    SetTrackType(SymToTrackType(type, true));
}

// Reconstructed from eboot.elf at 0xA3F1D0.
void BandUser::ClearTrackType() {
    SetTrackType(kTrackNone);
}

// Reconstructed from eboot.elf at 0xA3F270.
ScoreType BandUser::GetBasePreferredScoreType() const {
    return GetBaseScoreType(mPreferredScoreType);
}

// Reconstructed from eboot.elf at 0xA3F280.
ScoreType BandUser::GetPreferredScoreType() const {
    return mPreferredScoreType;
}

// Reconstructed from eboot.elf at 0xA3F290.
void BandUser::SetPreferredScoreType(ScoreType type) {
    mPreferredScoreType = type;
}

// Reconstructed from eboot.elf at 0xA3F2A0.
void BandUser::ClearPreferredScoreType() {
    mPreferredScoreType = kScoreBand;
}

// Reconstructed from eboot.elf at 0xA3F2B0.
ControllerType BandUser::GetControllerType() const {
    return mControllerType;
}

// Reconstructed from eboot.elf at 0xA3F2C0.
ControllerType BandUser::ConnectedControllerType() const {
    if (!IsJoypadConnected()) {
        return kControllerNone;
    }
    static Symbol joypad;
    if (joypad == Symbol()) {
        joypad = Symbol("joypad");
    }
    static Symbol instrumentMapping;
    if (instrumentMapping == Symbol()) {
        instrumentMapping = Symbol("instrument_mapping");
    }
    const Symbol controller = JoypadControllerTypePadNum(GetPadNum());
    const DataArray* const mapping = SystemConfig(joypad, instrumentMapping)->FindArray(controller, false);
    return static_cast<ControllerType>(mapping->Int(1));
}

// Reconstructed from eboot.elf at 0xA3F410.
Symbol BandUser::GetControllerSym() const {
    return ControllerTypeToSym(GetControllerType());
}

// Reconstructed from eboot.elf at 0xA3F430.
void BandUser::SetControllerType(ControllerType type) {
    mControllerType = type;
}

// Reconstructed from eboot.elf at 0xA3F440.
void BandUser::SetControllerType(Symbol type) {
    mControllerType = SymToControllerType(type);
}

// Reconstructed from eboot.elf at 0xA3F460.
bool BandUser::GetVocalImprovPossible() const {
    if (mTrackType != kTrackVocals || mPreferredScoreType == kScoreHarmony) {
        return false;
    }
    return const_cast<BandUser*>(this)->GetGameplayOptions()->IsVocalImprovEnabled();
}

// Reconstructed from eboot.elf at 0xA3F490.
unsigned long BandUser::GetSlot() const {
    return theBandUserMgr->GetSlot(mUserGuid);
}

// Reconstructed from eboot.elf at 0xA3F4B0.
bool BandUser::IsOvershellSlotOpen() const {
    return mOvershellSlotState != static_cast<OvershellSlotStateID>(4);
}

// Reconstructed from eboot.elf at 0xA3F4C0.
void BandUser::SetOvershellSlotState(OvershellSlotStateID state) {
    mOvershellSlotState = state;
}

// Reconstructed from eboot.elf at 0xA3F4D0.
bool BandUser::IsOvershellSlotClosed() const {
    return IsOvershellSlotStateClosed(GetOvershellSlotState());
}

// Reconstructed from eboot.elf at 0xA3F500. The states are
// kState_JoinedDefault (4), kState_Idle (46) and kState_Disabled (52), the
// names of OvershellSlotStateToString's table.
bool BandUser::IsOvershellSlotStateClosed(OvershellSlotStateID state) {
    return state == 4 || state == 46 || state == 52;
}

// Reconstructed from eboot.elf at 0xA3F520.
void BandUser::GetOnlineID(OnlineID& id) const {
    const RBProfile* const profile = GetProfile(false);
    if (profile != nullptr) {
        id = OnlineID(profile->mOnlineID);
        return;
    }
    _GetOnlineIDWithoutProfile(id);
}

// Reconstructed from eboot.elf at 0xA3F5E0.
void BandUser::_GetOnlineIDWithoutProfile(OnlineID& id) const {
    id = OnlineID();
}

// Reconstructed from eboot.elf at 0xA3F650. A number is the difficulty, a
// symbol or string its name.
DataNode BandUser::OnSetDifficulty(DataArray* msg) {
    const DataNode value = msg->Evaluate(2);
    if (value.Type() == kDataSymbol || value.Type() == kDataString) {
        SetDifficulty(SymToDifficulty(value.Sym(nullptr)), false);
    } else if (value.Type() == kDataInt) {
        SetDifficulty(static_cast<Difficulty>(value.Int(nullptr)), false);
    }
    return DataNode(1);
}

// Reconstructed from eboot.elf at 0xA3F750.
DataNode BandUser::OnSetTrackType(DataArray* msg) {
    const DataNode value = msg->Evaluate(2);
    if (value.Type() == kDataSymbol || value.Type() == kDataString) {
        SetTrackType(value.Sym(nullptr));
    } else if (value.Type() == kDataInt) {
        SetTrackType(static_cast<TrackType>(value.Int(nullptr)));
    }
    return DataNode(1);
}

// Reconstructed from eboot.elf at 0xA3F8A0.
DataNode BandUser::OnSetPreferredScoreType(DataArray* msg) {
    const DataNode value = msg->Evaluate(2);
    if (value.Type() == kDataInt) {
        SetPreferredScoreType(static_cast<ScoreType>(value.Int(nullptr)));
    }
    return DataNode(1);
}

// Reconstructed from eboot.elf at 0xA3F970.
DataNode BandUser::OnSetControllerType(DataArray* msg) {
    const DataNode value = msg->Evaluate(2);
    if (value.Type() == kDataSymbol || value.Type() == kDataString) {
        SetControllerType(value.Sym(nullptr));
    } else if (value.Type() == kDataInt) {
        SetControllerType(static_cast<ControllerType>(value.Int(nullptr)));
    }
    return DataNode(1);
}

// Reconstructed from eboot.elf at 0xA3FA50.
DataNode BandUser::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol getDifficulty;
    if (getDifficulty == Symbol()) {
        getDifficulty = Symbol("get_difficulty");
    }
    if (type == getDifficulty) {
        return DataNode(static_cast<int>(mDifficulty));
    }
    static Symbol getDifficultySym;
    if (getDifficultySym == Symbol()) {
        getDifficultySym = Symbol("get_difficulty_sym");
    }
    if (type == getDifficultySym) {
        return DataNode(GetDifficultySym());
    }
    static Symbol setDifficulty;
    if (setDifficulty == Symbol()) {
        setDifficulty = Symbol("set_difficulty");
    }
    if (type == setDifficulty) {
        DataNode result = OnSetDifficulty(msg);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol getSlotNum;
    if (getSlotNum == Symbol()) {
        getSlotNum = Symbol("get_slot_num");
    }
    if (type == getSlotNum) {
        return DataNode(static_cast<int>(GetSlot()));
    }
    static Symbol getTrackType;
    if (getTrackType == Symbol()) {
        getTrackType = Symbol("get_track_type");
    }
    if (type == getTrackType) {
        return DataNode(static_cast<int>(mTrackType));
    }
    static Symbol getTrackSym;
    if (getTrackSym == Symbol()) {
        getTrackSym = Symbol("get_track_sym");
    }
    if (type == getTrackSym) {
        return DataNode(GetTrackSym());
    }
    static Symbol setTrackType;
    if (setTrackType == Symbol()) {
        setTrackType = Symbol("set_track_type");
    }
    if (type == setTrackType) {
        DataNode result = OnSetTrackType(msg);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol getPreferredScoreType;
    if (getPreferredScoreType == Symbol()) {
        getPreferredScoreType = Symbol("get_preferred_score_type");
    }
    if (type == getPreferredScoreType) {
        return DataNode(static_cast<int>(mPreferredScoreType));
    }
    static Symbol setPreferredScoreType;
    if (setPreferredScoreType == Symbol()) {
        setPreferredScoreType = Symbol("set_preferred_score_type");
    }
    if (type == setPreferredScoreType) {
        DataNode result = OnSetPreferredScoreType(msg);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol getControllerType;
    if (getControllerType == Symbol()) {
        getControllerType = Symbol("get_controller_type");
    }
    if (type == getControllerType) {
        return DataNode(static_cast<int>(GetControllerType()));
    }
    static Symbol getControllerSym;
    if (getControllerSym == Symbol()) {
        getControllerSym = Symbol("get_controller_sym");
    }
    if (type == getControllerSym) {
        return DataNode(GetControllerSym());
    }
    static Symbol setControllerType;
    if (setControllerType == Symbol()) {
        setControllerType = Symbol("set_controller_type");
    }
    if (type == setControllerType) {
        DataNode result = OnSetControllerType(msg);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    static Symbol isParticipating;
    if (isParticipating == Symbol()) {
        isParticipating = Symbol("is_participating");
    }
    if (type == isParticipating) {
        return DataNode(static_cast<int>(mParticipating));
    }
    DataNode result = User::Handle(msg, false);
    if (result.Type() != kDataUnhandled) {
        return result;
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0xA400D0.
BandUser::~BandUser() {}
