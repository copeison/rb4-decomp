#include "render/shaders/RndShaderEnums.h"

#include <cstring>

namespace {

// Names of the user shading modes. Name not in the reference map.
const char* const kUserShadingModeNames[kNumUserShadingModes] = {
    "Lit",
    "Fast + Cheap",
    "Unlit",
    "Overdraw",
    "Batches",
    "Batch Size",
    "Lighting Only",
    "Lit (Diffuse)",
    "Lit (Specular)",
    "Lit (Direct)",
    "Lit (Direct Diffuse)",
    "Lit (Direct Specular)",
    "Lit (Indirect)",
    "Lit (Indirect Diffuse)",
    "Lit (Indirect Specular)",
    "No Negative Lights",
    "Lighting Overdraw",
    "Light Probe Overdraw",
    "Vertex Color",
    "Vertex Alpha",
    "Vertex Normal",
    "Vertex Tangent",
    "Vertex Bitangent",
    "Pixel Normal",
    "UV0",
    "UV1",
    "Lighting Path",
    "Material Color",
    "Material Alpha",
    "Material Smoothness",
    "Material Metallicity",
    "Material Emissive",
};

// Names and descriptions of the falloff functions (0x192F9C0, 0x192F9E0).
// Names not in the reference map.
const char* const kFalloffFunctionNames[kNumFalloffFunctions] = {
    "Linear Falloff",
    "Inverse-Square Falloff",
    "Exponential Falloff",
    "Sigmoid Falloff",
};
const char* const kFalloffFunctionDescriptions[kNumFalloffFunctions] = {
    "Linear falloff function",
    "Inverse-square falloff approximation which goes to zero",
    "Inverse-exponential falloff approximation which goes to zero",
    "Sigmoid falloff function",
};

}  // namespace

// Reconstructed from eboot.elf at 0x645E40.
const char* ToString(RndUserShadingMode mode) {
    return static_cast<unsigned int>(mode) < kNumUserShadingModes
        ? kUserShadingModeNames[mode]
        : nullptr;
}

// Reconstructed from eboot.elf at 0x645E80.
RndUserShadingMode UserShadingModeFromString(const char* name) {
    if (name == nullptr) {
        return static_cast<RndUserShadingMode>(-1);
    }
    for (unsigned int mode = 0; mode < kNumUserShadingModes; ++mode) {
        if (std::strcmp(kUserShadingModeNames[mode], name) == 0) {
            return static_cast<RndUserShadingMode>(mode);
        }
    }
    return static_cast<RndUserShadingMode>(-1);
}

// Reconstructed from eboot.elf at 0x645EE0.
const char* ToString(RndFalloffFunction function) {
    return static_cast<unsigned int>(function) < static_cast<unsigned int>(kNumFalloffFunctions)
        ? kFalloffFunctionNames[function]
        : "";
}

// Reconstructed from eboot.elf at 0x645F00.
const char* ToDescription(RndFalloffFunction function) {
    return static_cast<unsigned int>(function) < static_cast<unsigned int>(kNumFalloffFunctions)
        ? kFalloffFunctionDescriptions[function]
        : "";
}
