#include "render/debug/RndBufferInspection.h"

#include <cctype>

#include "render/debug/RndBufferInspectionShader.h"

namespace {

// Names of the buffer inspection modes. Name not in the reference map.
const char* const kModeNames[RndBufferInspection::kNumModes] = {
    "None",
    "Thumbnails (Common)",
    "Thumbnails (GBuffer)",
    "Thumbnails (Lighting)",
    "Thumbnails (Atmosphere)",
    "Thumbnails (Shadows)",
    "Thumbnails (PostProc)",
    "Depth Buffer",
    "Depth Buffer (Near 10%)",
    "Depth Buffer (Near 1%)",
    "Linear Depth",
    "Tiled Min Depth",
    "Tiled Max Depth",
    "Stencil Buffer (Raw)",
    "Stencil Buffer (EnvironIndex)",
    "Stencil Buffer (NoAtmosphere)",
    "Stencil Buffer (ReceiveDecals)",
    "Stencil Buffer (SceneMask)",
    "Stencil Buffer (ShadowIndex)",
    "Scene Mask (Unrefined)",
    "Scene Mask (Refined)",
    "Light Accum Buffer 0",
    "Light Accum Buffer 1",
    "GBuffer Base Color",
    "GBuffer Pixel Normal",
    "GBuffer Vertex Normal",
    "GBuffer Smoothness",
    "GBuffer Metallicity",
    "GBuffer 1 (Raw)",
    "GBuffer 1 Alpha Channel (Raw)",
    "GBuffer 2 (Raw)",
    "GBuffer 2 Alpha Channel (Raw)",
    "Light Probe Accumulation Buffer",
    "Light Probe Accumulation Weights",
    "Tiled Lights Overdraw",
    "Tiled Light Probes Overdraw",
    "Depth-Tiled Lights Overdraw",
    "Depth-Tiled Light Probes Overdraw",
    "Tiled Lights Overdraw (Both Eyes)",
    "Tiled Light Probes Overdraw (Both Eyes)",
    "Depth-Tiled Lights Overdraw (Both Eyes)",
    "Depth-Tiled Light Probes Overdraw (Both Eyes)",
    "Tiled Lights Interpolation (Diffuse)",
    "Tiled Lights Interpolation (Specular)",
    "Tiled Lights Interpolation (Similarity)",
    "Sky (Full Res)",
    "Sky (1/2 Res)",
    "Sky (1/4 Res)",
    "Sky (1/8 Res)",
    "Shadow Contribution 0",
    "Shadow Contribution 1",
    "Shadow Contribution 2",
    "Spot Shadow Depth 0",
    "Spot Shadow Depth 0 (Near 10%)",
    "Spot Shadow Depth 0 (Near 1%)",
    "Spot Shadow Depth 1",
    "Spot Shadow Depth 1 (Near 10%)",
    "Spot Shadow Depth 1 (Near 1%)",
    "Spot Shadow Depth 2",
    "Spot Shadow Depth 2 (Near 10%)",
    "Spot Shadow Depth 2 (Near 1%)",
    "Shadow Contrib Scratch 0",
    "Shadow Contrib Scratch 1",
    "Shadow Soften Tiles 0",
    "Shadow Soften Tiles 1",
    "Half-Size 0",
    "Half-Size 1",
    "Qtr-Size 0",
    "Qtr-Size 1",
    "Mask Buffer",
    "Mask Scratch Buffer",
    "Mask Tile Buffer",
    "Screen Space Ambient Occlusion",
    "Function Table",
};

// The inspection shader. Name not in the reference map.
RndBufferInspectionShader* gShader = nullptr;  // 0x1AAF8D0

// Name not in the reference map.
bool EqualsIgnoreCase(const char* left, const char* right) {
    while (*left != '\0' && *right != '\0') {
        const auto leftChar = static_cast<unsigned char>(*left++);
        const auto rightChar = static_cast<unsigned char>(*right++);
        if (std::tolower(leftChar) != std::tolower(rightChar)) {
            return false;
        }
    }
    return *left == *right;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B4EE0.
const char* RndBufferInspection::ToString(RndBufferInspectionMode mode) {
    return static_cast<unsigned int>(mode) < kNumModes ? kModeNames[mode]
                                                       : nullptr;
}

// Reconstructed from eboot.elf at 0x6B5510.
RndBufferInspectionMode RndBufferInspection::FromString(const char* name) {
    if (name == nullptr) {
        return static_cast<RndBufferInspectionMode>(-1);
    }
    for (unsigned int mode = 0; mode < kNumModes; ++mode) {
        if (EqualsIgnoreCase(name, kModeNames[mode])) {
            return static_cast<RndBufferInspectionMode>(mode);
        }
    }
    return static_cast<RndBufferInspectionMode>(-1);
}

// Reconstructed from eboot.elf at 0x6B54A0.
void RndBufferInspection::Init() {
    auto* shader = new RndBufferInspectionShader;
    shader->_Register();
    gShader = shader;
}

// Reconstructed from eboot.elf at 0x6B54E0.
void RndBufferInspection::Terminate() {
    delete gShader;
    gShader = nullptr;
}
