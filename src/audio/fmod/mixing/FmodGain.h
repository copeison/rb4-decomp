#pragma once

#include <cstddef>

#include "audio/fmod/api/fmod_api.h"

// The FMOD SDK's example gain plug-in, "FMOD Gain" (audio/FmodGain.o). Each
// DSP instance owns one as its plugin data. A gain change ramps over 256
// samples; inverting negates the target gain. Field names are not in the
// reference map.
class FmodGainPlugin {
public:
    enum {
        FMOD_GAIN_PARAM_GAIN = 0,
        FMOD_GAIN_PARAM_INVERT = 1,
        FMOD_GAIN_NUM_PARAMETERS = 2,
    };
    static constexpr int kRampCount = 256;  // Name not in the reference map.

    static const float FMOD_GAIN_PARAM_GAIN_MIN;      // -80 dB
    static const float FMOD_GAIN_PARAM_GAIN_MAX;      // 10 dB
    static const float FMOD_GAIN_PARAM_GAIN_DEFAULT;  // 0 dB

    // Fills the parameter descriptions and returns mDspDescription
    // (0x19B4E20). At 0x27F0B0.
    static FMOD_DSP_DESCRIPTION* GetDSPDescription();

    static FMOD_RESULT _Create(FMOD_DSP_STATE* state);   // 0x27EBB0
    static FMOD_RESULT _Release(FMOD_DSP_STATE* state);  // 0x27EBF0
    static FMOD_RESULT _Reset(FMOD_DSP_STATE* state);    // 0x27EC10
    static FMOD_RESULT _Read(
        FMOD_DSP_STATE* state,
        float* inBuffer,
        float* outBuffer,
        unsigned int length,
        int inChannels,
        int* outChannels);  // 0x27EC30
    static FMOD_RESULT _SetParamFloat(FMOD_DSP_STATE* state, int index, float value);  // 0x27EE70
    static FMOD_RESULT _SetParamBool(FMOD_DSP_STATE* state, int index, int value);     // 0x27EF00
    static FMOD_RESULT _GetParamFloat(
        FMOD_DSP_STATE* state, int index, float* value, char* valueString);  // 0x27EF40
    static FMOD_RESULT _GetParamBool(
        FMOD_DSP_STATE* state, int index, int* value, char* valueString);    // 0x27F050
    static FMOD_RESULT _ShouldIProcess(
        FMOD_DSP_STATE* state,
        int inputsIdle,
        unsigned int length,
        unsigned int inMask,
        int inChannels,
        FMOD_SPEAKERMODE speakerMode);  // 0x27F0A0

    FmodGainPlugin();  // 0x27F1C0
    void reset();      // 0x27F1E0
    // Applies the gain to interleaved samples, ramping first. At 0x27F1F0.
    void process(float* inBuffer, float* outBuffer, unsigned int length, int channels);
    // The target gain in dB. Like the SDK example's macro, the inverted case
    // tests the negated gain itself rather than comparing it with zero, so
    // an inverted nonzero gain reads as the minimum. At 0x27F430.
    float gain() const;
    void setGain(float gain);       // 0x27F490
    void setInvert(bool invert);    // 0x27F510

    static FMOD_DSP_DESCRIPTION mDspDescription;  // 0x19B4E20
    static FMOD_DSP_PARAMETER_DESC* mParams[FMOD_GAIN_NUM_PARAMETERS];  // 0x19B4E10

    float mTargetGain;  // Linear, negated while inverted.
    float mCurrentGain;
    int mRampSamplesLeft;
    bool mInvert;
};

static_assert(offsetof(FmodGainPlugin, mRampSamplesLeft) == 8);
static_assert(offsetof(FmodGainPlugin, mInvert) == 12);
static_assert(sizeof(FmodGainPlugin) == 16);
