#include "os/joypads/Keyboard.h"

#include "os/threading/CritSec.h"

SyncMsgSource* gKeyboardSource;

namespace {

// Guards the source and the override, at 0x1A03948. Name not in the
// reference map.
CritSec gKeyboardCritSec;
// The sink that receives every keyboard message, at 0x1A03960. Name not in
// the reference map.
MsgSink* gKeyboardOverride;

// Hands the message to the override, or exports it to the subscribers.
// Inlined into the senders. Name not in the reference map.
void SendKeyboardMsg(DataArray* msg) {
    ScopedCritSec lock(gKeyboardCritSec);
    if (gKeyboardOverride != nullptr) {
        gKeyboardOverride->Handle(msg, false);
    } else {
        gKeyboardSource->Export(msg);
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x3A18A0.
void KeyboardInitCommon() {
    gKeyboardSource = new SyncMsgSource(&gKeyboardCritSec);
}

// Reconstructed from eboot.elf at 0x3A18E0.
void KeyboardTerminateCommon() {
    ScopedCritSec lock(gKeyboardCritSec);
    delete gKeyboardSource;
    gKeyboardSource = nullptr;
}

// Reconstructed from eboot.elf at 0x3A1940.
void KeyboardSubscribe(MsgSource::EventSinkElem* subscription, MsgSink* sink) {
    gKeyboardSource->AddSink(subscription, sink, Symbol(), Symbol());
}

// Reconstructed from eboot.elf at 0x3A1970.
MsgSink* KeyboardOverride(MsgSink* sink) {
    MsgSink* old = gKeyboardOverride;
    gKeyboardOverride = sink;
    return old;
}

// Reconstructed from eboot.elf at 0x3A1980.
void KeyboardSendMsg(int key, bool shift, bool ctrl, bool alt, bool keyData) {
    KeyboardKeyMsg msg(key, shift, ctrl, alt, keyData);
    SendKeyboardMsg(msg.mData);
}

// Reconstructed from eboot.elf at 0x3A1AA0.
bool KeyboardSendMsgBypassOverride(DataArray* msg) {
    return gKeyboardSource->Export(msg);
}

// Reconstructed from eboot.elf at 0x3A1AC0.
void KeyboardSendReleaseMsg(int key) {
    KeyboardKeyReleaseMsg msg(key);
    SendKeyboardMsg(msg.mData);
}

// Reconstructed from eboot.elf at 0x3A1BC0.
KeyboardKeyMsg::KeyboardKeyMsg(int key, bool shift, bool ctrl, bool alt, bool keyData)
    : Message<5>(Event()) {
    mData->Node(2) = DataNode(key);
    mData->Node(3) = DataNode(static_cast<int>(shift));
    mData->Node(4) = DataNode(static_cast<int>(ctrl));
    mData->Node(5) = DataNode(static_cast<int>(alt));
    mData->Node(6) = DataNode(static_cast<int>(keyData));
}

// Reconstructed from eboot.elf at 0x392240.
KeyboardKeyMsg::KeyboardKeyMsg(DataArray* msg) : Message<5>(msg) {
    // What remains of the type check in this build: the type is evaluated
    // and the message type built.
    static_cast<void>(msg->Node(1).LiteralSym(msg));
    static_cast<void>(Event());
}

// Reconstructed from eboot.elf at 0x391750 and 0x3923A0.
KeyboardKeyMsg::~KeyboardKeyMsg() {}

// Reconstructed from eboot.elf at 0x3A2010.
KeyboardKeyReleaseMsg::KeyboardKeyReleaseMsg(int key) : Message<1>(Event()) {
    mData->Node(2) = DataNode(key);
}

// Reconstructed from eboot.elf at 0x3A1BB0 and 0x3A2180.
KeyboardKeyReleaseMsg::~KeyboardKeyReleaseMsg() {}
