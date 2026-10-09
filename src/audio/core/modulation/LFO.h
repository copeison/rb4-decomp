#pragma once

#include <cstddef>

#include "audio/core/dsp/ParameterSpec.h"
#include "audio/core/modulation/ModulatorTarget.h"
#include "utl/containers/Vector.h"

// Low-frequency oscillator (audio/LFO.o, 0xBDD10 to 0xBE7BF). Its phase runs
// from 0 to 1 at the settings' frequency, optionally scaled by the tempo.
// The object is 48 bytes.
class LFO {
public:
    // One element of a Fusion patch's "lfos" array. Field names follow the
    // properties the patch registry (0x6A520) binds to their offsets.
    struct Settings {
        // "shape", in the order of sApplyWaveshapingTbl. Value names not in
        // the reference map.
        enum Shape : unsigned char {
            kShapeSine = 0,
            kShapeSquare = 1,
            kShapeSawtoothUp = 2,
            kShapeSawtoothDown = 3,
            kShapeTriangle = 4,
            // LFO::Noise has no name and no table entry.
            kShapeNoise = 5,
            kShapeInvalid = 6,  // StringToShape's result for an unknown name.
        };
        // "target": "none", "pan", "pitch" or "filter_freq". Value names
        // not in the reference map.
        enum Target : unsigned char {
            kTargetNone = 0,
            kTargetPan = 1,
            kTargetPitch = 2,
            kTargetFilterFreq = 3,
            // StringToTarget's result for an unknown name; the value between
            // has no name.
            kTargetInvalid = 5,
        };

        // The table at 0x18E59E0; null past the last target. At 0xBDDD0.
        static const char* TargetToString(Target target);
        static Target StringToTarget(const char* name);  // 0xBDDF0
        // The table at 0x18E5A00; null past the triangle. At 0xBDE70.
        static const char* ShapeToString(Shape shape);
        static Shape StringToShape(const char* name);  // 0xBDE90
        // The editor values with their help. At 0xBDF20 and 0xBE1D0.
        static eastl::vector<AllowedValue<unsigned char>> GetAllowedTargetValues();
        static eastl::vector<AllowedValue<unsigned char>> GetAllowedShapeValues();

        // The patch's defaults, built by FusionSampler's constructor at
        // 0x95C40.
        Settings()
            : mShape(kShapeSine),
              mTarget(kTargetNone),
              mEnabled(false),
              mRetrigger(false),
              mBeatSync(false),
              mFrequency(4.0F),
              mDepth(0.5F),
              mInitialPhase(0.0F),
              mDefaultTempo(120.0F) {}

        Shape mShape;
        Target mTarget;
        bool mEnabled;
        bool mRetrigger;     // Restart at the initial phase on each note.
        bool mBeatSync;      // Scale the frequency by the tempo.
        float mFrequency;    // Hz, or cycles per beat when beat synced.
        float mDepth;
        float mInitialPhase;  // Degrees.
        float mDefaultTempo;  // BPM, "default_tempo": used when the host sets none.
    };

    // How GetValue applies the depth; the same shapes as
    // ModulatorTarget::DepthMode. Value names are not in the reference map.
    enum Mode : int {
        kModeAdditive = 0,
        kModeSubtractive = 1,
        kModeCentered = 2,
    };

    // The wave shapes, mapping a phase in [0, 1) to [0, 1]. Sine starts at
    // its trough. At 0xBDD10 through 0xBDDB0.
    static float Sine(float phase);
    static float Square(float phase);
    static float SawtoothUp(float phase);
    static float SawtoothDown(float phase);
    static float Triangle(float phase);
    static float Noise(float phase);

    // The shapes by Settings::Shape. At 0x19B0140.
    static float (*const sApplyWaveshapingTbl[5])(float phase);

    // A centered oscillator over [-1, 1]. At 0xBE510.
    LFO();

    // The map's SetPhase(double) is larger; this build only stores it. At
    // 0xBE550.
    void SetPhase(double phase);
    // At 0xBE560; nothing calls it in this build.
    void SetRangeAndMode(SPL::Range<float> range, Mode mode);
    // Binds the settings and recomputes the phase increment. At 0xBE570.
    void UseSettings(const Settings* settings);
    // The phase increment from the frequency, scaled by the tempo when beat
    // synced. Inlined into UseSettings, Advance and Prepare. At 0xBE5C0.
    void ComputeCyclesPerSample();
    double GetPhase() const;  // 0xBE610
    // Restarts at the settings' initial phase, a half cycle on. At 0xBE620.
    void Retrigger();
    // Recomputes the increment when a beat-synced tempo changed, then
    // advances the phase. At 0xBE650.
    void Advance(unsigned int numSamples);
    // Stores the sample period and recomputes the phase increment. At
    // 0xBE6D0.
    void Prepare(float sampleRate);
    // The shaped wave scaled by the depth while the settings are enabled,
    // mapped onto the range. At 0xBE730.
    float GetValue() const;

    // The settings' shape at a phase. Inlined into GetValue.
    float ApplyWaveshaping(float phase) const {
        return sApplyWaveshapingTbl[mSettings->mShape](phase);
    }

    // Field names are not in the reference map.
    double mPhase;
    SPL::Range<float> mRange;
    Mode mMode;
    const Settings* mSettings;
    double mCyclesPerSample;
    float mSecondsPerSample;
    float mTempo;  // The settings' tempo the increment was computed for.
};

static_assert(offsetof(LFO::Settings, mFrequency) == 8);
static_assert(offsetof(LFO::Settings, mDefaultTempo) == 20);
static_assert(sizeof(LFO::Settings) == 24);
static_assert(offsetof(LFO, mRange) == 8);
static_assert(offsetof(LFO, mMode) == 16);
static_assert(offsetof(LFO, mSettings) == 24);
static_assert(offsetof(LFO, mCyclesPerSample) == 32);
static_assert(offsetof(LFO, mTempo) == 44);
static_assert(sizeof(LFO) == 48);
