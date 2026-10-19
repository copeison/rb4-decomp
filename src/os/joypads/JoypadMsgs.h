#pragma once

#include "os/joypads/Joypad.h"
#include "utl/data/DataArray.h"
#include "utl/messages/Message.h"

class User;

// The joypad's button messages (os/JoypadMsgs.o): (target type user button
// action pad value). Only the members the reconstructed handlers use are
// declared; the object is not reconstructed. The destructors are
// Message<5>'s (0x392150).

// A pressed button ("button_down"); the vtable is at 0x18FBEC8.
class ButtonDownMsg : public Message<5> {
public:
    ButtonDownMsg(User* user, JoypadButton button, JoypadAction action, int padNum, float value);  // 0x3A0440

    // The user whose pad sent the button (node 2).
    User* GetUser() const;  // 0x3A0BC0
    // Nodes 3 to 6. Inlined into their users; names not in the reference
    // map, and the analog value's is weak.
    JoypadButton GetButton() const {
        return static_cast<JoypadButton>(mData->Int(3));
    }
    JoypadAction GetAction() const {
        return static_cast<JoypadAction>(mData->Int(4));
    }
    int GetPadNum() const {
        return mData->Int(5);
    }
    float GetValue() const {
        return mData->Float(6);
    }
};

// A released button ("button_up"), with ButtonDownMsg's arguments.
class ButtonUpMsg : public Message<5> {
public:
    ButtonUpMsg(User* user, JoypadButton button, JoypadAction action, int padNum, float value);  // 0x3A0BD0

    // The user whose pad sent the button (node 2).
    User* GetUser() const;  // 0x3A1030
    // Nodes 3 to 6, as ButtonDownMsg's.
    JoypadButton GetButton() const {
        return static_cast<JoypadButton>(mData->Int(3));
    }
    JoypadAction GetAction() const {
        return static_cast<JoypadAction>(mData->Int(4));
    }
    int GetPadNum() const {
        return mData->Int(5);
    }
    float GetValue() const {
        return mData->Float(6);
    }
};
