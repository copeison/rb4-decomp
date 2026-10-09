#include "audio/fmod/mixing/FmodGain.h"

#include <cmath>
#include <cstdio>
#include <cstring>

// The SDK example's dB conversions. Names not in the reference map.
#define FMOD_GAIN_DB_TO_LINEAR(x) \
    ((x <= FmodGainPlugin::FMOD_GAIN_PARAM_GAIN_MIN) ? 0.0f : powf(10.0f, x / 20.0f))
#define FMOD_GAIN_LINEAR_TO_DB(x) \
    ((x <= 0.0f) ? FmodGainPlugin::FMOD_GAIN_PARAM_GAIN_MIN : 20.0f * log10f(x))

const float FmodGainPlugin::FMOD_GAIN_PARAM_GAIN_MIN = -80.0f;
const float FmodGainPlugin::FMOD_GAIN_PARAM_GAIN_MAX = 10.0f;
const float FmodGainPlugin::FMOD_GAIN_PARAM_GAIN_DEFAULT = 0.0f;

namespace {

// Filled by GetDSPDescription, at 0x19F3758 and 0x19F37B8. Names not in the
// reference map.
FMOD_DSP_PARAMETER_DESC sGainParam;
FMOD_DSP_PARAMETER_DESC sInvertParam;

}  // namespace

FMOD_DSP_PARAMETER_DESC* FmodGainPlugin::mParams[FMOD_GAIN_NUM_PARAMETERS] = {
    &sGainParam,
    &sInvertParam,
};

FMOD_DSP_DESCRIPTION FmodGainPlugin::mDspDescription = {
    110,
    "FMOD Gain",
    0x00010000,
    1,
    1,
    FmodGainPlugin::_Create,
    reinterpret_cast<void*>(FmodGainPlugin::_Release),
    reinterpret_cast<void*>(FmodGainPlugin::_Reset),
    reinterpret_cast<void*>(FmodGainPlugin::_Read),
    nullptr,
    nullptr,
    FMOD_GAIN_NUM_PARAMETERS,
    FmodGainPlugin::mParams,
    reinterpret_cast<void*>(FmodGainPlugin::_SetParamFloat),
    nullptr,
    reinterpret_cast<void*>(FmodGainPlugin::_SetParamBool),
    nullptr,
    reinterpret_cast<void*>(FmodGainPlugin::_GetParamFloat),
    nullptr,
    reinterpret_cast<void*>(FmodGainPlugin::_GetParamBool),
    nullptr,
    reinterpret_cast<void*>(FmodGainPlugin::_ShouldIProcess),
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};

// Reconstructed from eboot.elf at 0x27EBB0.
FMOD_RESULT FmodGainPlugin::_Create(FMOD_DSP_STATE* state) {
    state->plugindata = new FmodGainPlugin();
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27EBF0.
FMOD_RESULT FmodGainPlugin::_Release(FMOD_DSP_STATE* state) {
    delete static_cast<FmodGainPlugin*>(state->plugindata);
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27EC10.
FMOD_RESULT FmodGainPlugin::_Reset(FMOD_DSP_STATE* state) {
    static_cast<FmodGainPlugin*>(state->plugindata)->reset();
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27EC30.
FMOD_RESULT FmodGainPlugin::_Read(
    FMOD_DSP_STATE* state,
    float* inBuffer,
    float* outBuffer,
    unsigned int length,
    int inChannels,
    int*) {
    static_cast<FmodGainPlugin*>(state->plugindata)->process(inBuffer, outBuffer, length, inChannels);
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27EE70.
FMOD_RESULT FmodGainPlugin::_SetParamFloat(FMOD_DSP_STATE* state, int index, float value) {
    if (index != FMOD_GAIN_PARAM_GAIN) {
        return FMOD_ERR_INVALID_PARAM;
    }
    static_cast<FmodGainPlugin*>(state->plugindata)->setGain(value);
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27EF00.
FMOD_RESULT FmodGainPlugin::_SetParamBool(FMOD_DSP_STATE* state, int index, int value) {
    if (index != FMOD_GAIN_PARAM_INVERT) {
        return FMOD_ERR_INVALID_PARAM;
    }
    static_cast<FmodGainPlugin*>(state->plugindata)->setInvert(value != 0);
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27EF40.
FMOD_RESULT FmodGainPlugin::_GetParamFloat(
    FMOD_DSP_STATE* state, int index, float* value, char* valueString) {
    if (index != FMOD_GAIN_PARAM_GAIN) {
        return FMOD_ERR_INVALID_PARAM;
    }
    FmodGainPlugin* plugin = static_cast<FmodGainPlugin*>(state->plugindata);
    *value = plugin->gain();
    if (valueString != nullptr) {
        sprintf(valueString, "%.1f dB", plugin->gain());
    }
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27F050.
FMOD_RESULT FmodGainPlugin::_GetParamBool(
    FMOD_DSP_STATE* state, int index, int* value, char* valueString) {
    if (index != FMOD_GAIN_PARAM_INVERT) {
        return FMOD_ERR_INVALID_PARAM;
    }
    FmodGainPlugin* plugin = static_cast<FmodGainPlugin*>(state->plugindata);
    *value = plugin->mInvert;
    if (valueString != nullptr) {
        sprintf(valueString, plugin->mInvert ? "Inverted" : "Off");
    }
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27F0A0.
FMOD_RESULT FmodGainPlugin::_ShouldIProcess(
    FMOD_DSP_STATE*, int inputsIdle, unsigned int, unsigned int, int, FMOD_SPEAKERMODE) {
    if (inputsIdle) {
        return FMOD_ERR_DSP_DONTPROCESS;
    }
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27F0B0. The parameter setup expands the
// SDK's FMOD_DSP_INIT_PARAMDESC_FLOAT_WITH_MAPPING and _BOOL macros.
FMOD_DSP_DESCRIPTION* FmodGainPlugin::GetDSPDescription() {
    // At 0x19B4F00 and 0x19B4F20.
    static float gain_mapping_values[] = {-80, -50, -30, -10, 10};
    static float gain_mapping_positions[] = {0, 2, 4, 7, 11};

    std::memset(&sGainParam, 0, sizeof(sGainParam));
    sGainParam.type = FMOD_DSP_PARAMETER_TYPE_FLOAT;
    strncpy(sGainParam.name, "Gain", 15);
    strncpy(sGainParam.label, "dB", 15);
    sGainParam.description = "Gain in dB. -80 to 10. Default = 0";
    sGainParam.floatdesc.min = gain_mapping_values[0];
    sGainParam.floatdesc.max = gain_mapping_values[sizeof(gain_mapping_values) / sizeof(float) - 1];
    sGainParam.floatdesc.defaultval = FMOD_GAIN_PARAM_GAIN_DEFAULT;
    sGainParam.floatdesc.mapping.type = FMOD_DSP_PARAMETER_FLOAT_MAPPING_TYPE_PIECEWISE_LINEAR;
    sGainParam.floatdesc.mapping.piecewiselinearmapping.numpoints =
        sizeof(gain_mapping_values) / sizeof(float);
    sGainParam.floatdesc.mapping.piecewiselinearmapping.pointparamvalues = gain_mapping_values;
    sGainParam.floatdesc.mapping.piecewiselinearmapping.pointpositions = gain_mapping_positions;

    std::memset(&sInvertParam, 0, sizeof(sInvertParam));
    sInvertParam.type = FMOD_DSP_PARAMETER_TYPE_BOOL;
    strncpy(sInvertParam.name, "Invert", 15);
    strncpy(sInvertParam.label, "", 15);
    sInvertParam.description = "Invert signal. Default = off";
    sInvertParam.booldesc.defaultval = false;
    sInvertParam.booldesc.valuenames = nullptr;
    return &mDspDescription;
}

// Reconstructed from eboot.elf at 0x27F1C0.
FmodGainPlugin::FmodGainPlugin() {
    mTargetGain = FMOD_GAIN_DB_TO_LINEAR(FMOD_GAIN_PARAM_GAIN_DEFAULT);
    mInvert = false;
    reset();
}

// Reconstructed from eboot.elf at 0x27F1E0.
void FmodGainPlugin::reset() {
    mCurrentGain = mTargetGain;
    mRampSamplesLeft = 0;
}

// Reconstructed from eboot.elf at 0x27F1F0.
void FmodGainPlugin::process(float* inBuffer, float* outBuffer, unsigned int length, int channels) {
    float gain = mCurrentGain;
    if (mRampSamplesLeft) {
        float target = mTargetGain;
        float delta = (target - gain) / mRampSamplesLeft;
        while (length) {
            if (--mRampSamplesLeft) {
                gain += delta;
                for (int i = 0; i < channels; ++i) {
                    *outBuffer++ = *inBuffer++ * gain;
                }
            } else {
                gain = target;
                break;
            }
            --length;
        }
    }
    unsigned int samples = length * channels;
    while (samples--) {
        *outBuffer++ = *inBuffer++ * gain;
    }
    mCurrentGain = gain;
}

// Reconstructed from eboot.elf at 0x27F430.
float FmodGainPlugin::gain() const {
    return FMOD_GAIN_LINEAR_TO_DB(mInvert ? -mTargetGain : mTargetGain);
}

// Reconstructed from eboot.elf at 0x27F490.
void FmodGainPlugin::setGain(float gain) {
    mTargetGain = mInvert ? -FMOD_GAIN_DB_TO_LINEAR(gain) : FMOD_GAIN_DB_TO_LINEAR(gain);
    mRampSamplesLeft = kRampCount;
}

// Reconstructed from eboot.elf at 0x27F510.
void FmodGainPlugin::setInvert(bool invert) {
    if (invert != mInvert) {
        mTargetGain = -mTargetGain;
        mRampSamplesLeft = kRampCount;
    }
    mInvert = invert;
}
