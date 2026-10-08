#include "render/core/debug/render_debug_mode.h"

#include <array>
#include <cctype>
#include <cstring>

namespace rb4 {

namespace {

constexpr std::array<const char*, 32> kDrawModeNames = {{
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
}};

constexpr std::array<const char*, 74> kDebugViewNames = {{
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
}};

bool equals_ignore_ascii_case(const char* left, const char* right) {
    while (*left != '\0' && *right != '\0') {
        const auto left_char = static_cast<unsigned char>(*left++);
        const auto right_char = static_cast<unsigned char>(*right++);
        if (std::tolower(left_char) != std::tolower(right_char)) {
            return false;
        }
    }
    return *left == *right;
}

}  // namespace

// Reconstructed from eboot.elf at 0x645E40.
const char* render_draw_mode_name(std::uint32_t mode) {
    return mode < kDrawModeNames.size() ? kDrawModeNames[mode] : nullptr;
}

// Reconstructed from eboot.elf at 0x645E80.
std::uint32_t render_draw_mode_from_name(const char* name) {
    if (name == nullptr) {
        return kInvalidRenderDebugMode;
    }
    for (std::uint32_t index = 0; index < kDrawModeNames.size(); ++index) {
        if (std::strcmp(kDrawModeNames[index], name) == 0) {
            return index;
        }
    }
    return kInvalidRenderDebugMode;
}

// Reconstructed from eboot.elf at 0x6B4EE0.
const char* render_debug_view_name(std::uint32_t view) {
    return view < kDebugViewNames.size() ? kDebugViewNames[view] : nullptr;
}

// Reconstructed from eboot.elf at 0x6B5510.
std::uint32_t render_debug_view_from_name(const char* name) {
    if (name == nullptr) {
        return kInvalidRenderDebugMode;
    }
    for (std::uint32_t index = 0; index < kDebugViewNames.size(); ++index) {
        if (equals_ignore_ascii_case(name, kDebugViewNames[index])) {
            return index;
        }
    }
    return kInvalidRenderDebugMode;
}

}  // namespace rb4
