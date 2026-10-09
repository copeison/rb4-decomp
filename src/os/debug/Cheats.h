#pragma once

#include <cstddef>

#include "utl/messages/MsgSink.h"
#include "utl/text/Str.h"

// The keyboard and joypad cheats (os/Cheats.o). Only the text the cheats
// overlay shows is named; the constructor (0x390540) and the methods are
// not reconstructed. The vtable is at 0x18FBEA8.
class CheatsManager : public MsgSink {
public:
    CheatsManager();  // 0x390540
    ~CheatsManager() override;  // 0x390710
    DataNode Handle(DataArray* msg, bool warn) override;

    // Appends "Cheats Used:" and the logged cheats, at most the configured
    // number, to `out`.
    void AppendLog(FixedString& out);  // 0x391E50

    // Field names are not in the reference map. The constructor (0x390540)
    // and destructor (0x390710) give the layout.
    // Two eastl::vectors (8 and 40), the cheat tables; Cheats.o in the map
    // instantiates vectors of KeyCheat* and QuickJoyCheat*.
    unsigned char mCheatTables[64];
    // Gate the joypad cheats (OnMsg(ButtonDownMsg const&), 0x3911F0) and
    // the keyboard cheats (OnMsg(KeyboardKeyMsg const&), 0x3914C0). Both
    // start set; a keyboard-capturing helper clears the second while it is
    // alive and its destructor (0x3A2EF0) sets it again.
    bool mJoypadCheatsEnabled;
    bool mKeyCheatsEnabled;
    unsigned char mPadding74[6];  // Never read or written.
    // An eastl::list of 32-byte nodes holding DataNodes (80) and a zeroed
    // field (112); the map's eastl::list<CheatLog> fits it.
    unsigned char mLog[40];
    // Set by the constructor; no reader was found.
    bool mReserved;
    unsigned char mPadding121[7];  // Never read or written.
    // The cheat message RndCheatsOverlay prints; empty when there is none.
    String mMessage;
    // Not touched by the constructor or destructor; the map's
    // ShowCheatMessage(char const*, float) suggests the message's display
    // time. Weak.
    unsigned char mMessageTime[8];
    // The joypad and keyboard subscriptions, registered through
    // JoypadSubscribe (0x39C7E0) and KeyboardSubscribe (0x3A1940) and
    // unlinked by the destructor.
    unsigned char mJoypadSubscription[40];
    unsigned char mKeyboardSubscription[40];
};

static_assert(offsetof(CheatsManager, mLog) == 80);
static_assert(offsetof(CheatsManager, mKeyCheatsEnabled) == 73);
static_assert(offsetof(CheatsManager, mMessage) == 128);
static_assert(offsetof(CheatsManager, mJoypadSubscription) == 152);
static_assert(sizeof(CheatsManager) == 232);

// Created by CheatsInit unless the "cheats" configuration disables cheats;
// null otherwise. At 0x1A01DE8.
extern CheatsManager* theCheatsManager;

// Creates theCheatsManager unless the cheats configuration disables them.
void CheatsInit();  // 0x391D90
void CheatsTerminate();  // 0x391E20
