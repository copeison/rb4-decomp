#pragma once

// Joypad types shared by the platform controllers (os/Joypad.o). Only the
// declarations the reconstructed controllers use are present.

// Controller type. Only the two Pembroke guitar values are known; their
// enumerator names are not in the reference map.
enum JoypadType : unsigned int {
    kJoypadPembrokeGuitarMadCatz = 0x1F,  // USB vendor 0x0738, product 0x8261
    kJoypadPembrokeGuitarPdp = 0x20,      // USB vendor 0x0E6F, product 0x0173
};

// Calbert (guitar calibration sensor) mode. Enumerator names are not in the
// reference map.
enum JoypadCalbertMode : unsigned int {
    kJoypadCalbertDisabled = 0,
    kJoypadCalbertMode1 = 1,
    kJoypadCalbertMode2 = 2,
};

// Ring of Calbert sensor samples copied out by GetCalbertValues.
struct CalbertValues {
    static constexpr int kMaxValues = 64;  // Name not in the reference map.

    float mValues[kMaxValues];  // Name not in the reference map.
    int mNumValues;             // Name not in the reference map.
};

static_assert(sizeof(CalbertValues) == 0x104);

// os/Joypad_PS4.o and os/Joypad.o. Not reconstructed.
// Starts the pad service unless the joypad configuration disables it.
void JoypadInit();  // 0x8CEFA0
void JoypadTerminate();  // 0x8CF810
// Reads the pads and refreshes the players' pad assignments. The frame loop
// documentation's descriptive name; the map's JoypadPoll is the likely
// match. Weak.
void input_refresh_player_assignments();  // 0x8CF240
// Sends the held-button repeats of every joypad client (os/JoypadClient.o).
void JoypadClientPoll();  // 0x39FB50

// The joypad information component ("Provides information about a
// connected joypad"). Only its registration is declared. Name not in the
// reference map.
class EditorJoypadDataCom {
public:
    static void Init();  // 0x368D00
};
