#include "audio/core/dsp/BiquadFilter.h"

#include <cmath>

namespace {

// The angular frequency of a frequency clamped to the Nyquist rate. Inlined
// into each of the map's _Make*Coefs. Name not in the reference map.
double AngularFrequency(float frequency, double sampleRate) {
    float nyquist = static_cast<float>(sampleRate) * 0.5F;
    float clamped = nyquist < frequency ? nyquist : frequency;
    return static_cast<double>(clamped * 6.2831855F) / sampleRate;
}

}  // namespace

// Reconstructed from eboot.elf at 0xD9B40, where MakeFromSettings inlines it.
void BiquadFilter::Coefs::_MakeLowPassCoefs(float frequency, float q) {
    double w0 = AngularFrequency(frequency, mSampleRate);
    double cosw = std::cos(w0);
    double alpha = std::sin(w0) / static_cast<double>(q + q);
    double scale = 1.0 / (alpha + 1.0);
    mB0 = (1.0 - cosw) * 0.5 * scale;
    mB1 = (1.0 - cosw) * scale;
    mB2 = (1.0 - cosw) * 0.5 * scale;
    mA1 = cosw * -2.0 * scale;
    mA2 = (1.0 - alpha) * scale;
}

// Reconstructed from eboot.elf at 0xD9B40, where MakeFromSettings inlines it.
void BiquadFilter::Coefs::_MakeHighPassCoefs(float frequency, float q) {
    double w0 = AngularFrequency(frequency, mSampleRate);
    double cosw = std::cos(w0);
    double alpha = std::sin(w0) / static_cast<double>(q + q);
    double scale = 1.0 / (alpha + 1.0);
    mB0 = 0.5 * (cosw + 1.0) * scale;
    mB1 = -((cosw + 1.0) * scale);
    mB2 = 0.5 * (cosw + 1.0) * scale;
    mA1 = cosw * -2.0 * scale;
    mA2 = scale * (1.0 - alpha);
}

// Reconstructed from eboot.elf at 0xD9B40, where MakeFromSettings inlines
// it. The peak gain is 0 dB.
void BiquadFilter::Coefs::_MakeBandPassCoefs(float frequency, float q) {
    double w0 = AngularFrequency(frequency, mSampleRate);
    double cosw = std::cos(w0);
    double alpha = std::sin(w0) / static_cast<double>(q + q);
    double scale = 1.0 / (alpha + 1.0);
    mB0 = scale * alpha;
    mB1 = 0.0;
    mB2 = -(scale * alpha);
    mA1 = cosw * -2.0 * scale;
    mA2 = (1.0 - alpha) * scale;
}

// Reconstructed from eboot.elf at 0xD9B40, where MakeFromSettings inlines
// it. The bandwidth constant is 2 ln 2.
void BiquadFilter::Coefs::_MakePeakingCoefs(float frequency, float bandwidth, float gainDb) {
    double w0 = AngularFrequency(frequency, mSampleRate);
    double cosw = std::cos(w0);
    double sinw = std::sin(w0);
    double a = std::pow(10.0, static_cast<double>(gainDb * 0.025F));
    double alpha =
        std::sinh(static_cast<double>(bandwidth) * 1.38629436111989 * (w0 / sinw)) * sinw;
    double scale = 1.0 / (alpha / a + 1.0);
    mB0 = scale * (alpha * a + 1.0);
    mB1 = cosw * -2.0 * scale;
    mB2 = (1.0 - alpha * a) * scale;
    mA1 = cosw * -2.0 * scale;
    mA2 = scale * (1.0 - alpha / a);
}

// Reconstructed from eboot.elf at 0xD9B40, where MakeFromSettings inlines it.
void BiquadFilter::Coefs::_MakeLowShelfCoefs(float frequency, float slope, float gainDb) {
    double w0 = AngularFrequency(frequency, mSampleRate);
    double cosw = std::cos(w0);
    double sinw = std::sin(w0);
    float inverseSlope = 1.0F / slope;
    double a = std::pow(10.0, static_cast<double>(gainDb * 0.025F));
    // Twice the square root of A times the cookbook's alpha.
    double k = std::sqrt(a) * sinw *
        std::sqrt((1.0 / a + a) * static_cast<double>(inverseSlope + -1.0F) + 2.0);
    double scale = 1.0 / (k + ((a - 1.0) * cosw + (a + 1.0)));
    mB0 = a * ((a + 1.0) - (a - 1.0) * cosw + k) * scale;
    mB1 = (a + a) * ((a - 1.0) - (a + 1.0) * cosw) * scale;
    mB2 = a * ((a + 1.0) - (a - 1.0) * cosw - k) * scale;
    mA1 = -2.0 * ((a + 1.0) * cosw + (a - 1.0)) * scale;
    mA2 = scale * ((a - 1.0) * cosw + (a + 1.0) - k);
}

// Reconstructed from eboot.elf at 0xD9B40, where MakeFromSettings inlines it.
void BiquadFilter::Coefs::_MakeHighShelfCoefs(float frequency, float slope, float gainDb) {
    double w0 = AngularFrequency(frequency, mSampleRate);
    double cosw = std::cos(w0);
    double sinw = std::sin(w0);
    float inverseSlope = 1.0F / slope;
    double a = std::pow(10.0, static_cast<double>(gainDb * 0.025F));
    double k = std::sqrt(a) * sinw *
        std::sqrt((1.0 / a + a) * static_cast<double>(inverseSlope + -1.0F) + 2.0);
    double scale = 1.0 / (k + ((a + 1.0) - (a - 1.0) * cosw));
    mB0 = a * ((a - 1.0) * cosw + (a + 1.0) + k) * scale;
    mB1 = a * -2.0 * ((a + 1.0) * cosw + (a - 1.0)) * scale;
    mB2 = a * ((a - 1.0) * cosw + (a + 1.0) - k) * scale;
    mA1 = 2.0 * ((a - 1.0) - (a + 1.0) * cosw) * scale;
    mA2 = scale * ((a + 1.0) - (a - 1.0) * cosw - k);
}

// Reconstructed from eboot.elf at 0xD9B40.
void BiquadFilter::Coefs::MakeFromSettings(const Settings& settings) {
    mB0 = 1.0;
    mB1 = 0.0;
    mB2 = 0.0;
    mA1 = 0.0;
    mA2 = 0.0;
    if (!settings.mEnabled) {
        return;
    }
    switch (settings.mType) {
    case Settings::kTypeLowPass:
        _MakeLowPassCoefs(settings.mFrequency, settings.mQ);
        break;
    case Settings::kTypeHighPass:
        _MakeHighPassCoefs(settings.mFrequency, settings.mQ);
        break;
    case Settings::kTypeBandPass:
        _MakeBandPassCoefs(settings.mFrequency, settings.mQ);
        break;
    case Settings::kTypePeaking:
        _MakePeakingCoefs(settings.mFrequency, settings.mQ, settings.mGain);
        break;
    case Settings::kTypeLowShelf:
        _MakeLowShelfCoefs(settings.mFrequency, settings.mQ, settings.mGain);
        break;
    case Settings::kTypeHighShelf:
        _MakeHighShelfCoefs(settings.mFrequency, settings.mQ, settings.mGain);
        break;
    }
}
