#include "audio/fmod/mixing/SmbPitchShiftPlugin.h"

#include <cmath>
#include <cstdio>
#include <cstring>

#include "audio/core/dsp/SmbPitchShift.h"

namespace {

// Names not in the reference map.
const int kWindowSizes[] = {128, 256, 512, 1024, 2048, 4096};  // 0x125D9B0
const char* sWindowSizeNames[] = {"128", "256", "512", "1024", "2048", "4096"};  // 0x19B5120
const char* sOverlapNames[] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16",
};  // 0x19B5150

// Filled by GetDSPDescription, from 0x19F3878.
FMOD_DSP_PARAMETER_DESC sWindowParam;
FMOD_DSP_PARAMETER_DESC sOverlapParam;
FMOD_DSP_PARAMETER_DESC sShiftParam;
FMOD_DSP_PARAMETER_DESC sMonoCutoffParam;
FMOD_DSP_PARAMETER_DESC sWetDryParam;

float ClampFloat(float value, float low, float high) {
    return value > high ? high : (value > low ? value : low);
}

int ClampInt(int value, int low, int high) {
    return value > high ? high : (value > low ? value : low);
}

// Stand-ins for the SDK's FMOD_DSP_INIT_PARAMDESC_INT and _FLOAT macros,
// which the getter expands inline.
void InitParamInt(
    FMOD_DSP_PARAMETER_DESC& param,
    const char* name,
    const char* label,
    const char* description,
    int min,
    int max,
    int defaultValue,
    const char* const* valueNames) {
    std::memset(&param, 0, sizeof(param));
    param.type = FMOD_DSP_PARAMETER_TYPE_INT;
    strncpy(param.name, name, 15);
    strncpy(param.label, label, 15);
    param.description = description;
    param.intdesc.min = min;
    param.intdesc.max = max;
    param.intdesc.defaultval = defaultValue;
    param.intdesc.goestoinf = false;
    param.intdesc.valuenames = valueNames;
}

void InitParamFloat(
    FMOD_DSP_PARAMETER_DESC& param,
    const char* name,
    const char* label,
    const char* description,
    float min,
    float max,
    float defaultValue) {
    std::memset(&param, 0, sizeof(param));
    param.type = FMOD_DSP_PARAMETER_TYPE_FLOAT;
    strncpy(param.name, name, 15);
    strncpy(param.label, label, 15);
    param.description = description;
    param.floatdesc.min = min;
    param.floatdesc.max = max;
    param.floatdesc.defaultval = defaultValue;
    param.floatdesc.mapping.type = FMOD_DSP_PARAMETER_FLOAT_MAPPING_TYPE_AUTO;
}

}  // namespace

FMOD_DSP_PARAMETER_DESC* SmbPitchShiftPlugin::mParams[kNumParams] = {
    &sShiftParam,
    &sMonoCutoffParam,
    &sWindowParam,
    &sOverlapParam,
    &sWetDryParam,
};

// The name is filled in by GetDSPDescription.
FMOD_DSP_DESCRIPTION SmbPitchShiftPlugin::mDspDescription = {
    110,
    "",
    0x00010000,
    1,
    1,
    SmbPitchShiftPlugin::_Create,
    reinterpret_cast<void*>(SmbPitchShiftPlugin::_Release),
    reinterpret_cast<void*>(SmbPitchShiftPlugin::_Reset),
    reinterpret_cast<void*>(SmbPitchShiftPlugin::_Read),
    nullptr,
    nullptr,
    kNumParams,
    SmbPitchShiftPlugin::mParams,
    reinterpret_cast<void*>(SmbPitchShiftPlugin::_SetParamFloat),
    reinterpret_cast<void*>(SmbPitchShiftPlugin::_SetParamInt),
    nullptr,
    nullptr,
    reinterpret_cast<void*>(SmbPitchShiftPlugin::_GetParamFloat),
    reinterpret_cast<void*>(SmbPitchShiftPlugin::_GetParamInt),
    nullptr,
    nullptr,
    reinterpret_cast<void*>(SmbPitchShiftPlugin::_ShouldIProcess),
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};

// Reconstructed from eboot.elf at 0x27F920.
FMOD_DSP_DESCRIPTION* SmbPitchShiftPlugin::GetDSPDescription() {
    strcpy(mDspDescription.name, "HMX.SmbPitchShift");
    InitParamInt(sWindowParam, "Window", "samples", "Window Size for FFT", 0, 5, 4, sWindowSizeNames);
    InitParamInt(sOverlapParam, "Overlap", "", "Window overlap factor", 1, 16, 4, sOverlapNames);
    InitParamFloat(sShiftParam, "Shift", "semitones", "Pitch shift", -12.0f, 12.0f, 0.0f);
    InitParamFloat(
        sMonoCutoffParam, "Mono cutoff", "Hz", "Low frequency mono cutoff", 0.0f, 5000.0f, 100.0f);
    InitParamFloat(sWetDryParam, "Wet/Dry", "", "Wet/Dry", 0.0f, 1.0f, 1.0f);
    return &mDspDescription;
}

// Reconstructed from eboot.elf at 0x27FBE0.
FMOD_RESULT SmbPitchShiftPlugin::_Create(FMOD_DSP_STATE* state) {
    SmbPitchShiftPlugin* plugin = new SmbPitchShiftPlugin();
    state->plugindata = plugin;
    SmbPitchShift* pitchShift = new SmbPitchShift();
    pitchShift->ClearBuffers();
    plugin->mPitchShift = pitchShift;
    int sampleRate;
    state->functions->getsamplerate(state, &sampleRate);
    plugin->mSampleRate = static_cast<float>(sampleRate);
    plugin->mUserData.mPluginData = pitchShift;
    static_cast<FMOD::DSP*>(state->instance)->setUserData(&plugin->mUserData);
    plugin->_ApplySettings();
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27FD40.
FMOD_RESULT SmbPitchShiftPlugin::_Release(FMOD_DSP_STATE* state) {
    SmbPitchShiftPlugin* plugin = static_cast<SmbPitchShiftPlugin*>(state->plugindata);
    SmbPitchShift* pitchShift = plugin->mPitchShift;
    static_cast<FMOD::DSP*>(state->instance)->setUserData(nullptr);
    delete pitchShift;
    delete plugin;
    state->plugindata = nullptr;
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27FD90.
FMOD_RESULT SmbPitchShiftPlugin::_Reset(FMOD_DSP_STATE* state) {
    static_cast<SmbPitchShiftPlugin*>(state->plugindata)->_ApplySettings();
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27FE00. Shifts the first two channels
// in place in the output, then mixes them with the input by the wet/dry
// setting; further channels are silenced.
FMOD_RESULT SmbPitchShiftPlugin::_Read(
    FMOD_DSP_STATE* state,
    float* inBuffer,
    float* outBuffer,
    unsigned int length,
    int inChannels,
    int*) {
    SmbPitchShiftPlugin* plugin = static_cast<SmbPitchShiftPlugin*>(state->plugindata);
    SmbPitchShift* pitchShift = plugin->mPitchShift;
    plugin->mPitchRatio = ClampFloat(plugin->mPitchRatio, 0.5f, 2.0f);
    pitchShift->ProcessChannel(length, inBuffer, outBuffer, 0, inChannels, plugin->mPitchRatio);
    if (inChannels >= 2) {
        pitchShift->ProcessChannel(
            length, inBuffer + 1, outBuffer + 1, 1, inChannels, plugin->mPitchRatio);
    }
    for (int channel = 0; channel < inChannels; ++channel) {
        float wetDry = plugin->mWetDry;
        if (channel < 2 && wetDry == 1.0f) {
            continue;
        }
        float wet = channel < 2 ? wetDry : 0.0f;
        float dry = channel < 2 ? 1.0f - wetDry : 0.0f;
        for (unsigned int i = 0; i < length; ++i) {
            float& out = outBuffer[i * inChannels + channel];
            out = wet * out;
            out = dry * inBuffer[i * inChannels + channel] + out;
        }
    }
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x27FF30.
FMOD_RESULT SmbPitchShiftPlugin::_SetParamFloat(FMOD_DSP_STATE* state, int index, float value) {
    SmbPitchShiftPlugin* plugin = static_cast<SmbPitchShiftPlugin*>(state->plugindata);
    switch (index) {
    case kParamShift:
        plugin->mPitchRatio = exp2f(ClampFloat(value / 12.0f, -1.0f, 1.0f));
        return FMOD_OK;
    case kParamMonoCutoff:
        plugin->mMonoCutoff = ClampFloat(value, 0.0f, 5000.0f);
        plugin->mPitchShift->SetMonoCutoff(plugin->mMonoCutoff);
        return FMOD_OK;
    case kParamWetDry:
        // Turning the effect off resets the shifter.
        if (!(value > 0.0f) && plugin->mWetDry > 0.0f) {
            plugin->_ApplySettings();
        }
        plugin->mWetDry = ClampFloat(value, 0.0f, 1.0f);
        return FMOD_OK;
    default:
        return FMOD_ERR_INVALID_PARAM;
    }
}

// Reconstructed from eboot.elf at 0x280060.
FMOD_RESULT SmbPitchShiftPlugin::_SetParamInt(FMOD_DSP_STATE* state, int index, int value) {
    SmbPitchShiftPlugin* plugin = static_cast<SmbPitchShiftPlugin*>(state->plugindata);
    switch (index) {
    case kParamWindow:
        plugin->mWindowSize = kWindowSizes[ClampInt(value, 0, 5)];
        break;
    case kParamOverlap:
        plugin->mOverlap = value;
        break;
    default:
        return FMOD_ERR_INVALID_PARAM;
    }
    plugin->_ApplySettings();
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x280130.
FMOD_RESULT SmbPitchShiftPlugin::_GetParamFloat(
    FMOD_DSP_STATE* state, int index, float* value, char* valueString) {
    SmbPitchShiftPlugin* plugin = static_cast<SmbPitchShiftPlugin*>(state->plugindata);
    switch (index) {
    case kParamShift:
        *value = logf(plugin->mPitchRatio) * (12.0f / 0.693147181f);
        break;
    case kParamMonoCutoff:
        *value = plugin->mMonoCutoff;
        break;
    case kParamWetDry:
        *value = plugin->mWetDry;
        break;
    default:
        return FMOD_ERR_INVALID_PARAM;
    }
    if (valueString != nullptr) {
        sprintf(valueString, "%.2f", *value);
    }
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x2801C0.
FMOD_RESULT SmbPitchShiftPlugin::_GetParamInt(
    FMOD_DSP_STATE* state, int index, int* value, char* valueString) {
    SmbPitchShiftPlugin* plugin = static_cast<SmbPitchShiftPlugin*>(state->plugindata);
    switch (index) {
    case kParamWindow: {
        int sizeIndex;
        if (plugin->mWindowSize == 128) {
            sizeIndex = 0;
        } else if (plugin->mWindowSize == 256) {
            sizeIndex = 1;
        } else if (plugin->mWindowSize == 512) {
            sizeIndex = 2;
        } else if (plugin->mWindowSize == 1024) {
            sizeIndex = 3;
        } else if (plugin->mWindowSize == 2048) {
            sizeIndex = 4;
        } else {
            sizeIndex = 5;
        }
        *value = sizeIndex;
        if (valueString != nullptr) {
            strcpy(valueString, sWindowSizeNames[sizeIndex]);
        }
        return FMOD_OK;
    }
    case kParamOverlap:
        *value = plugin->mOverlap;
        if (valueString != nullptr) {
            sprintf(valueString, "%i", plugin->mOverlap);
        }
        return FMOD_OK;
    default:
        return FMOD_ERR_INVALID_PARAM;
    }
}

// Reconstructed from eboot.elf at 0x2802C0.
FMOD_RESULT SmbPitchShiftPlugin::_ShouldIProcess(
    FMOD_DSP_STATE* state, int inputsIdle, unsigned int, unsigned int, int, FMOD_SPEAKERMODE) {
    if (inputsIdle) {
        return FMOD_ERR_DSP_DONTPROCESS;
    }
    if (static_cast<SmbPitchShiftPlugin*>(state->plugindata)->mWetDry == 0.0f) {
        return FMOD_ERR_DSP_DONTPROCESS;
    }
    return FMOD_OK;
}

// Reconstructed from eboot.elf at 0x2802E0.
SmbPitchShiftPlugin::SmbPitchShiftPlugin()
    : mPitchShift(nullptr),
      mWindowSize(1024),
      mOverlap(4),
      mPitchRatio(1.0f),
      mMonoCutoff(100.0f),
      mWetDry(1.0f),
      mSampleRate(48000.0f) {
    mUserData.mMagic = kPluginUserDataMagic;
    mUserData.mPluginData = nullptr;
    mUserData.mGenerator = nullptr;
    mUserData.mTempoListener = nullptr;
}

// Reconstructed from eboot.elf at 0x280330 (complete) and 0x280340
// (deleting).
SmbPitchShiftPlugin::~SmbPitchShiftPlugin() {}

// Reconstructed from eboot.elf at 0x280350. Keeps only the lowest set bit
// of the window size, which leaves the power-of-two sizes unchanged.
void SmbPitchShiftPlugin::_ApplySettings() {
    int windowSize = ClampInt(mWindowSize, 128, 4096);
    mWindowSize = windowSize & -windowSize;
    mOverlap = ClampInt(mOverlap, 1, 16);
    mPitchShift->Setup(mWindowSize, mOverlap);
    mPitchShift->SetSampleRate(mSampleRate);
}

// Reconstructed from eboot.elf at 0x2803C0.
SmbPitchShift* SmbPitchShiftPlugin::GetPitchShift() const {
    return mPitchShift;
}

// Reconstructed from eboot.elf at 0x2803D0.
const AudioData* SmbPitchShiftPlugin::GetAudioData() const {
    return mPitchShift->mAudioData;
}
