#pragma once

#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

// The meta game's UI state (rb_meta/RBMetaStateCom.o): the layout IDs, the
// layout-ID defines, and RBMetaState, which goes to layouts by ID and keeps
// the read-only "rb meta globals" state the RBMetaStateCom component
// exposes. Only what the startup and the main loop use is declared.

// The game's UI layouts. The map names the type; the enumerator names are
// the strings RBUILayoutToSym interns.
enum RBUILayout {
    kLayoutInvalid = -1,
#define RB_UI_LAYOUT(name, path) name,
#include "rb_meta/state/RBUILayout.inc"
#undef RB_UI_LAYOUT
};

static_assert(sizeof(RBUILayout) == 4);

// The layout's name, interned into a guarded table on the first call;
// "kLayoutInvalid" for kLayoutInvalid.
Symbol RBUILayoutToSym(RBUILayout layout);  // 0xBB06A0

// Registers the kLayout* defines for the UI layout IDs with the data system.
void InitRBLayoutDefines();  // 0xBC4760

// The meta game's UI state. theRBMetaState holds the one instance; its
// layout and the rest of the class are not reconstructed.
class RBMetaState : public MsgSource {
public:
    // Goes to the layout through UIMgr::Goto unless it is current. The
    // map's Goto(RBUILayout); this build adds the flag, which skips the
    // layout's name report (compiled out but for its RBUILayoutToSym call)
    // that a global switch (0x1B1E030) enables. The startup clears it.
    void Goto(RBUILayout layout, bool quiet);  // 0xBB5D40
    // Starts the shell music once its scene is loaded, follows the SFX and
    // crowd volume options, plays a pending back sound, and counts the
    // joined players.
    void Poll();  // 0xBB58C0
};

extern RBMetaState theRBMetaState;  // 0x1AFF588
