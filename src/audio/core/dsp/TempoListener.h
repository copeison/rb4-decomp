#pragma once

#include <cstddef>

#include "audio/core/containers/LinkedListSizeTracked.h"
#include "os/threading/CritSec.h"

class AudioEmitterCom;

// Receives tempo and speed changes, for example the HMX DSP plugins and
// Delay. The map emits its destructor in audio/DelayPlugin.o; this build
// has an object of its own for the members (0x5B2A0 to 0x5B56C), which
// TempoListener.cpp reconstructs. The vtable is at 0x18E2638; the object is
// 40 bytes.
class TempoListener {
public:
    TempoListener();           // 0x5B2A0
    virtual ~TempoListener();  // slots 0-1: 0x5B2D0, 0x5B3F0
    // Slot 2. Name not in the reference map.
    virtual void OnTempoChanged(float tempo, float speed) = 0;

    // Leaves the emitter under sCritSec; the destructor inlines it. Delay's
    // inline destructor calls it first. At 0x5B390. Name not in the
    // reference map.
    void Unregister();

    // Returns sCritSec; the emitter's registration code locks it when it is
    // not null. At 0x5B4C0. Name not in the reference map.
    static CritSec* GetCritSec();

    // Guards every emitter's listener list. 0x19C87A8, built by the object's
    // static initializer (0x5B4D0). Name not in the reference map.
    static CritSec sCritSec;

    // Field names are not in the reference map.
    LinkedListSizeTracked::Node mNode;  // In the emitter's listener list.
    // The emitter interface that registered the listener (+0x228 in the
    // AudioEmitterCom component); slot 2 removes it.
    AudioEmitterCom* mOwner;
};

static_assert(offsetof(TempoListener, mNode) == 8);
static_assert(offsetof(TempoListener, mOwner) == 32);
static_assert(sizeof(TempoListener) == 40);
