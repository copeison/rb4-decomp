#include "audio/core/dsp/DistortionEffect.h"

#include <cmath>

#include "audio/core/system/Audio.h"
#include "utl/text/Str.h"

namespace {

// The symmetric 32-tap low-pass that both the up- and the downsamplers use,
// at 0x124E080. Name not in the reference map.
const float kOversampleTaps[32] = {
    0.004184821f,  0.013551255f,  0.012686975f,  0.01469913f,
    0.008901898f,  -0.0012622656f, -0.015190761f, -0.027898235f,
    -0.033905827f, -0.027685944f, -0.0060992166f, 0.030271528f,
    0.07652286f,   0.12425289f,   0.1635898f,    0.18580517f,
    0.18580517f,   0.1635898f,    0.12425289f,   0.07652286f,
    0.030271528f,  -0.0060992166f, -0.027685944f, -0.033905827f,
    -0.027898235f, -0.015190761f, -0.0012622656f, 0.008901898f,
    0.01469913f,   0.012686975f,  0.013551255f,  0.004184821f,
};

// Direct form II over one filter's channel history; the same arithmetic as
// BiquadFilter::Filter without the output gain. _ProcessChannel inlines it
// six times. Name not in the reference map.
void FilterSamples(const BiquadFilter::Coefs& coefs, double* history, float* samples, unsigned int numSamples) {
    double w1 = history[0];
    double w2 = history[1];
    for (unsigned int i = 0; i < numSamples; ++i) {
        double w = static_cast<double>(samples[i]) - coefs.mA1 * w1 - coefs.mA2 * w2;
        samples[i] = static_cast<float>(w2 * coefs.mB2 + w1 * coefs.mB1 + w * coefs.mB0);
        w2 = w1;
        w1 = w;
    }
    history[0] = w1;
    history[1] = w2;
}

}  // namespace

// Reconstructed from eboot.elf at 0xDCD50. Every oversampling filter shares
// the static taps.
DistortionEffect::DistortionEffect()
    : mType(Settings::kTypeClean),
      mInputGain(1.0f),
      mOutputGain(1.0f),
      mBias(0.0f),
      mFilterHistory(),
      mOversample(false),
      mSampleRate(static_cast<int>(Audio::GetSamplesPerSecond())) {
    for (int i = 0; i < kNumFilters; ++i) {
        mFilterEnabled[i] = false;
        mFilterPreClip[i] = false;
    }
    for (int i = 0; i < kMaxChannels; ++i) {
        mUpsamplers[i].Init(kOversampleTaps, 32, false);
        mDownsamplers[i].Init(kOversampleTaps, 32, false);
    }
}

// Reconstructed from eboot.elf at 0xDD4D0.
eastl::vector<AllowedValue<int>> DistortionEffect::Settings::GetAllowedTypes() {
    eastl::vector<AllowedValue<int>> values;
    values.reserve(5);
    values.emplace_back(AllowedValue<int>{kTypeClean, String("Clean"), String("")});
    values.emplace_back(AllowedValue<int>{kTypeWarm, String("Warm"), String("")});
    values.emplace_back(AllowedValue<int>{kTypeDirty, String("Dirty"), String("")});
    values.emplace_back(AllowedValue<int>{kTypeSoft, String("Soft"), String("")});
    values.emplace_back(AllowedValue<int>{kTypeAsymmetric, String("Asymmetric"), String("")});
    return values;
}

// Reconstructed from eboot.elf at 0xDD820.
void DistortionEffect::SetInputGainDb(float db) {
    mInputGain = std::pow(10.0f, db * 0.05f);
}

// Reconstructed from eboot.elf at 0xDD850.
void DistortionEffect::SetOutputGainDb(float db) {
    mOutputGain = std::pow(10.0f, db * 0.05f);
}

// Reconstructed from eboot.elf at 0xDD880.
void DistortionEffect::SetType(int type) {
    mType = type > Settings::kTypeAsymmetric ? Settings::kTypeAsymmetric : (type < 0 ? 0 : type);
}

// Reconstructed from eboot.elf at 0xDD8A0. An enabled filter is designed at
// the effect's sample rate and its history cleared.
void DistortionEffect::SetupFilter(int index, const BiquadFilter::Settings& settings) {
    mFilterEnabled[index] = settings.mEnabled;
    if (settings.mEnabled) {
        mFilterCoefs[index].mSampleRate = static_cast<float>(mSampleRate);
        mFilterCoefs[index].MakeFromSettings(settings);
        for (auto& channel : mFilterHistory[index]) {
            channel[0] = 0.0;
            channel[1] = 0.0;
        }
    }
}

// Reconstructed from eboot.elf at 0xDD920. Turning oversampling on clears the
// filters and sizes the buffer for four times the engine's block.
void DistortionEffect::SetOversample(bool oversample) {
    if (oversample && !mOversample) {
        for (int i = 0; i < kMaxChannels; ++i) {
            mUpsamplers[i].ClearHistory();
            mDownsamplers[i].ClearHistory();
        }
        if (mOversampleBuffer.mNumValidFrames != Audio::sBufferSize * kOversampling) {
            mOversampleBuffer.Configure(
                AudioBufferConfig(1, Audio::sBufferSize * kOversampling, static_cast<float>(mSampleRate), false),
                AudioBufferBase::kCleanupFree);
        }
    }
    mOversample = oversample;
}

// Reconstructed from eboot.elf at 0xDDA40.
void DistortionEffect::SetSampleRate(int sampleRate) {
    mSampleRate = sampleRate;
}

// Reconstructed from eboot.elf at 0xDDA50. The pre-clip filters run, the
// channel is upsampled when oversampling, every sample goes through the
// type's curve between the gains, and the downsampled result is filtered by
// the remaining filters. Each clip loop is vectorized in the binary.
void DistortionEffect::_ProcessChannel(int channel, float* samples, int numFrames) {
    unsigned int numSamples = numFrames;
    for (int i = 0; i < kNumFilters; ++i) {
        if (mFilterEnabled[i] && mFilterPreClip[i]) {
            FilterSamples(mFilterCoefs[i], mFilterHistory[i][channel], samples, numSamples);
        }
    }

    float* data = samples;
    int count = numFrames;
    if (mOversample) {
        float* oversampled = mOversampleBuffer.mChannelData[0];
        for (int i = 0; i < numFrames; ++i) {
            mUpsamplers[channel].Upsample4x(samples[i], oversampled + kOversampling * i);
        }
        data = mOversampleBuffer.mChannelData[0];
        count = numFrames * kOversampling;
    }

    switch (mType) {
    case Settings::kTypeClean:
        for (int i = 0; i < count; ++i) {
            data[i] = std::tanh((mBias + data[i]) * mInputGain) * mOutputGain;
        }
        break;
    case Settings::kTypeWarm:
        for (int i = 0; i < count; ++i) {
            data[i] = std::sin((mBias + data[i]) * mInputGain) * mOutputGain;
        }
        break;
    case Settings::kTypeDirty:
        for (int i = 0; i < count; ++i) {
            float x = (mBias + data[i]) * mInputGain;
            x = x > 1.0f ? 1.0f : (x > -1.0f ? x : -1.0f);
            data[i] = x * mOutputGain;
        }
        break;
    case Settings::kTypeSoft:
        // 0.66666 stands for the 2/3 that x - x^3/3 reaches at 1.
        for (int i = 0; i < count; ++i) {
            float x = (mBias + data[i]) * mInputGain;
            float y;
            if (x > 1.0f) {
                y = 0.66666f;
            } else if (x < -1.0f) {
                y = -0.66666f;
            } else {
                y = x + x * x * (-1.0f / 3.0f) * x;
            }
            data[i] = y * mOutputGain;
        }
        break;
    case Settings::kTypeAsymmetric:
        // A tube-style curve on half the input: a constant above 0.32, a
        // quadratic down to -0.089, then a power-12 knee, and -0.98 below -1.
        for (int i = 0; i < count; ++i) {
            float x = (0.5f * data[i] + mBias) * mInputGain;
            float y;
            if (x >= 0.320018f) {
                y = 0.630035f;
            } else if (x >= -0.08905f) {
                y = x * 3.9375f + x * x * -6.153f;
            } else if (x < -1.0f) {
                y = -0.9818f;
            } else {
                float magnitude = std::fabs(x);
                float knee = 1.032847f - magnitude;
                float knee3 = knee * knee * knee;
                float knee6 = knee3 * knee3;
                float knee12 = knee6 * knee6;
                y = (knee12 + ((0.032847f - magnitude) * 0.333f + -1.0f)) * 0.75f + 0.01f;
            }
            data[i] = y * mOutputGain;
        }
        break;
    default:
        break;
    }

    if (mOversample) {
        float* oversampled = mOversampleBuffer.mChannelData[0];
        for (int i = 0; i < numFrames; ++i) {
            mDownsamplers[channel].AddData(oversampled + kOversampling * i, kOversampling);
            samples[i] = mDownsamplers[channel].GetSample();
        }
    }

    for (int i = 0; i < kNumFilters; ++i) {
        if (mFilterEnabled[i] && !mFilterPreClip[i]) {
            FilterSamples(mFilterCoefs[i], mFilterHistory[i][channel], samples, numSamples);
        }
    }
}

// Reconstructed from eboot.elf at 0xDE8D0. The buffer must be planar. The
// map's version is four times longer.
void DistortionEffect::Process(AudioBuffer<float>& buffer) {
    for (int channel = 0; channel < buffer.mNumChannels; ++channel) {
        _ProcessChannel(channel, buffer.mChannelData[channel], buffer.mNumFrames);
    }
}
