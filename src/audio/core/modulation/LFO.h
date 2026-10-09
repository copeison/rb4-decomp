#pragma once

#include <cstddef>

// Low-frequency oscillator (audio/LFO.o). The class has not been
// reconstructed; only the members FusionVoice uses are declared. The object
// is 48 bytes.
class LFO {
public:
    // One element of a Fusion patch's "lfos" array. Field names follow the
    // properties the patch registry (0x6A520) binds to their offsets.
    struct Settings {
        // "shape", in the order of the wave table at 0x19B0140. Value names
        // not in the reference map.
        enum Shape : unsigned char {
            kShapeSine = 0,
            kShapeSquare = 1,
            kShapeSawtoothUp = 2,
            kShapeSawtoothDown = 3,
            kShapeTriangle = 4,
        };
        // "target": "none", "pan", "pitch" or "filter_freq". Value names
        // not in the reference map.
        enum Target : unsigned char {
            kTargetNone = 0,
            kTargetPan = 1,
            kTargetPitch = 2,
            kTargetFilterFreq = 3,
        };

        Shape mShape;
        Target mTarget;
        bool mEnabled;
        bool mRetrigger;     // Restart at the initial phase on each note.
        bool mBeatSync;      // Scale the frequency by the tempo.
        float mFrequency;    // Hz.
        float mDepth;
        float mInitialPhase;
        float mDefaultTempo;  // BPM.
    };

    LFO();  // 0xBE510

    // The map's SetPhase(double) is larger; this build only stores it. At
    // 0xBE550.
    void SetPhase(double phase);
    // Binds the settings and recomputes the phase increment. At 0xBE570.
    void UseSettings(const Settings* settings);
    double GetPhase() const;  // 0xBE610
    // Restarts at the settings' initial phase. At 0xBE620.
    void Retrigger();
    void Advance(unsigned int numSamples);  // 0xBE650
    // Stores the sample period and recomputes the phase increment. At
    // 0xBE6D0.
    void Prepare(float sampleRate);
    // The shaped wave in the output range, scaled by the depth while the
    // settings are enabled. At 0xBE730.
    float GetValue() const;

    // Field names are not in the reference map.
    double mPhase;
    // The output range, as an offset and a span: the constructor maps the
    // [0, 1] wave onto [-1, 1].
    float mRangeOffset;
    float mRangeSpan;
    int mDepthMode;  // How GetValue applies the depth (0-2).
    const Settings* mSettings;
    double mPhaseIncrement;    // Cycles per sample.
    float mSecondsPerSample;
    float mTempo;  // The tempo the increment was computed for.
};

static_assert(offsetof(LFO::Settings, mFrequency) == 8);
static_assert(offsetof(LFO::Settings, mDefaultTempo) == 20);
static_assert(sizeof(LFO::Settings) == 24);
static_assert(offsetof(LFO, mRangeOffset) == 8);
static_assert(offsetof(LFO, mDepthMode) == 16);
static_assert(offsetof(LFO, mSettings) == 24);
static_assert(offsetof(LFO, mPhaseIncrement) == 32);
static_assert(offsetof(LFO, mTempo) == 44);
static_assert(sizeof(LFO) == 48);
