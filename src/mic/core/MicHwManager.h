#pragma once

#include <cstddef>

#include "audio/core/system/Audio.h"

class Mic;
class MicHwManager_FMOD;

// The engine's microphone list at 0x19C8FC0. Platform managers such as
// MicHwManager_FMOD fill it. The mic module has not been reconstructed; only
// the members the FMOD manager uses are declared. Names not in the reference
// map unless noted.
class MicHwManager {
public:
    void RegisterPlatform(MicHwManager_FMOD* platform);  // 0xA1EB0
    void ClearMics();                    // 0xA1C40
    void AddMic(Mic* mic);               // 0xA1F70
    // Returns an unbound mic of the given type. At 0xA26F0.
    Mic* GetFreeMic(int type);
    // Flags the mic list as changed. Reached through 0xA2DE0.
    void MarkMicsChanged();

    unsigned char mUnknown0[16];
    CritSec mCritSec;
    Mic** mMicsBegin;
    Mic** mMicsEnd;
};

static_assert(offsetof(MicHwManager, mCritSec) == 16);
static_assert(offsetof(MicHwManager, mMicsBegin) == 32);

extern MicHwManager gMicHwManager;
