#include "stage_presence.h"

#include <array>
#include <cstddef>

namespace rb4 {

namespace {

const std::array<Symbol, 22>& stage_presence_symbols() {
    static const std::array<Symbol, 22> symbols{
        Symbol{"kBandOverdrive"},
        Symbol{"kGreatGuitarSolo"},
        Symbol{"kMaxStreakTrack"},
        Symbol{"kMaxStreakBand"},
        Symbol{"kBandUnity"},
        Symbol{"kUpstrummer"},
        Symbol{"kHopoMaster"},
        Symbol{"kFullOfFills"},
        Symbol{"kSuperSavior"},
        Symbol{"kVocalFreestyler"},
        Symbol{"kHarmonizer"},
        Symbol{"kVocalCrowdWork"},
        Symbol{"kImprovGuitarSixteenths"},
        Symbol{"kImprovGuitarEights"},
        Symbol{"kImprovGuitarLicks"},
        Symbol{"kImprovGuitarHeldNotes"},
        Symbol{"kImprovGuitarTapping"},
        Symbol{"kTakingRequests"},
        Symbol{"kGreatBassSolo"},
        Symbol{"kGreatDrumSolo"},
        Symbol{"kSoloFiveStars"},
        Symbol{"kImprovGuitarGeneral"},
    };
    return symbols;
}

}  // namespace

// Reconstructed from eboot.elf at 0x997590.
Symbol stage_presence_id_to_symbol(StagePresenceId id) {
    return stage_presence_symbols()[static_cast<std::size_t>(id)];
}

}  // namespace rb4
