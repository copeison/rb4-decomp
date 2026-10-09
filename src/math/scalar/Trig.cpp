#include "math/scalar/Trig.h"

#include <cmath>

namespace {

constexpr int kSinTableSize = 256;

// Table samples per radian, 256 / (2 pi).
constexpr float kSinTableScale = 40.743664F;

// Largest scaled angle the tables convert; larger ones read sample 0.
constexpr float kSinTableMaxIndex = 10000000.0F;

// One sample of the sine table and the step to the next sample. Name not in
// the reference map.
struct SinTableEntry {
    float mValue;
    float mSlope;
};

// Name not in the reference map (file-local in math/Trig.o). The reference
// build's table is 0x1000 bytes; this build's is 256 entries of 8 bytes at
// 0x19E66F0.
SinTableEntry gBigSinTable[kSinTableSize];

static_assert(sizeof(SinTableEntry) == 8);
static_assert(sizeof(gBigSinTable) == 0x800);

}  // namespace

// Reconstructed from eboot.elf at 0x219610.
void TrigTableInit() {
    constexpr float kStep = 0.024543693F;  // 2 pi / 256.
    for (int i = 0; i <= kSinTableSize; i++) {
        float value = std::sin(static_cast<float>(i) * kStep);
        if (i < kSinTableSize) {
            gBigSinTable[i].mValue = value;
        }
        if (i > 0) {
            gBigSinTable[i - 1].mSlope = value - gBigSinTable[i - 1].mValue;
        }
    }
}

// Reconstructed from eboot.elf at 0x219680.
void TrigTableTerminate() {}

// Reconstructed from eboot.elf at 0x219690. Negative angles use
// sin(-x) = -sin(x); the index wraps to the table by its low byte.
float Sine(float angle) {
    if (angle >= 0.0F) {
        float scaled = angle * kSinTableScale;
        if (kSinTableMaxIndex < scaled) {
            scaled = 0.0F;
        }
        int index = static_cast<int>(scaled);
        const SinTableEntry& entry =
            gBigSinTable[static_cast<unsigned char>(index)];
        return (scaled - static_cast<float>(index)) * entry.mSlope +
            entry.mValue;
    }

    float scaled = angle * -kSinTableScale;
    if (kSinTableMaxIndex < scaled) {
        scaled = 0.0F;
    }
    int index = static_cast<int>(scaled);
    const SinTableEntry& entry =
        gBigSinTable[static_cast<unsigned char>(index)];
    return (static_cast<float>(index) - scaled) * entry.mSlope - entry.mValue;
}

// Reconstructed from eboot.elf at 0x219710.
float FastSin(float angle) {
    constexpr float kRound = 0.49999F;
    float scaled = angle * kSinTableScale;
    if (angle >= 0.0F) {
        int index = static_cast<int>(scaled + kRound);
        return gBigSinTable[static_cast<unsigned char>(index)].mValue;
    }
    int index = static_cast<int>(kRound - scaled);
    return -gBigSinTable[static_cast<unsigned char>(index)].mValue;
}
