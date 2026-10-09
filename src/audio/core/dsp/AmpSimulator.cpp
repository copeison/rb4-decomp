#include "audio/core/dsp/AmpSimulator.h"

#include <cstring>

namespace {

// The "model_type" names, at 0x199F3F0. Name not in the reference map.
const char* sModelNames[AmpSimulator::kNumModels] = {"jcm800", "ac30tb", "ad30", "engl", "twin"};

}  // namespace

// Reconstructed from eboot.elf at 0x1090650.
AmpSimulator::AmpSimulator()
    : mModel(nullptr),
      mParameters{6.0f, 7.0f, 4.0f, 7.0f, 6.0f, 6.0f, 0.0f, 10.0f},
      mOpaque40(),
      mClipHoldCounter(0) {}

// Reconstructed from eboot.elf at 0x10906B0.
AmpSimulator::~AmpSimulator() {
    delete mModel;
}

// Reconstructed from eboot.elf at 0x10906C0. A clipped block restarts the
// clip indication; a clean one counts it down.
void AmpSimulator::Process(float** input, float** output, int numFrames) {
    if (mModel != nullptr) {
        mModel->Process(input, output, numFrames);
        if (mModel->mClipped) {
            mClipHoldCounter = kClipHoldBlocks;
        } else if (mClipHoldCounter > 0) {
            --mClipHoldCounter;
        }
    } else {
        std::memcpy(output[0], input[0], sizeof(float) * numFrames);
        mClipHoldCounter = 0;
    }
}

// Reconstructed from eboot.elf at 0x1090720.
const char* AmpSimulator::ModelToString(int model) {
    if (static_cast<unsigned int>(model) > kModelTwin) {
        return "unknown";
    }
    return sModelNames[model];
}

// Reconstructed from eboot.elf at 0x1090740.
int AmpSimulator::StringToModel(const char* name) {
    if (std::strcmp(name, "jcm800") == 0) {
        return kModelJCM800;
    }
    if (std::strcmp(name, "ac30tb") == 0) {
        return kModelAC30TB;
    }
    if (std::strcmp(name, "ad30") == 0) {
        return kModelAD30;
    }
    if (std::strcmp(name, "engl") == 0) {
        return kModelENGL;
    }
    if (std::strcmp(name, "twin") == 0) {
        return kModelTwin;
    }
    return kModelJCM800;
}

// Reconstructed from eboot.elf at 0x10907D0. The model receives a tenth of
// the value; the channel switch is stored and passed as 0 or 1. An unknown
// parameter still reaches the model.
void AmpSimulator::SetParameter(int parameter, float value) {
    switch (parameter) {
    case kParamPresence:
        mParameters[kParamPresence] = value;
        break;
    case kParamBass:
        mParameters[kParamBass] = value;
        break;
    case kParamMiddle:
        mParameters[kParamMiddle] = value;
        break;
    case kParamTreble:
        mParameters[kParamTreble] = value;
        break;
    case kParamMaster:
        mParameters[kParamMaster] = value;
        break;
    case kParamPreamp:
        mParameters[kParamPreamp] = value;
        break;
    case kParamChannelSwitch:
        value = value > 0.5f ? 1.0f : 0.0f;
        mParameters[kParamChannelSwitch] = value;
        break;
    case kParamOutputGain:
        mParameters[kParamOutputGain] = value;
        break;
    default:
        break;
    }
    if (mModel != nullptr) {
        if (parameter == kParamChannelSwitch) {
            mModel->SetParameter(kParamChannelSwitch, value);
        } else {
            mModel->SetParameter(parameter, value * 0.1f);
        }
    }
}

// Reconstructed from eboot.elf at 0x1090880. It scales the stored value by
// 10 rather than undoing SetParameter's tenth.
float AmpSimulator::GetParameter(int parameter) const {
    float value = 0.0f;
    if (mModel != nullptr) {
        value = mParameters[parameter];
        if (parameter != kParamChannelSwitch) {
            value *= 10.0f;
        }
    }
    return value;
}

// Reconstructed from eboot.elf at 0x10908B0.
bool AmpSimulator::IsParameterUsed(int parameter) const {
    if (mModel == nullptr) {
        return false;
    }
    return mModel->IsParameterUsed(parameter);
}

// Reconstructed from eboot.elf at 0x10908D0. The cases are the models' slot
// 3 implementations.
bool AmpSimulator::IsParameterUsedByModel(int model, int parameter) {
    switch (model) {
    case kModelJCM800:
        return true;
    case kModelAC30TB:
        return parameter != kParamMiddle && parameter != kParamChannelSwitch;
    case kModelAD30:
        return parameter != kParamPresence && parameter != kParamChannelSwitch;
    case kModelENGL:
        return parameter != kParamChannelSwitch;
    case kModelTwin:
        return parameter != kParamPresence && parameter != kParamMaster && parameter != kParamChannelSwitch;
    default:
        return false;
    }
}

// Reconstructed from eboot.elf at 0x1090940. After a model is created every
// parameter is sent mParameters[0], the presence, rather than its own value.
void AmpSimulator::SetModel(int model) {
    if (mModel != nullptr) {
        delete mModel;
        mModel = nullptr;
    }
    switch (model) {
    case kModelJCM800:
        mModel = new AmpModelJCM800();
        break;
    case kModelAC30TB:
        mModel = new AmpModelAC30TB();
        break;
    case kModelAD30:
        mModel = new AmpModelAD30();
        break;
    case kModelENGL:
        mModel = new AmpModelENGL();
        break;
    case kModelTwin:
        mModel = new AmpModelTwin();
        break;
    default:
        break;
    }
    if (mModel != nullptr) {
        for (int i = 0; i < kNumParameters; ++i) {
            SetParameter(i, mParameters[0]);
        }
    }
}
