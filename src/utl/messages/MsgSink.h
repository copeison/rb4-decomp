#pragma once

#include "utl/data/DataNode.h"

class DataArray;

// A receiver of script messages (utl/MsgSink.o). Only the vtable layout the
// reconstructed sinks override is declared: the destructors, Handle, and a
// fourth slot whose default at 0x8E60 returns zero.
class MsgSink {
public:
    virtual ~MsgSink() {}
    // Slot 2: handles the message, returning its result.
    virtual DataNode Handle(DataArray* msg, bool warn) = 0;
    // Slot 3 at 0x8E60. Name not in the reference map.
    virtual void* _Unknown3() {
        return nullptr;
    }
};

static_assert(sizeof(MsgSink) == 8);
