#pragma once

#include <cstddef>

#include "audio/core/containers/LinkedListSizeTracked.h"
#include "os/threading/CritSec.h"

class TempoListener;

// The object a TempoListener is registered with; slot 2 removes the
// listener. No implementation is identified in this build; slots 0-1 are
// taken to be the destructor pair, which is weak evidence. Name not in the
// reference map.
class TempoListenerOwner {
public:
    virtual ~TempoListenerOwner();
    virtual void RemoveTempoListener(TempoListener* listener);
};

// Receives tempo and speed changes, for example the HMX DSP plugins and
// Delay. The map emits its destructor in audio/DelayPlugin.o; this build
// calls out-of-line copies of the constructor and the destructor, which
// have not been reconstructed. The vtable is at 0x18E2638; the object is 40
// bytes.
class TempoListener {
public:
    TempoListener();           // 0x5B2A0
    virtual ~TempoListener();  // slots 0-1: 0x5B2D0, 0x5B3F0
    // Slot 2. Name not in the reference map.
    virtual void OnTempoChanged(float tempo, float speed) = 0;

    // Leaves the owner under sCritSec; the destructor inlines the same
    // steps. Delay's inline destructor calls it first. At 0x5B390. Name not
    // in the reference map.
    void Unregister();

    // Guards every owner's listener list. 0x19C87A8, returned by 0x5B4C0.
    // Name not in the reference map.
    static CritSec sCritSec;

    // Field names are not in the reference map.
    LinkedListSizeTracked::Node mNode;
    TempoListenerOwner* mOwner;
};

static_assert(offsetof(TempoListener, mNode) == 8);
static_assert(offsetof(TempoListener, mOwner) == 32);
static_assert(sizeof(TempoListener) == 40);
