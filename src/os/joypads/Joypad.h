#pragma once

#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

template <typename T>
struct AllowedValue;

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

class User;

// The users' pad assignments (os/Joypad.o). Not reconstructed.
// The user on the pad, or null.
User* JoypadGetUserFromPadNum(int padNum);  // 0x39C990
// The pad of the user, or -1. The map has both GetUsersPadNum and
// JoypadGetUsersPadNum with this signature; which one this is rests on
// the name of its pair JoypadGetUserFromPadNum. Weak.
int JoypadGetUsersPadNum(const User* user);  // 0x39C9B0
// Whether a joypad is connected on the pad; false for -1.
bool JoypadIsConnectedPadNum(int padNum);  // 0x39D3C0
// Assigns each of the user manager's users the pad of its position, under
// the pad lock. The map places ResetAllUsersPads in os/Joypad.o; this
// build's copy sits with the PS4 joypads. Weak.
void ResetAllUsersPads();  // 0x8CF060

// The joypad information component ("Provides information about a
// connected joypad"). Only its registration is declared. Name not in the
// reference map.
class EditorJoypadDataCom {
public:
    static void Init();  // 0x368D00
};

// A joypad button (26 of them) and a joypad action (15), the parameters of
// the UI button messages. The map names the types; their enumerators are
// not recovered.
enum JoypadButton : int {};
enum JoypadAction : int {};

// The display names of a button and an action, which UIInputReceiverCom's
// exported events list. Not reconstructed.
const char* JoypadActionToName(JoypadAction action);  // 0x39E050
const char* JoypadButtonToName(JoypadButton button);  // 0x39E060
// The 15 actions with their names, for properties that hold an action.
// Not reconstructed.
eastl::vector<AllowedValue<int>> GetJoypadActionsAllowedValues();  // 0x39DF00

// A pad's state, one of the table at 0x1A02320 (624 bytes each). Only the
// held buttons are modelled. Field names are not in the reference map.
struct JoypadData {
    // A bit per held JoypadButton.
    unsigned int mButtons;
    unsigned char mOpaque[500];  // Not modelled.
    // The pad's controller type, which maps its buttons to actions. Name not
    // in the reference map.
    Symbol mControllerType;
    unsigned char mOpaque2[112];  // Not modelled.
};

static_assert(__builtin_offsetof(JoypadData, mControllerType) == 0x1F8);
static_assert(sizeof(JoypadData) == 624);

// os/Joypad.o. Not reconstructed.
JoypadData* JoypadGetPadData(unsigned long padNum);  // 0x39C710
// The action the controller type's button stands for. Not reconstructed.
JoypadAction ButtonToAction(JoypadButton button, Symbol controllerType);  // 0x39C580
// The controller type of the pad, "none" without one.
Symbol JoypadControllerTypePadNum(int padNum);  // 0x39C2C0
// The controller type's "pad_shift_button" and "cymbal_shift_button" from
// the joypad configuration.
JoypadButton JoypadTypePadShiftButton(Symbol type);     // 0x39D8B0
JoypadButton JoypadTypeCymbalShiftButton(Symbol type);  // 0x39D970
// Whether the controller type flips its directions for left-handed
// players.
bool JoypadTypeHasLeftyFlip(Symbol type);  // 0x39D790
