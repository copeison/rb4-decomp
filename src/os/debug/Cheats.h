#pragma once

#include <cstddef>

#include "utl/messages/MsgSink.h"
#include "utl/text/Str.h"

// The keyboard and joypad cheats (os/Cheats.o). Only the text the cheats
// overlay shows is named; the constructor (0x390540) and the methods are
// not reconstructed. The vtable is at 0x18FBEA8.
class CheatsManager : public MsgSink {
public:
    DataNode Handle(DataArray* msg, bool warn) override;

    // Field names are not in the reference map.
    unsigned char mUnknown8[120];
    // The cheat message RndCheatsOverlay prints; empty when there is none.
    String mMessage;
    unsigned char mUnknown144[88];
};

static_assert(offsetof(CheatsManager, mMessage) == 128);
static_assert(sizeof(CheatsManager) == 232);

// Created by CheatsInit unless the "cheats" configuration disables cheats;
// null otherwise. At 0x1A01DE8.
extern CheatsManager* theCheatsManager;
