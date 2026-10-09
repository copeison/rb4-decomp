#pragma once

#include <cstddef>

// The amplifier models an AmpSimulator drives: convolution-based guitar amp
// simulations, one class per "model_type". They were added after the
// reference map's build, so every name here is inferred, and they have no
// type information. Only the interface AmpSimulator uses is declared: each
// model's constructor (2.8 to 3.2 KiB) and Process (over 2 KiB) are filter
// and convolver code (0x1090BE0 to 0x1099000, with an FDConvolver at
// 0x1099BE0) and are not reconstructed. The base's inline destructor leaves
// no code.
class AmpModel {
public:
    virtual ~AmpModel() {}
    // Renders the first channel. Sets mClipped when an output sample's
    // square exceeds the model's threshold.
    virtual void Process(float** input, float** output, int numFrames) = 0;
    // Whether the model has a control for an AmpSimulator::Parameter.
    virtual bool IsParameterUsed(int parameter) = 0;
    // A value in [0, 1], or 0 or 1 for the channel switch.
    virtual void SetParameter(int parameter, float value) = 0;

    // Field names are not in the reference map.
    unsigned char mOpaque8[32];  // The base's convolver state.
    bool mClipped;               // Whether the last block clipped.
};

static_assert(offsetof(AmpModel, mClipped) == 40);
static_assert(sizeof(AmpModel) == 48);

// "jcm800". The vtable is at 0x199F430. The object is 976 bytes.
class AmpModelJCM800 : public AmpModel {
public:
    AmpModelJCM800();            // 0x1091050
    ~AmpModelJCM800() override;  // Slots 0-1 at 0x1090F80 and 0x1090FE0.
    void Process(float** input, float** output, int numFrames) override;  // slot 2: 0x1092210
    bool IsParameterUsed(int parameter) override;                         // slot 3: 0x1091040
    void SetParameter(int parameter, float value) override;               // slot 4: 0x1091B90

    unsigned char mOpaque48[0x3D0 - 0x30];  // Not modelled.
};

static_assert(sizeof(AmpModelJCM800) == 0x3D0);

// "ac30tb". The vtable is at 0x199F470. The object is 1,128 bytes.
class AmpModelAC30TB : public AmpModel {
public:
    AmpModelAC30TB();            // 0x1092B10
    ~AmpModelAC30TB() override;  // Slots 0-1 at 0x1093740 and 0x10937A0.
    void Process(float** input, float** output, int numFrames) override;  // slot 2: 0x1093CD0
    bool IsParameterUsed(int parameter) override;                         // slot 3: 0x1092B00
    void SetParameter(int parameter, float value) override;               // slot 4: 0x1093800

    unsigned char mOpaque48[0x468 - 0x30];  // Not modelled.
};

static_assert(sizeof(AmpModelAC30TB) == 0x468);

// "ad30". The vtable is at 0x199F4B0. The object is 1,056 bytes.
class AmpModelAD30 : public AmpModel {
public:
    AmpModelAD30();            // 0x1094520
    ~AmpModelAD30() override;  // Slots 0-1 at 0x1095190 and 0x10951F0.
    void Process(float** input, float** output, int numFrames) override;  // slot 2: 0x1095680
    bool IsParameterUsed(int parameter) override;                         // slot 3: 0x1094510
    void SetParameter(int parameter, float value) override;               // slot 4: 0x1095250

    unsigned char mOpaque48[0x420 - 0x30];  // Not modelled.
};

static_assert(sizeof(AmpModelAD30) == 0x420);

// "engl". The vtable is at 0x199F4F0. The object is 1,120 bytes.
class AmpModelENGL : public AmpModel {
public:
    AmpModelENGL();            // 0x1095F10
    ~AmpModelENGL() override;  // Slots 0-1 at 0x1096BB0 and 0x1096C10.
    void Process(float** input, float** output, int numFrames) override;  // slot 2: 0x1097240
    bool IsParameterUsed(int parameter) override;                         // slot 3: 0x1095F00
    void SetParameter(int parameter, float value) override;               // slot 4: 0x1096C70

    unsigned char mOpaque48[0x460 - 0x30];  // Not modelled.
};

static_assert(sizeof(AmpModelENGL) == 0x460);

// "twin". The vtable is at 0x199F530. The object is 1,128 bytes.
class AmpModelTwin : public AmpModel {
public:
    AmpModelTwin();            // 0x1097B40
    ~AmpModelTwin() override;  // Slots 0-1 at 0x1098750 and 0x10987B0.
    void Process(float** input, float** output, int numFrames) override;  // slot 2: 0x1098C50
    bool IsParameterUsed(int parameter) override;                         // slot 3: 0x1097B20
    void SetParameter(int parameter, float value) override;               // slot 4: 0x1098810

    unsigned char mOpaque48[0x468 - 0x30];  // Not modelled.
};

static_assert(sizeof(AmpModelTwin) == 0x468);

// Guitar amplifier simulation of a Fusion patch's "amp_simulation" settings
// (0x1090650 to 0x1090BCA). It forwards its parameters to one of five
// AmpModels. The code was added after the reference map's build; the class
// name, its member names and the object file name are inferred. The object
// is 56 bytes.
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

    // The "model_type"s, named by ModelToString. kNumModels selects no
    // model. Names not in the reference map.
    enum Model : int {
        kModelJCM800 = 0,
        kModelAC30TB = 1,
        kModelAD30 = 2,
        kModelENGL = 3,
        kModelTwin = 4,
        kNumModels = 5,
    };

    // The blocks the clip flag stays set after a clipped block, about a
    // second at 44.1 kHz in 128-frame blocks. Name not in the reference map.
    static constexpr int kClipHoldBlocks = 344;

    AmpSimulator();   // 0x1090650
    ~AmpSimulator();  // 0x10906B0
    // Copies the first channel when no model is set. At 0x10906C0.
    void Process(float** input, float** output, int numFrames);
    // "unknown" for a value past the table. At 0x1090720.
    static const char* ModelToString(int model);
    // kModelJCM800 for an unknown name. At 0x1090740.
    static int StringToModel(const char* name);
    void SetParameter(int parameter, float value);  // 0x10907D0
    // 0 without a model. Not called. At 0x1090880.
    float GetParameter(int parameter) const;
    // Asks the model; false without one. Not called. At 0x10908B0.
    bool IsParameterUsed(int parameter) const;
    // The models' IsParameterUsed by model index, inlined. At 0x10908D0.
    static bool IsParameterUsedByModel(int model, int parameter);
    // Replaces the model with one of the five "model_type"s, or none. At
    // 0x1090940.
    void SetModel(int model);

    // Field names are not in the reference map.
    AmpModel* mModel;  // Null for no model.
    float mParameters[kNumParameters];  // 0 to 10; the switch 0 or 1.
    unsigned char mOpaque40[8];  // A flag and an int only the constructor writes.
    int mClipHoldCounter;  // Blocks left in the clip indication.
};

static_assert(offsetof(AmpSimulator, mParameters) == 8);
static_assert(offsetof(AmpSimulator, mOpaque40) == 40);
static_assert(offsetof(AmpSimulator, mClipHoldCounter) == 48);
static_assert(sizeof(AmpSimulator) == 56);
