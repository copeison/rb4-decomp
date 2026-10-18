#pragma once

#include <cstddef>

#include "utl/messages/MsgSink.h"

// Delivers joypad button messages to a sink with repeat and hold handling
// (os/JoypadClient.o). Only the members UIMgr uses are declared; the class
// is not reconstructed. The vtable at 0x18FC690 has the four MsgSink slots;
// the object is 600 bytes.
class JoypadClient : public MsgSink {
public:
    // Subscribes nothing yet; the sink receives the messages.
    explicit JoypadClient(MsgSink* sink);  // 0x39F4B0
    // Slots 0-1: 0x39F740, 0x39F800.
    ~JoypadClient() override;
    // Slot 2.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x39FC30

    // The buttons that repeat while held.
    void SetRepeatMask(int mask);  // 0x39FA40
    // Whether the sticks also send d-pad buttons.
    void SetVirtualDpad(bool virtualDpad);  // 0x39FAA0
    // Leaves the joypad source and, when `enabled` is set, subscribes to it
    // again. Name not in the reference map.
    void SetInputEnabled(bool enabled);  // 0x39F6E0

    unsigned char mOpaque[592];  // Not modelled.
};

static_assert(sizeof(JoypadClient) == 600);
