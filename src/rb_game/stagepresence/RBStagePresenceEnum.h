#pragma once

#include "utl/text/Symbol.h"

// Stage presence events (rb_game/RBStagePresenceEnum.o). StagePresence is a
// namespace in the map: its rule classes nest in it and its operator== is a
// two-argument free function.
namespace StagePresence {

// The enumerator names are the strings _ToSymbol interns.
enum Id {
    kBandOverdrive,
    kGreatGuitarSolo,
    kMaxStreakTrack,
    kMaxStreakBand,
    kBandUnity,
    kUpstrummer,
    kHopoMaster,
    kFullOfFills,
    kSuperSavior,
    kVocalFreestyler,
    kHarmonizer,
    kVocalCrowdWork,
    kImprovGuitarSixteenths,
    kImprovGuitarEights,
    kImprovGuitarLicks,
    kImprovGuitarHeldNotes,
    kImprovGuitarTapping,
    kTakingRequests,
    kGreatBassSolo,
    kGreatDrumSolo,
    kSoloFiveStars,
    kImprovGuitarGeneral,
};

static_assert(sizeof(Id) == 4);

Symbol _ToSymbol(Id id);  // 0x997590

}  // namespace StagePresence
