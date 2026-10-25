#include "rb_game/users/RemoteBandUser.h"

// Reconstructed from eboot.elf at 0x9A51E0.
const char* RemoteBandUser::GetPlatformName() const {
    if (mName != Symbol()) {
        return mName.Str();
    }
    return RBBandUser::GetPlatformName();
}

// Reconstructed from eboot.elf at 0x9A5200.
void RemoteBandUser::SetRemoteJoined(bool joined) {
    mRemoteJoined = joined;
}

// Reconstructed from eboot.elf at 0x9A5210.
void RemoteBandUser::SetSessionValue(float value) {
    mSessionValue = value;
}

// Reconstructed from eboot.elf at 0x9A52A0.
void RemoteBandUser::ClearSessionRecordStamp() {
    mSessionRecordStamped = false;
}

// Reconstructed from eboot.elf at 0x9A52B0.
OvershellSlotStateID RemoteBandUser::GetOvershellSlotState() const {
    return static_cast<OvershellSlotStateID>(61);
}

// Reconstructed from eboot.elf at 0x9A52C0.
void RemoteBandUser::SetOvershellSlotState(OvershellSlotStateID state) {
    static_cast<void>(state);
}

// Reconstructed from eboot.elf at 0x9A52D0.
bool RemoteBandUser::IsOvershellSlotClosed() const {
    return false;
}

// Reconstructed from eboot.elf at 0x9A52E0.
ControllerType RemoteBandUser::GetControllerType() const {
    return GameSlotToControllerType(static_cast<GameSlot>(GetSlot()));
}

// Reconstructed from eboot.elf at 0x9A5650.
bool RemoteBandUser::IsJoypadConnected() const {
    return false;
}

// Reconstructed from eboot.elf at 0x9A5660.
bool RemoteBandUser::IsRemoteJoined() const {
    return mRemoteJoined;
}

// Reconstructed from eboot.elf at 0x9A5670.
bool RemoteBandUser::IsOvershellSlotOpen() const {
    return false;
}

// Reconstructed from eboot.elf at 0x9A5800.
IGameplayOptions* RemoteBandUser::GetGameplayOptions() {
    return &mOptions;
}

// Reconstructed from eboot.elf at 0x9A5810.
RBProfile* RemoteBandUser::GetProfile(bool any) const {
    static_cast<void>(any);
    return nullptr;
}

// Reconstructed from eboot.elf at 0x9A5820.
bool RemoteBandUser::IsLocal() const {
    return false;
}

// Reconstructed from eboot.elf at 0x9A5900.
bool RemoteBandUser::IsSompLagging() const {
    return mSessionFlags[0];
}

// Reconstructed from eboot.elf at 0x9A5910.
bool RemoteBandUser::IsSompIdle() const {
    return mSessionFlags[2];
}
