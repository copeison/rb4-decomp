#pragma once

#include "utl/text/Symbol.h"

// The beatmatch module's shared definitions (rb_beatmatch/Defines.o,
// 0x917D80-0x917F52 in this build). Only the difficulty conversions the
// band users call are declared; the object is not reconstructed.

// A part's difficulty, in the order of the DIFF_SYMBOLS macro. The map names
// the type; the enumerator names are not in the reference map.
// kNumDifficulties is SymToDifficulty's answer for an unknown symbol.
enum Difficulty {
    kDifficultyEasy = 0,
    kDifficultyMedium = 1,
    kDifficultyHard = 2,
    kDifficultyExpert = 3,
    kNumDifficulties = 4,
};

// The difficulty's DIFF_SYMBOLS entry.
Symbol DifficultyToSym(Difficulty difficulty);  // 0x917D80
// The difficulty's DIFF_SHORT_SYMBOLS entry.
Symbol DifficultyToShortSym(Difficulty difficulty);  // 0x917DE0
// The first of the four DIFF_SYMBOLS entries the symbol matches, else
// kNumDifficulties.
Difficulty SymToDifficulty(Symbol sym);  // 0x917E40
// kDifficultyEasy in this build.
Difficulty DefaultDifficulty();  // 0x917F50
