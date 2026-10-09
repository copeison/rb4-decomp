#include "os/joypads/Keyboard.h"

namespace {

// The sink that receives every keyboard message, at 0x1A03960. Name not in
// the reference map.
MsgSink* gKeyboardOverride;

}  // namespace

// Reconstructed from eboot.elf at 0x392240.
KeyboardKeyMsg::KeyboardKeyMsg(DataArray* msg) : Message<5>(msg) {
    // What remains of the type check in this build: the type is evaluated
    // and the message type built.
    static_cast<void>(msg->Node(1).LiteralSym(msg));
    static_cast<void>(Event());
}

// Reconstructed from eboot.elf at 0x391750 and 0x3923A0.
KeyboardKeyMsg::~KeyboardKeyMsg() {}

// Reconstructed from eboot.elf at 0x3A1970.
MsgSink* KeyboardOverride(MsgSink* sink) {
    MsgSink* old = gKeyboardOverride;
    gKeyboardOverride = sink;
    return old;
}
