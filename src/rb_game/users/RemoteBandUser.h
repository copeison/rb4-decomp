#pragma once

#include <cstddef>

#include "os/users/UserGuid.h"
#include "rb_game/options/GameplayOptions.h"
#include "rb_game/users/RBBandUser.h"
#include "utl/text/Symbol.h"

// A band user who plays on another console in an online (SOMP) session
// (0x9A48C0-0x9A5920): its name, online id, options and character come
// from the session's member record. The band user manager creates three.
// Newer than the map; the class name is not in the reference map (the older
// game's RemoteBandUser had the same role) and the evidence for it is weak.
// The vtable at 0x19621C0 has 43 slots; the object is 1352 bytes. Only the
// small members are reconstructed; the session record is not modelled.
class RemoteBandUser : public RBBandUser {
public:
    RemoteBandUser();  // 0x9A48C0
    // Slots 0-1: 0x9A5530, 0x9A55C0.
    ~RemoteBandUser() override;
    // Slot 5: a remote user has no joypad here.
    bool IsJoypadConnected() const override;  // 0x9A5650
    // Slot 8: the controller of the user's band slot.
    ControllerType GetControllerType() const override;  // 0x9A52E0
    // Slot 12.
    bool IsRemoteJoined() const override;  // 0x9A5660
    // Slot 13: the session's name for the user, else RBBandUser's.
    const char* GetPlatformName() const override;  // 0x9A51E0
    // Slots 14-17: the overshell slot of a remote user stays in state 61.
    bool IsOvershellSlotOpen() const override;  // 0x9A5670
    OvershellSlotStateID GetOvershellSlotState() const override;  // 0x9A52B0
    void SetOvershellSlotState(OvershellSlotStateID state) override;  // 0x9A52C0
    bool IsOvershellSlotClosed() const override;  // 0x9A52D0
    // Slot 18: also forgets the session record. Not reconstructed.
    void _Reset() override;  // 0x9A5680
    // Slot 19: the options the session sent.
    IGameplayOptions* GetGameplayOptions() override;  // 0x9A5800
    // Slot 21: a remote user has no profile.
    RBProfile* GetProfile(bool any) const override;  // 0x9A5810
    // Slot 23: the online id the session sent, else an unset one. Not
    // reconstructed.
    void _GetOnlineIDWithoutProfile(OnlineID& id) const override;  // 0x9A5310
    // Slot 24.
    bool IsLocal() const override;  // 0x9A5820
    // Slots 25-26 and 29: the session record's preferences. Not
    // reconstructed.
    Symbol GetPreferenceSymbol() const override;  // 0x9A53B0
    long GetPreferenceIndex() const override;  // 0x9A5450
    bool GetPreferenceFlag0() const override;  // 0x9A54E0
    // Slots 41-42: two flags of the session record.
    bool IsSompLagging() const override;  // 0x9A5900
    bool IsSompIdle() const override;  // 0x9A5910

    // Setters of the record values below. Names not in the reference map.
    void SetRemoteJoined(bool joined);  // 0x9A5200
    void SetSessionValue(float value);  // 0x9A5210
    // Copies the 264-byte record the session sent and stamps the time.
    // Not reconstructed.
    void SetSessionRecord(const void* record);  // 0x9A5220
    void ClearSessionRecordStamp();  // 0x9A52A0

    // Field names are not in the reference map. The member record the
    // session sends (names, character, scores); not modelled.
    unsigned char mSessionData[600];
    // The options the session sent.
    GameplayOptions mOptions;
    // The owner's id and a session member id (-1 until set).
    UserGuid mOwnerGuid;
    unsigned char mMemberId[16];
    // The name and the online id text the session sent.
    Symbol mName;
    Symbol mOnlineIDText;
    bool mRemoteJoined;
    float mSessionValue;
    unsigned char mSessionRecord[264];
    // When the record arrived, in milliseconds.
    float mSessionRecordTime;
    bool mSessionRecordStamped;
    bool mSessionFlags[3];
    unsigned char mSessionState[8];
};

static_assert(offsetof(RemoteBandUser, mOptions) == 832);
static_assert(offsetof(RemoteBandUser, mOwnerGuid) == 1016);
static_assert(offsetof(RemoteBandUser, mName) == 1048);
static_assert(offsetof(RemoteBandUser, mOnlineIDText) == 1056);
static_assert(offsetof(RemoteBandUser, mRemoteJoined) == 1064);
static_assert(offsetof(RemoteBandUser, mSessionValue) == 1068);
static_assert(offsetof(RemoteBandUser, mSessionRecord) == 1072);
static_assert(offsetof(RemoteBandUser, mSessionRecordTime) == 1336);
static_assert(offsetof(RemoteBandUser, mSessionRecordStamped) == 1340);
static_assert(sizeof(RemoteBandUser) == 1352);
