#pragma once

#include "entity/core/Component.h"

class BandUser;

// The component that runs the overshell scene ("Total control of the
// overshell scene."; rb_meta/RBOvershellCom.o, constructor 0xBE8810, vtable
// 0x19719E8). Only the member the dialogs call is declared; the class is
// not reconstructed.
class RBOvershellCom : public Component {
public:
    // Whether the user's input goes to the shell rather than to the
    // overshell: true unless the slot that holds the user, or has the user
    // waiting to join, is in a state that keeps the input. The map has the
    // per-slot IsAnySlotAllowingInputToShell(); this per-user form is newer.
    // Name not in the reference map; the evidence is weak.
    bool IsUserAllowingInputToShell(BandUser* user);  // 0xBEB060
};

// The overshell component, which its constructor sets and its destructor
// clears. Name not in the reference map.
extern RBOvershellCom* theOvershell;  // 0x1B02C38
