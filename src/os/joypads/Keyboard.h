#pragma once

#include <cstddef>

#include "utl/data/DataArray.h"
#include "utl/text/Symbol.h"

class MsgSink;

// A key press sent to the keyboard subscribers (os/Keyboard.o): a message
// whose array is ("key" key shift ctrl alt). The message base (vtable
// 0x18FBEE8) is not modelled; it keeps the array pointer and inline storage
// for a small array. The vtable is at 0x18FBF08.
class KeyboardKeyMsg {
public:
    // Wraps an existing message array.
    explicit KeyboardKeyMsg(DataArray* msg);  // 0x392240
    virtual ~KeyboardKeyMsg();                // 0x392150

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

    // Field names are not in the reference map.
    DataArray* mData;
    // The message base's inline array and its seven nodes.
    unsigned char mInlineStorage[136];
};

static_assert(offsetof(KeyboardKeyMsg, mData) == 8);
static_assert(sizeof(KeyboardKeyMsg) == 152);

// Routes every keyboard message to `sink` before the subscribers; null
// restores normal delivery. Returns the previous override.
MsgSink* KeyboardOverride(MsgSink* sink);  // 0x3A1970
// Delivers the message to the subscribers, skipping the override. The map
// gives no return type.
void KeyboardSendMsgBypassOverride(DataArray* msg);  // 0x3A1AA0
