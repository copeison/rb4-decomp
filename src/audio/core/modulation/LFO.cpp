#include "audio/core/modulation/LFO.h"

#include <cstdlib>
#include <cstring>

#include "math/scalar/Trig.h"

namespace {

// The tables at 0x18E59E0 and 0x18E5A00. Names not in the reference map.
const char* const kTargetNames[] = {"none", "pan", "pitch", "filter_freq"};
const char* const kShapeNames[] = {"sine", "square", "saw_up", "saw_down", "triangle"};

}  // namespace

float (*const LFO::sApplyWaveshapingTbl[5])(float phase) = {
    LFO::Sine, LFO::Square, LFO::SawtoothUp, LFO::SawtoothDown, LFO::Triangle};

// Reconstructed from eboot.elf at 0xBDD10.
float LFO::Sine(float phase) {
    return ::Sine((phase + 0.75F) * 6.2831855F) * 0.5F + 0.5F;
}

// Reconstructed from eboot.elf at 0xBDD40.
float LFO::Square(float phase) {
    return phase > 0.5F ? 1.0F : 0.0F;
}

// Reconstructed from eboot.elf at 0xBDD60.
float LFO::SawtoothUp(float phase) {
    return phase;
}

// Reconstructed from eboot.elf at 0xBDD70.
float LFO::SawtoothDown(float phase) {
    return 1.0F - phase;
}

// Reconstructed from eboot.elf at 0xBDD80.
float LFO::Triangle(float phase) {
    float rise = phase + phase;
    return rise > 1.0F ? 2.0F - rise : rise;
}

// Reconstructed from eboot.elf at 0xBDDB0. The phase is ignored; the scale
// is 2^-30.
float LFO::Noise(float) {
    return static_cast<float>(std::rand()) * 9.31322575e-10F;
}

// Reconstructed from eboot.elf at 0xBDDD0.
const char* LFO::Settings::TargetToString(Target target) {
    if (target > kTargetFilterFreq) {
        return nullptr;
    }
    return kTargetNames[target];
}

// Reconstructed from eboot.elf at 0xBDDF0.
LFO::Settings::Target LFO::Settings::StringToTarget(const char* name) {
    if (std::strcmp(name, "none") == 0) {
        return kTargetNone;
    }
    if (std::strcmp(name, "pan") == 0) {
        return kTargetPan;
    }
    if (std::strcmp(name, "pitch") == 0) {
        return kTargetPitch;
    }
    if (std::strcmp(name, "filter_freq") == 0) {
        return kTargetFilterFreq;
    }
    return kTargetInvalid;
}

// Reconstructed from eboot.elf at 0xBDE70.
const char* LFO::Settings::ShapeToString(Shape shape) {
    if (shape > kShapeTriangle) {
        return nullptr;
    }
    return kShapeNames[shape];
}

// Reconstructed from eboot.elf at 0xBDE90.
LFO::Settings::Shape LFO::Settings::StringToShape(const char* name) {
    if (std::strcmp(name, "sine") == 0) {
        return kShapeSine;
    }
    if (std::strcmp(name, "square") == 0) {
        return kShapeSquare;
    }
    if (std::strcmp(name, "saw_up") == 0) {
        return kShapeSawtoothUp;
    }
    if (std::strcmp(name, "saw_down") == 0) {
        return kShapeSawtoothDown;
    }
    if (std::strcmp(name, "triangle") == 0) {
        return kShapeTriangle;
    }
    return kShapeInvalid;
}

// Reconstructed from eboot.elf at 0xBDF20.
eastl::vector<AllowedValue<unsigned char>> LFO::Settings::GetAllowedTargetValues() {
    eastl::vector<AllowedValue<unsigned char>> values;
    values.reserve(4);
    values.emplace_back(
        AllowedValue<unsigned char>{kTargetNone, String("None"), String("No target.")});
    values.emplace_back(
        AllowedValue<unsigned char>{kTargetPan, String("Pan"), String("Target pan")});
    values.emplace_back(
        AllowedValue<unsigned char>{kTargetPitch, String("Pitch"), String("Target pitch")});
    values.emplace_back(AllowedValue<unsigned char>{
        kTargetFilterFreq, String("Filter Freq"), String("Target the filter frequency.")});
    return values;
}

// Reconstructed from eboot.elf at 0xBE1D0.
eastl::vector<AllowedValue<unsigned char>> LFO::Settings::GetAllowedShapeValues() {
    eastl::vector<AllowedValue<unsigned char>> values;
    values.reserve(5);
    values.emplace_back(
        AllowedValue<unsigned char>{kShapeSine, String("Sine"), String("Sine wave shape")});
    values.emplace_back(
        AllowedValue<unsigned char>{kShapeSquare, String("Square"), String("Square wave shape")});
    values.emplace_back(AllowedValue<unsigned char>{
        kShapeSawtoothUp, String("Ramp Up"), String("Upward sawtooth wave shape")});
    values.emplace_back(AllowedValue<unsigned char>{
        kShapeSawtoothDown, String("Ramp Down"), String("Downward sawtooth wave shape")});
    values.emplace_back(AllowedValue<unsigned char>{
        kShapeTriangle, String("Triangle"), String("Triangle wave shape")});
    return values;
}

// Reconstructed from eboot.elf at 0xBE510.
LFO::LFO()
    : mPhase(0.0),
      mRange{-1.0F, 2.0F},
      mMode(kModeCentered),
      mSettings(nullptr),
      mCyclesPerSample(0.0),
      mSecondsPerSample(1.0F),
      mTempo(0.0F) {}

// Reconstructed from eboot.elf at 0xBE550.
void LFO::SetPhase(double phase) {
    mPhase = phase;
}

// Reconstructed from eboot.elf at 0xBE560.
void LFO::SetRangeAndMode(SPL::Range<float> range, Mode mode) {
    mRange = range;
    mMode = mode;
}

// Reconstructed from eboot.elf at 0xBE570.
void LFO::UseSettings(const Settings* settings) {
    mSettings = settings;
    ComputeCyclesPerSample();
}

// Reconstructed from eboot.elf at 0xBE5C0.
void LFO::ComputeCyclesPerSample() {
    if (mSettings == nullptr) {
        return;
    }
    mCyclesPerSample = mSecondsPerSample * mSettings->mFrequency;
    if (mSettings->mBeatSync) {
        mCyclesPerSample *= mSettings->mDefaultTempo * (1.0F / 60.0F);
    }
    mTempo = mSettings->mDefaultTempo;
}

// Reconstructed from eboot.elf at 0xBE610.
double LFO::GetPhase() const {
    return mPhase;
}

// Reconstructed from eboot.elf at 0xBE620.
void LFO::Retrigger() {
    mPhase = (mSettings->mInitialPhase + 180.0F) * (1.0F / 360.0F);
}

// Reconstructed from eboot.elf at 0xBE650.
void LFO::Advance(unsigned int numSamples) {
    if (mTempo != mSettings->mDefaultTempo && mSettings->mBeatSync) {
        ComputeCyclesPerSample();
    }
    mPhase += numSamples * mCyclesPerSample;
    while (mPhase >= 1.0) {
        mPhase -= 1.0;
    }
}

// Reconstructed from eboot.elf at 0xBE6D0.
void LFO::Prepare(float sampleRate) {
    mSecondsPerSample = 1.0F / sampleRate;
    ComputeCyclesPerSample();
}

// Reconstructed from eboot.elf at 0xBE730.
float LFO::GetValue() const {
    float value = ApplyWaveshaping(static_cast<float>(mPhase));
    float depth = mSettings != nullptr && mSettings->mEnabled ? mSettings->mDepth : 0.0F;
    switch (mMode) {
    case kModeAdditive:
        value = depth * value;
        break;
    case kModeSubtractive:
        value = 1.0F + depth - depth * value;
        break;
    case kModeCentered:
        value = depth * value + (1.0F - depth) * 0.5F;
        break;
    }
    return value * mRange.mLength + mRange.mStart;
}
