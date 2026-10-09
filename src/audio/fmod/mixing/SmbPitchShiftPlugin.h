#pragma once

#include <cstddef>

#include "audio/fmod/api/fmod_api.h"
#include "audio/fmod/system/FmodPlatform.h"

class AudioData;
class SmbPitchShift;

// FMOD DSP plug-in "HMX.SmbPitchShift" wrapping an SmbPitchShift. FModSystem
// registers its description with each Studio system. The map has no object
// for this plug-in; the class, its file and its member names are not in the
// reference map, and the callbacks follow the map's naming for the other
// plug-ins. The vtable is at 0x18F1288; the object is 0x48 bytes.
class SmbPitchShiftPlugin {
public:
    // Parameter indices; names not in the reference map.
    enum {
        kParamShift = 0,       // Semitones, -12 to 12.
        kParamMonoCutoff = 1,  // Hz, 0 to 5000.
        kParamWindow = 2,      // Index into 128 to 4096 samples.
        kParamOverlap = 3,     // 1 to 16.
        kParamWetDry = 4,      // 0 to 1.
        kNumParams = 5,
    };

    // Names the description and fills the parameter descriptions. At
    // 0x27F920.
    static FMOD_DSP_DESCRIPTION* GetDSPDescription();

    static FMOD_RESULT _Create(FMOD_DSP_STATE* state);   // 0x27FBE0
    static FMOD_RESULT _Release(FMOD_DSP_STATE* state);  // 0x27FD40
    static FMOD_RESULT _Reset(FMOD_DSP_STATE* state);    // 0x27FD90
    static FMOD_RESULT _Read(
        FMOD_DSP_STATE* state,
        float* inBuffer,
        float* outBuffer,
        unsigned int length,
        int inChannels,
        int* outChannels);  // 0x27FE00
    static FMOD_RESULT _SetParamFloat(FMOD_DSP_STATE* state, int index, float value);  // 0x27FF30
    static FMOD_RESULT _SetParamInt(FMOD_DSP_STATE* state, int index, int value);      // 0x280060
    static FMOD_RESULT _GetParamFloat(
        FMOD_DSP_STATE* state, int index, float* value, char* valueString);  // 0x280130
    static FMOD_RESULT _GetParamInt(
        FMOD_DSP_STATE* state, int index, int* value, char* valueString);    // 0x2801C0
    // Skips idle input and a fully dry mix. At 0x2802C0.
    static FMOD_RESULT _ShouldIProcess(
        FMOD_DSP_STATE* state,
        int inputsIdle,
        unsigned int length,
        unsigned int inMask,
        int inChannels,
        FMOD_SPEAKERMODE speakerMode);

    SmbPitchShiftPlugin();  // 0x2802E0, inlined into _Create.
    virtual ~SmbPitchShiftPlugin();  // slots 0-1: 0x280330, 0x280340

    // Clamps the window to 128-4096 samples and the overlap to 1-16, then
    // sets the shifter up again, which clears it. At 0x280350, also inlined
    // into the callbacks.
    void _ApplySettings();
    // Accessors with no caller in this build; names are guesses.
    SmbPitchShift* GetPitchShift() const;  // 0x2803C0
    const AudioData* GetAudioData() const;  // 0x2803D0

    static FMOD_DSP_DESCRIPTION mDspDescription;              // 0x19B5018
    static FMOD_DSP_PARAMETER_DESC* mParams[kNumParams];      // 0x19B50F0

    SmbPitchShift* mPitchShift;  // Created by _Create; not freed by the destructor.
    int mWindowSize;
    int mOverlap;
    float mPitchRatio;  // 2^(semitones / 12), limited to 0.5-2 when read.
    float mMonoCutoff;
    float mWetDry;
    float mSampleRate;
    FmodPluginUserData mUserData;  // Set as the DSP's user data.
};

static_assert(offsetof(SmbPitchShiftPlugin, mPitchShift) == 0x8);
static_assert(offsetof(SmbPitchShiftPlugin, mPitchRatio) == 0x18);
static_assert(offsetof(SmbPitchShiftPlugin, mSampleRate) == 0x24);
static_assert(offsetof(SmbPitchShiftPlugin, mUserData) == 0x28);
static_assert(sizeof(SmbPitchShiftPlugin) == 0x48);
