#pragma once

#include <cstddef>

#include "os/users/User.h"
#include "rb_beatmatch/core/Defines.h"
#include "rb_beatmatch/tracks/TrackType.h"
#include "rb_game/core/Defines.h"
#include "rb_meta/overshell/RBOvershellDefines.h"

class DataArray;
class IGameplayOptions;
class IRBAchievementLogger;
class OnlineID;
class RBPlayer;
class RBProfile;
class String;

// A player of the band (rb_game/BandUser.o, 0xA3ED10-0xA40130 with the
// static initializer at 0xA400F0): the part, difficulty, controller and
// score type the user plays, the user's gameplay player and overshell slot
// state. It answers the get/set difficulty, track, score type and
// controller messages, "get_slot_num" and "is_participating". The map's
// character code moved into RBBandUser, and the profile, options and
// player name are pure in this build. The vtable at 0x1968498 has 24 slots
// (slots 9-13 and 18-22 pure); the object is 72 bytes.
//
// The static initializer only builds statics of headers the object
// includes (an id of -1 with a record at 0x1AEF43C, and a -1 pair with a
// record at 0x1AEF460) that nothing in the object reads; it is not
// reconstructed.
class BandUser : public User {
public:
    BandUser();  // 0xA3ED10
    // Slots 0-1: 0xA400D0, 0xA400E0.
    ~BandUser() override;
    // Slot 2: the get/set difficulty, track type, preferred score type and
    // controller messages, "get_slot_num" and "is_participating", then
    // User's.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xA3FA50
    // Slot 8: the controller the user plays.
    virtual ControllerType GetControllerType() const;  // 0xA3F2B0
    // Slots 9-11: the user's flags in the online (SOMP) session, copied from
    // the session's member records by the band user manager. Slot 10 is
    // set by the update that exports "somp_leader_changed". Names not in the
    // reference map; the meanings of slots 9 and 11 are not recovered and
    // the evidence for all three is weak.
    virtual bool IsSompHost() const = 0;
    virtual bool IsSompLeader() const = 0;
    virtual bool IsSompReady() const = 0;
    // Slot 12: false for the local users; a remote user answers a flag its
    // session sets, true when created. Name not in the reference map; the
    // evidence is weak.
    virtual bool IsRemoteJoined() const = 0;
    // Slot 13: the name to show for the user: the profile's platform name,
    // or the localized "guest". The map's name; this build makes it pure.
    virtual const char* GetPlatformName() const = 0;
    // Slot 14: whether the overshell slot left its default joined state.
    // Name not in the reference map; the evidence is weak.
    virtual bool IsOvershellSlotOpen() const;  // 0xA3F4B0
    // Slot 15: the overshell slot's state. Inline; the copy kept is at
    // 0x92B8B0. Name not in the reference map.
    virtual OvershellSlotStateID GetOvershellSlotState() const {
        return mOvershellSlotState;
    }
    // Slot 16.
    virtual void SetOvershellSlotState(OvershellSlotStateID state);  // 0xA3F4C0
    // Slot 17: whether the overshell slot is in its default joined state,
    // idle or disabled. Name not in the reference map; the evidence is
    // weak.
    virtual bool IsOvershellSlotClosed() const;  // 0xA3F4D0
    // Slot 18: the derived classes' part of Reset. Name not in the
    // reference map.
    virtual void _Reset() = 0;
    // Slot 19: the options the user plays with: the profile's, or the
    // user's own.
    virtual IGameplayOptions* GetGameplayOptions() = 0;
    // Slot 20: the profile's achievement logger (its achievement data), or
    // null. Name not in the reference map.
    virtual IRBAchievementLogger* GetAchievementData() const = 0;
    // Slot 21: the profile of the user's band slot; `any` also looks among
    // the profiles of the user's pad when the user has no slot. The map has
    // GetProfile() const.
    virtual RBProfile* GetProfile(bool any) const = 0;
    // Slot 22: applies the volume settings of the user's part after the
    // part changed. Name not in the reference map; the evidence is weak.
    virtual void _ApplyTrackVolumeSettings() = 0;
    // Slot 23: the user's online id when no profile gives it; unset here.
    // Name not in the reference map.
    virtual void _GetOnlineIDWithoutProfile(OnlineID& id) const;  // 0xA3F5E0

    // Restores the defaults: no part, difficulty, controller or overshell
    // state, not participating; then the derived classes' _Reset.
    void Reset();  // 0xA3ED60
    Difficulty GetDifficulty() const;  // 0xA3EE40
    Symbol GetDifficultySym() const;  // 0xA3EE50
    // Marks the difficulty chosen and, when it changed or `force` is set,
    // tells the player. The map has SetDifficulty(Difficulty).
    void SetDifficulty(Difficulty difficulty, bool force);  // 0xA3EE60
    // Restores the default difficulty and marks it not chosen. Name not in
    // the reference map.
    void ResetDifficulty();  // 0xA3EE90
    // The user's online id as text. Name not in the reference map; the
    // map's GetPlatformName (0x160 bytes) may be this function.
    String GetOnlineIDString() const;  // 0xA3EF10
    // Whether the difficulty was chosen and the user has a part.
    bool IsFullyInGame() const;  // 0xA3EFF0
    // The map has SetDifficulty(Symbol).
    void SetDifficulty(Symbol difficulty, bool force);  // 0xA3F010
    // The track the user plays.
    TrackType GetTrackType() const;  // 0xA3F060
    Symbol GetTrackSym() const;  // 0xA3F070
    void SetTrackType(TrackType type);  // 0xA3F080
    void SetTrackType(Symbol type);  // 0xA3F120
    // Sets kTrackNone. Name not in the reference map.
    void ClearTrackType();  // 0xA3F1D0
    // The preferred score type with the pro drum and harmony scores folded
    // in. Name not in the reference map.
    ScoreType GetBasePreferredScoreType() const;  // 0xA3F270
    ScoreType GetPreferredScoreType() const;  // 0xA3F280
    void SetPreferredScoreType(ScoreType type);  // 0xA3F290
    // Sets kScoreBand. Name not in the reference map.
    void ClearPreferredScoreType();  // 0xA3F2A0
    // The kind of instrument connected on the user's pad, from the
    // "joypad" "instrument_mapping" config; kControllerNone without a
    // joypad.
    ControllerType ConnectedControllerType() const;  // 0xA3F2C0
    Symbol GetControllerSym() const;  // 0xA3F410
    void SetControllerType(ControllerType type);  // 0xA3F430
    void SetControllerType(Symbol type);  // 0xA3F440
    // Whether the user sings the lead vocals with vocal improvisation on.
    bool GetVocalImprovPossible() const;  // 0xA3F460
    // The user's band slot, looked up by the user's id in theBandUserMgr
    // (BandUserMgr::GetSlot(UserGuid const&) const).
    unsigned long GetSlot() const;  // 0xA3F490
    // Whether the overshell slot state is the default joined state, idle or
    // disabled. Name not in the reference map.
    static bool IsOvershellSlotStateClosed(OvershellSlotStateID state);  // 0xA3F500
    // The user's online id: the profile's, else the derived class's. Name
    // not in the reference map.
    void GetOnlineID(OnlineID& id) const;  // 0xA3F520

    // The message handlers; each returns 1.
    DataNode OnSetDifficulty(DataArray* msg);  // 0xA3F650
    DataNode OnSetTrackType(DataArray* msg);  // 0xA3F750
    DataNode OnSetPreferredScoreType(DataArray* msg);  // 0xA3F8A0
    DataNode OnSetControllerType(DataArray* msg);  // 0xA3F970

    // Field names are not in the reference map.
    Difficulty mDifficulty;
    TrackType mTrackType;
    ControllerType mControllerType;
    ScoreType mPreferredScoreType;
    // Cleared by the constructor and Reset, read by nothing in this build.
    // Named after the map's SetHasButtonGuitar(bool) and the older
    // BandUser's flags; the evidence is weak.
    bool mHasButtonGuitar;
    bool mHas22FretGuitar;
    // The gameplay player, while the user plays a song.
    RBPlayer* mPlayer;
    OvershellSlotStateID mOvershellSlotState;
    // Cleared by Reset; named after the older BandUser's field order, and
    // the evidence is weak.
    bool mAutoplay;
    // Not written by the band user code. The evidence for the name, taken
    // from the older BandUser, is weak.
    bool mJustDisconnected;
    // Set by SetDifficulty, cleared by Reset and ResetDifficulty.
    bool mDifficultyChosen;
    // "is_participating"; the band user manager counts and filters on it.
    bool mParticipating;
};

static_assert(offsetof(BandUser, mDifficulty) == 32);
static_assert(offsetof(BandUser, mTrackType) == 36);
static_assert(offsetof(BandUser, mControllerType) == 40);
static_assert(offsetof(BandUser, mPreferredScoreType) == 44);
static_assert(offsetof(BandUser, mHasButtonGuitar) == 48);
static_assert(offsetof(BandUser, mPlayer) == 56);
static_assert(offsetof(BandUser, mOvershellSlotState) == 64);
static_assert(offsetof(BandUser, mAutoplay) == 68);
static_assert(offsetof(BandUser, mDifficultyChosen) == 70);
static_assert(offsetof(BandUser, mParticipating) == 71);
static_assert(sizeof(BandUser) == 72);
