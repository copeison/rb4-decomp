#pragma once

#include "utl/text/Symbol.h"

// The instrument track types (rb_beatmatch/TrackType.o). Only the type and
// the symbol conversions are declared; the object is not reconstructed. The
// map names the type; the enumerator names and values are the allowed
// values of the track type properties (for example the one built at
// 0xC45160). Values 7 and 8 are later track types whose names are not
// recovered.
enum TrackType {
    kTrackDrum = 0,
    kTrackGuitar = 1,
    kTrackBass = 2,
    kTrackVocals = 3,
    // The two parts a keyboard controller plays (ControllerTypeToTrackType,
    // 0xA43F60). Names not in the reference map.
    kTrackKeys = 4,
    kTrackRealKeys = 5,
    // No part: the band user's reset value and SymToTrackType's answer for
    // an unknown symbol. The map's enumerator name; its value follows the
    // seventh TRACK_SYMBOLS entry.
    kTrackNone = 6,
};

// The track type's entry of the TRACK_SYMBOLS macro.
Symbol TrackTypeToSym(TrackType type);  // 0x922E10
// The first of the six TRACK_SYMBOLS entries the symbol matches, else
// kTrackNone. The map has SymToTrackType(Symbol); this build's callers also
// pass the address of a true flag, which the release body ignores.
TrackType SymToTrackType(Symbol sym, const bool& fail = true);  // 0x922E70
