#include "rb_game/stagepresence/RBStagePresenceEnum.h"

// Reconstructed from eboot.elf at 0x997590. The names are interned into a
// guarded function-local table on the first call.
Symbol StagePresence::_ToSymbol(Id id) {
    static const Symbol sSymbols[] = {
        Symbol("kBandOverdrive"),
        Symbol("kGreatGuitarSolo"),
        Symbol("kMaxStreakTrack"),
        Symbol("kMaxStreakBand"),
        Symbol("kBandUnity"),
        Symbol("kUpstrummer"),
        Symbol("kHopoMaster"),
        Symbol("kFullOfFills"),
        Symbol("kSuperSavior"),
        Symbol("kVocalFreestyler"),
        Symbol("kHarmonizer"),
        Symbol("kVocalCrowdWork"),
        Symbol("kImprovGuitarSixteenths"),
        Symbol("kImprovGuitarEights"),
        Symbol("kImprovGuitarLicks"),
        Symbol("kImprovGuitarHeldNotes"),
        Symbol("kImprovGuitarTapping"),
        Symbol("kTakingRequests"),
        Symbol("kGreatBassSolo"),
        Symbol("kGreatDrumSolo"),
        Symbol("kSoloFiveStars"),
        Symbol("kImprovGuitarGeneral"),
    };
    static_assert(sizeof(sSymbols) / sizeof(sSymbols[0]) ==
                  kImprovGuitarGeneral + 1);
    return sSymbols[id];
}
