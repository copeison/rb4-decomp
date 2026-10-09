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
