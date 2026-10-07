#pragma once

#include <cstdint>

#include "../core/symbol.h"

namespace rb4 {

enum class StagePresenceId : std::int32_t {
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

Symbol stage_presence_id_to_symbol(StagePresenceId id);

}  // namespace rb4
