#pragma once

#include <cstddef>

// Two-pole, two-zero filter (audio/BiquadFilter.o). The map's object holds
// only the Coefs and Settings members; the filter itself is inline. Only what
// FusionVoice uses is declared. The object is 208 bytes.
class BiquadFilter {
public:
    // The settings of a patch's "filter" struct. Field names are not in the
    // reference map.
    struct Settings {
        // The response type, in the order of Coefs::MakeFromSettings's
        // cases at 0xD9B40: the map's _Make*Coefs members matched by their
        // formulas. Value names not in the reference map.
        enum Type : unsigned char {
            kTypeLowPass = 0,
            kTypeHighPass = 1,
            kTypeBandPass = 2,
            kTypePeaking = 3,
            kTypeLowShelf = 4,
            kTypeHighShelf = 5,
        };

        // Inlined into FusionVoice's constructor at 0x9D920 and
        // FusionSampler's at 0x95C40.
        Settings() : mType(kTypeLowPass), mEnabled(false), mFrequency(630.0f), mQ(1.0f), mGain(0.0f) {}

        Type mType;
        bool mEnabled;
        float mFrequency;  // Hz.
        float mQ;
        float mGain;       // dB, for the peaking and shelf types.
    };

    // Normalized direct-form coefficients for one sample rate. Field names
    // are not in the reference map.
    class Coefs {
    public:
        // A pass-through. Inlined into FusionVoice's constructor at
        // 0x9D920.
        Coefs() : mB0(1.0), mB1(0.0), mB2(0.0), mA1(0.0), mA2(0.0), mSampleRate(1.0) {}
        // Leaves the coefficients for MakeFromSettings. Inlined into
        // FusionVoice::_UpdateFilterSettings at 0x9F150.
        explicit Coefs(double sampleRate) : mSampleRate(sampleRate) {}

        // Resets to a pass-through, then designs the settings' response at
        // the stored sample rate when they are enabled. At 0xD9B40.
        void MakeFromSettings(const Settings& settings);

        // The responses of the RBJ audio EQ cookbook. Each clamps the
        // frequency to the Nyquist rate. MakeFromSettings inlines them.
        void _MakeLowPassCoefs(float frequency, float q);
        void _MakeHighPassCoefs(float frequency, float q);
        void _MakeBandPassCoefs(float frequency, float q);
        // The bandwidth in octaves, the gain in decibels.
        void _MakePeakingCoefs(float frequency, float bandwidth, float gainDb);
        // The shelf slope, the gain in decibels.
        void _MakeLowShelfCoefs(float frequency, float slope, float gainDb);
        void _MakeHighShelfCoefs(float frequency, float slope, float gainDb);

        // Whether the coefficients pass the signal through. Inlined into
        // FusionVoice::Process at 0x9F2A0. Name not in the reference map.
        bool IsPassThrough() const {
            return mB0 == 1.0 && mB1 == 0.0 && mB2 == 0.0 && mA1 == 0.0 && mA2 == 0.0;
        }
        // Element-wise arithmetic for coefficient ramps; the results keep the
        // default sample rate. Inlined into FusionVoice::_UpdateFilterSettings
        // and FusionVoice::Process. Not in the reference map.
        Coefs operator+(const Coefs& other) const {
            Coefs result;
            result.mB0 = mB0 + other.mB0;
            result.mB1 = mB1 + other.mB1;
            result.mB2 = mB2 + other.mB2;
            result.mA1 = mA1 + other.mA1;
            result.mA2 = mA2 + other.mA2;
            return result;
        }
        Coefs operator-(const Coefs& other) const {
            Coefs result;
            result.mB0 = mB0 - other.mB0;
            result.mB1 = mB1 - other.mB1;
            result.mB2 = mB2 - other.mB2;
            result.mA1 = mA1 - other.mA1;
            result.mA2 = mA2 - other.mA2;
            return result;
        }
        Coefs operator*(double scale) const {
            Coefs result;
            result.mB0 = mB0 * scale;
            result.mB1 = mB1 * scale;
            result.mB2 = mB2 * scale;
            result.mA1 = mA1 * scale;
            result.mA2 = mA2 * scale;
            return result;
        }

        double mB0;
        double mB1;
        double mB2;
        double mA1;
        double mA2;
        double mSampleRate;
    };

    // Inlined into FusionVoice's constructor at 0x9D920.
    BiquadFilter()
        : mHistory(),
          mRampIncrement(1.0f),
          mRampProgress(1.0f),
          mRate(1.0f),
          mUpdating(false),
          mCallback(nullptr) {}

    // Clears both channels' delay lines. Inlined into
    // FusionVoice::_PrepareWithPitchOffsetAndGain at 0x9E6B0. Name not in
    // the reference map.
    void ResetHistory() {
        for (auto& channel : mHistory) {
            channel[0] = 0.0;
            channel[1] = 0.0;
        }
    }
    // Sets the control rate and the coefficient ramp length. Inlined into
    // FusionVoice::SetSampleRate at 0x9DC50. Name not in the reference map.
    void SetRate(float rate, float rampSeconds) {
        mRate = rate;
        mRampIncrement = 1.0f / rate * (1.0f / rampSeconds);
    }
    // Starts a ramp from the current coefficients to the target. Inlined
    // into FusionVoice::_UpdateFilterSettings at 0x9F150. Name not in the
    // reference map.
    void SetTargetCoefs(const Coefs& target, void (*callback)(void*), void* data) {
        mCoefsStep = (target - mCoefs) * static_cast<double>(mRampIncrement);
        mUpdating = true;
        mTargetCoefs = target;
        mRampProgress = 0.0f;
        mUpdating = false;
        mCallback = callback;
        mCallbackData = data;
    }
    // Jumps to the target coefficients and fires the callback once, unless
    // SetTargetCoefs is in progress. Inlined into
    // FusionVoice::_PrepareWithPitchOffsetAndGain at 0x9E6B0. Name not in
    // the reference map.
    void FinishRamp() {
        if (!mUpdating) {
            mCoefs = mTargetCoefs;
            mRampProgress = 1.0f;
            if (mCallback != nullptr) {
                mCallback(mCallbackData);
            }
            mCallback = nullptr;
            mCallbackData = nullptr;
        }
    }
    // Moves the coefficients one control block along the ramp. Inlined into
    // FusionVoice::Process at 0x9F2A0. Name not in the reference map.
    void AdvanceRamp() {
        if (mRampProgress != 1.0f) {
            mRampProgress += mRampIncrement;
            mCoefs = mCoefs + mCoefsStep;
            if (mRampProgress >= 1.0f) {
                FinishRamp();
            }
        }
    }
    // Whether the current and the target coefficients both pass the signal
    // through. Name not in the reference map.
    bool IsPassThrough() const {
        return mCoefs.IsPassThrough() && mTargetCoefs.IsPassThrough();
    }
    // Filters one channel in place (direct form II) and scales the output.
    // Inlined into FusionVoice::Process at 0x9F2A0. Name not in the
    // reference map.
    void Filter(float* samples, unsigned int numSamples, int channel, float gain) {
        double outputGain = gain;
        double w1 = mHistory[channel][0];
        double w2 = mHistory[channel][1];
        for (unsigned int i = 0; i < numSamples; ++i) {
            double w = static_cast<double>(samples[i]) - mCoefs.mA1 * w1 - mCoefs.mA2 * w2;
            double y = w2 * mCoefs.mB2 + w1 * mCoefs.mB1 + w * mCoefs.mB0;
            samples[i] = static_cast<float>(y * outputGain);
            w2 = w1;
            w1 = w;
        }
        mHistory[channel][0] = w1;
        mHistory[channel][1] = w2;
    }

    // Field names are not in the reference map.
    double mHistory[2][2];  // Per channel: the last two intermediate values.
    Coefs mCoefs;
    Coefs mTargetCoefs;
    Coefs mCoefsStep;      // Change per control block while ramping.
    float mRampIncrement;  // Progress per control block.
    float mRampProgress;   // 0 to 1; 1 when idle.
    float mRate;           // Control blocks per second.
    // Set while SetTargetCoefs runs, so that the audio thread does not finish
    // a half-written ramp.
    volatile bool mUpdating;
    void (*mCallback)(void*);
    void* mCallbackData;
};

static_assert(offsetof(BiquadFilter::Settings, mFrequency) == 4);
static_assert(sizeof(BiquadFilter::Settings) == 16);
static_assert(sizeof(BiquadFilter::Coefs) == 48);
static_assert(offsetof(BiquadFilter, mCoefs) == 32);
static_assert(offsetof(BiquadFilter, mTargetCoefs) == 80);
static_assert(offsetof(BiquadFilter, mCoefsStep) == 128);
static_assert(offsetof(BiquadFilter, mRampIncrement) == 176);
static_assert(offsetof(BiquadFilter, mUpdating) == 188);
static_assert(offsetof(BiquadFilter, mCallback) == 192);
static_assert(sizeof(BiquadFilter) == 208);
