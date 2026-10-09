#include "math/scalar/Trig.h"

#include <cmath>

#include "utl/data/DataArray.h"
#include "utl/data/DataFunc.h"

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

// Degree conversions of the script functions. Names not in the reference
// map.
constexpr float kDegreesToRadians = 0.017453292F;
constexpr float kRadiansToDegrees = 57.295776F;

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

// The script functions take their argument from the first node after the
// name. They are file-local; their names are not in the reference map.

// Reconstructed from eboot.elf at 0x219860.
static DataNode DataSin(DataArray* args) {
    return DataNode(std::sin(args->Float(1) * kDegreesToRadians));
}

// Reconstructed from eboot.elf at 0x2198B0.
static DataNode DataCos(DataArray* args) {
    return DataNode(std::cos(args->Float(1) * kDegreesToRadians));
}

// Reconstructed from eboot.elf at 0x219900.
static DataNode DataTan(DataArray* args) {
    return DataNode(std::tan(args->Float(1) * kDegreesToRadians));
}

// Reconstructed from eboot.elf at 0x219940.
static DataNode DataASin(DataArray* args) {
    return DataNode(std::asin(args->Float(1)) * kRadiansToDegrees);
}

// Reconstructed from eboot.elf at 0x219980.
static DataNode DataACos(DataArray* args) {
    return DataNode(std::acos(args->Float(1)) * kRadiansToDegrees);
}

// Reconstructed from eboot.elf at 0x2199C0.
static DataNode DataATan(DataArray* args) {
    return DataNode(std::atan(args->Float(1)) * kRadiansToDegrees);
}

// Reconstructed from eboot.elf at 0x219770.
void TrigInit() {
    DataRegisterFunc(Symbol("sin"), DataSin);
    DataRegisterFunc(Symbol("cos"), DataCos);
    DataRegisterFunc(Symbol("tan"), DataTan);
    DataRegisterFunc(Symbol("asin"), DataASin);
    DataRegisterFunc(Symbol("acos"), DataACos);
    DataRegisterFunc(Symbol("atan"), DataATan);
}
