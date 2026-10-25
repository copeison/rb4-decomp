#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataNode.h"
#include "utl/messages/Message.h"
#include "utl/text/Symbol.h"

class User;

// The UI manager's input messages (ui/UIMsg.o). Only the members the
// reconstructed handlers use are declared; the object is not
// reconstructed.

// A selection: (target "ui_select" object ... user). The vtable is at
// 0x195CF18; the destructor is Message<5>'s (0x392150).
class UISelectMsg : public Message<5> {
public:
    // Wraps an existing message array. The type check the release build
    // keeps only the calls of (the node's literal symbol and Event()) is
    // emitted in the users' objects, for example at 0x8C0C90.
    explicit UISelectMsg(DataArray* msg) : Message<5>(msg) {
        static_cast<void>(msg->Node(1).LiteralSym(msg));
        static_cast<void>(Event());
    }

    // The message type, built on first use (0x1AC8D10).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("ui_select");
        }
        return t;
    }

    // The user who selected (node 5).
    User* GetUser() const;  // 0x8CC5E0
    // The selected object (node 2). Inlined into its users. Name not in the
    // reference map.
    GameObjectId GetSelectedObject() const {
        return mData->Node(2).ObjectId(mData);
    }
};

static_assert(sizeof(UISelectMsg) == 152);

// A button press passed to a UI input receiver: (target "ui_button_down"
// user button action pad). The vtable is at 0x195C418; the destructor is
// Message<4>'s (0x3E990).
class UIButtonDownMsg : public Message<4> {
public:
    // Wraps an existing message array, as UISelectMsg's does (0x896590).
    explicit UIButtonDownMsg(DataArray* msg) : Message<4>(msg) {
        static_cast<void>(msg->Node(1).LiteralSym(msg));
        static_cast<void>(Event());
    }

    // The message type, built on first use (0x1AC7EA0).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("ui_button_down");
        }
        return t;
    }

    // The user who pressed the button (node 2).
    User* GetUser() const;  // 0x8CCA70
    // The joypad action of the button (node 4). Inlined into its users.
    // Name not in the reference map.
    int GetAction() const {
        return mData->Int(4);
    }
    // The pad (node 5). Inlined into its users. Name not in the reference
    // map.
    int GetPadNum() const {
        return mData->Int(5);
    }
};

static_assert(sizeof(UIButtonDownMsg) == 136);

// A button release passed to a UI input receiver: (target "ui_button_up"
// user button action pad). The map names the class
// (UIInputReceiverCom::OnMsg(UIButtonUpMsg const&)); the destructor is
// Message<5>'s (0x392150).
class UIButtonUpMsg : public Message<5> {
public:
    // Wraps an existing message array; the type check the release build
    // keeps is emitted in the users' objects (0x8A8FB0).
    explicit UIButtonUpMsg(DataArray* msg) : Message<5>(msg) {
        static_cast<void>(msg->Node(1).LiteralSym(msg));
        static_cast<void>(Event());
    }

    // The message type, built on first use (0x1AC8D30).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("ui_button_up");
        }
        return t;
    }

    // The user who released the button (node 2).
    User* GetUser() const;  // 0x8CD1B0
};

// A key press passed to a UI input receiver: (target "ui_key" key shift ctrl
// alt keyData), the fields of the KeyboardKeyMsg it comes from. The
// destructor is Message<5>'s (0x392150).
class UIKeyboardKeyMsg : public Message<5> {
public:
    // Wraps an existing message array, as UISelectMsg's does (0x896700).
    explicit UIKeyboardKeyMsg(DataArray* msg) : Message<5>(msg) {
        static_cast<void>(msg->Node(1).LiteralSym(msg));
        static_cast<void>(Event());
    }

    // The message type, built on first use (0x1AC7EC0).
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("ui_key");
        }
        return t;
    }

    // The key (node 2) and KeyboardKeyMsg's fifth flag (node 6). Inlined
    // into their users. Names not in the reference map.
    int GetKey() const {
        return mData->Int(2);
    }
    bool GetKeyData() const {
        return mData->Int(6) != 0;
    }
};

static_assert(sizeof(UIKeyboardKeyMsg) == 152);
