#pragma once

#include <cstddef>

#include "utl/data/DataArray.h"
#include "utl/messages/Message.h"
#include "utl/text/Symbol.h"

class MsgSink;

// A key press sent to the keyboard subscribers (os/Keyboard.o): a message
// whose array is (target "key" key shift ctrl alt keyData). The vtable is at
// 0x18FBF08.
class KeyboardKeyMsg : public Message<5> {
public:
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

// Routes every keyboard message to `sink` instead of the subscribers; null
// restores normal delivery. Returns the previous override, kept at
// 0x1A03960.
MsgSink* KeyboardOverride(MsgSink* sink);  // 0x3A1970
// Delivers the message to the subscribers, skipping the override. The map
// gives no return type.
void KeyboardSendMsgBypassOverride(DataArray* msg);  // 0x3A1AA0
