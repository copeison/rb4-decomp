#pragma once

#include <cstddef>

#include "os/joypads/DualShock4_PS4.h"
#include "os/joypads/Joypad.h"

struct ScePadData;

// Rock Band 4 guitar on the PS4 special-controller pad API
// (os/PembrokeGuitar_PS4.o). Only the Calbert sensor support and the
// hardware detection in Activate are reconstructed.
class PembrokeGuitarController : public DualShock4Controller {
public:
    // Activates the base controller, then probes both guitar models.
    void Activate(int platformUserId);                // 0x8D28A0
    // The map does not show the return type.
    bool SetCalbertMode(JoypadCalbertMode mode);      // 0x8D3060
    // The map does not show the return type.
    bool GetCalbertValues(CalbertValues& values);     // 0x8D3000
    void _ProcessScePadData(ScePadData* data, int count);  // 0x8D2EA0

    JoypadCalbertMode mCalbertMode;  // Name not in the reference map.
    // The game thread's copy of mPendingCalbertState, taken by the
    // slot-17 virtual at 0x8D31D0 (the map's _PostGameThreadRead). Cleared
    // by SetCalbertMode and by Reset (0x8D2A10). Name not in the reference
    // map; its meaning is not recovered beyond that.
    int mCalbertState;
    // The pad thread's side of mCalbertState; only Reset writes it in the
    // recovered code. Name not in the reference map.
    int mPendingCalbertState;
    CalbertValues mCalbertValues;    // Name not in the reference map.
    // Detected guitar model. Name not in the reference map.
    JoypadType mType;
};

static_assert(offsetof(PembrokeGuitarController, mCalbertMode) == 0x2004);
static_assert(offsetof(PembrokeGuitarController, mCalbertState) == 0x2008);
static_assert(offsetof(PembrokeGuitarController, mPendingCalbertState) == 0x200C);
static_assert(offsetof(PembrokeGuitarController, mCalbertValues) == 0x2010);
static_assert(offsetof(PembrokeGuitarController, mType) == 0x2114);
static_assert(sizeof(PembrokeGuitarController) == 0x2118);
