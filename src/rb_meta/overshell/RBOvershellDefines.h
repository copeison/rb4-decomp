#pragma once

#include "utl/text/Symbol.h"

// The overshell's shared definitions (rb_meta/RBOvershellDefines.o,
// 0xBF7280-0xBF7???). Declared only; the object is not reconstructed.

// Whether the overshell runs, and where ("active_status"). The enumerator
// names are the strings OvershellActiveStatusToSymbol interns.
enum OvershellActiveStatus : int {
    // "Overshell is inactive (loading screens, etc)."
    kOvershellInactive = 0,
    // "Game is in a meta screen."
    kOvershellInMeta = 1,
    // "In gameplay."
    kOvershellInSong = 2,
};

// The flow that takes every slot through one screen. The map names the type
// (RBOvershellCom::BeginOverrideFlow(OvershellOverrideFlow)); the enumerator
// names are not in the reference map and follow the slot states' flags
// (RBOvershellSlotState) that tell whether a slot is in the flow.
enum OvershellOverrideFlow : int {
    kOverrideFlowNone = 0,
    // "song_settings_flow".
    kOverrideFlowSongSettings = 1,
    // "choose_band_members_flow".
    kOverrideFlowChooseBandMembers = 2,
    // "idle_flow".
    kOverrideFlowIdle = 3,
    // "disabled_flow": the overshell is inactive or a gameplay state keeps
    // it hidden.
    kOverrideFlowDisabled = 4,
    // "somp_transition_flow": a SOMP session changes the layout.
    kOverrideFlowSompTransition = 5,
};

// A slot's state (RBOvershellSlotState::GetStateID), indexing
// OvershellSlotStateToString's table of 84 names. The map names the type;
// only the states the overshell names are listed.
enum OvershellSlotStateID : int {
    kOvershellSlotStateCount = 84,
};

// The state's name, "-UnknownSlotState-" out of range.
const char* OvershellSlotStateToString(OvershellSlotStateID state);  // 0xBF7280
// The status's name ("kOvershellInactive", ...), interned on first use.
Symbol OvershellActiveStatusToSymbol(OvershellActiveStatus status);  // 0xBF72A0
// The status the name stands for; kOvershellInMeta for any other name.
OvershellActiveStatus SymbolToOvershellActiveStatus(Symbol status);  // 0xBF7440
