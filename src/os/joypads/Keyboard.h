#pragma once

#include <cstddef>

#include "utl/data/DataArray.h"
#include "utl/messages/Message.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

class MsgSink;
class SyncMsgSource;

// A key press sent to the keyboard subscribers (os/Keyboard.o): a message
// whose array is (target "key" key shift ctrl alt keyData). The vtable is at
// 0x18FBF08.
class KeyboardKeyMsg : public Message<5> {
public:
    // A press of the key with the modifier states. The map has
    // KeyboardKeyMsg(int, bool, bool, bool); this build adds the fifth
    // flag, node 6, which the reconstructed users do not read.
    KeyboardKeyMsg(int key, bool shift, bool ctrl, bool alt, bool keyData);  // 0x3A1BC0
    // Wraps an existing message array.
    explicit KeyboardKeyMsg(DataArray* msg);  // 0x392240
    // The complete destructor at 0x391750 is folded with other messages';
    // the deleting destructor is at 0x3923A0.
    ~KeyboardKeyMsg() override;

    // The message type. The map has the static KeyboardKeyMsg::Event()::t;
    // in this build it is built on first use. Inlined into its users.
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("key");
        }
        return t;
    }

    // Name not in the reference map.
    int GetKey() const {
        return mData->Int(2);
    }
};

static_assert(offsetof(KeyboardKeyMsg, mData) == 8);
static_assert(sizeof(KeyboardKeyMsg) == 152);

// A key release: (target "key_release" key). The vtable is at 0x18FC700.
class KeyboardKeyReleaseMsg : public Message<1> {
public:
    explicit KeyboardKeyReleaseMsg(int key);  // 0x3A2010
    // The complete destructor at 0x3A1BB0 jumps to the destructor shared by
    // the one-argument messages (0x8D710); the deleting destructor is at
    // 0x3A2180.
    ~KeyboardKeyReleaseMsg() override;

    // The message type, built on first use. Inlined into its users.
    static Symbol Event() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("key_release");
        }
        return t;
    }
};

static_assert(sizeof(KeyboardKeyReleaseMsg) == 88);

// The source the keyboard messages are exported from, created by
// KeyboardInitCommon. At 0x1A03958. Name not in the reference map.
extern SyncMsgSource* gKeyboardSource;

// Creates the keyboard message source.
void KeyboardInitCommon();  // 0x3A18A0
// Deletes it.
void KeyboardTerminateCommon();  // 0x3A18E0
// Subscribes the sink to every keyboard message through `subscription`,
// which the subscriber owns. The map has KeyboardSubscribe(MsgSink*).
void KeyboardSubscribe(MsgSource::EventSinkElem* subscription, MsgSink* sink);  // 0x3A1940

// Routes every keyboard message to `sink` instead of the subscribers; null
// restores normal delivery. Returns the previous override, kept at
// 0x1A03960.
MsgSink* KeyboardOverride(MsgSink* sink);  // 0x3A1970
// Sends a key press to the override, or to the subscribers when there is
// none. The map has KeyboardSendMsg(int, bool, bool, bool).
void KeyboardSendMsg(int key, bool shift, bool ctrl, bool alt, bool keyData);  // 0x3A1980
// Delivers the message to the subscribers, skipping the override; true when
// one handled it. The map gives no return type.
bool KeyboardSendMsgBypassOverride(DataArray* msg);  // 0x3A1AA0
// Sends a key release like KeyboardSendMsg.
void KeyboardSendReleaseMsg(int key);  // 0x3A1AC0

// os/Keyboard_PS4.o: the common setup and the IME library.
void KeyboardInit();  // 0x36D990
void KeyboardTerminate();  // 0x36D9B0

// The keyboard controls for moving a transform in the editor. Only its
// registration is declared. Name not in the reference map.
class KeyboardTransControllerCom {
public:
    static void Init();  // 0x3690C0
};
