#pragma once

#include <cstddef>

// Guitar amplifier model of a Fusion patch's "amp_simulation" settings. The
// code (0x1090650 to 0x10909CB) was added after the reference map's build
// and is not reconstructed; the class name and the member names are not in
// the reference map. It drives one of five model objects through their
// vtables. The object is 56 bytes.
class AmpSimulator {
public:
    // The "amp_simulation" parameters, in SetParameter's order. Names not in
    // the reference map.
    enum Parameter : int {
        kParamPresence = 0,
        kParamBass = 1,
        kParamMiddle = 2,
        kParamTreble = 3,
        kParamMaster = 4,
        kParamPreamp = 5,
        kParamChannelSwitch = 6,  // Stored as on or off.
        kParamOutputGain = 7,
    };
    static constexpr int kNumParameters = 8;

    AmpSimulator();   // 0x1090650
    ~AmpSimulator();  // 0x10906B0
    // Copies the input when no model is set. At 0x10906C0.
    void Process(float** input, float** output, int numFrames);
    void SetParameter(int parameter, float value);  // 0x10907D0
    // Replaces the model with one of the five "model_type"s. At 0x1090940.
    void SetModel(int model);

    // Field names are not in the reference map.
    void* mModel;                // The model object, or null.
    float mParameters[kNumParameters];
    unsigned char mOpaque40[16];  // Not modelled.
};

static_assert(sizeof(AmpSimulator) == 56);
