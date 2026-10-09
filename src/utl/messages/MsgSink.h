#pragma once

#include "utl/data/DataNode.h"

class DataArray;

// A receiver of script messages (utl/MsgSink.o). Only the vtable layout the
// reconstructed sinks override is declared: the destructors, Handle, and a
// fourth slot whose default at 0x8E60 returns null.
class MsgSink {
public:
    virtual ~MsgSink() {}
    // Slot 2: handles the message, returning its result.
    virtual DataNode Handle(DataArray* msg, bool warn) = 0;
    // Slot 3 at 0x8E60: the object behind the sink. Name not in the
    // reference map, which names no MsgSink methods. Weak: every vtable
    // found, MsgSource-derived ones included, keeps the shared null-returning
    // body, and no caller was identified.
    virtual void* GetSinkObject() {
        return nullptr;
    }
};

static_assert(sizeof(MsgSink) == 8);
