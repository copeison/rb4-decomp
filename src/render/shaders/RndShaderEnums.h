#pragma once

// Shader program stages, in the order of ToDefine's name table at 0x63E5D0.
// Enumerator names are not in the reference map.
enum RndShaderProgramType : unsigned int {
    kShaderProgramVertex = 0,
    kShaderProgramHull = 1,
    kShaderProgramDomain = 2,
    kShaderProgramGeometry = 3,
    kShaderProgramPixel = 4,
    kShaderProgramCompute = 5,
};

// Name not in the reference map.
constexpr unsigned int kNumShaderProgramTypes = 6;

// Geometry kind selected by the HX_GEO_TYPE define. Enumerator names are not
// in the reference map.
enum RndShaderGeoType : int {
    kShaderGeoTypeDefault = 0,
};

// Shading modes, the values of HX_SHADING_MODE. The name is the map's; the
// enumerators follow the HX_SHADING_MODE_* defines.
enum RndShadingMode : int {
    kShadingModeStandard = 0,
    kShadingModeStandardFog = 1,
    kShadingModeStandardVScat = 2,
    kShadingModeDepthOnly = 3,
    kShadingModeSolidColor = 4,
    kShadingModeDeferredNormalsAndZFill = 5,
    kShadingModeDeferredUnlit = 6,
    kShadingModeDeferredUnlitAndZFill = 7,
    kShadingModeDeferredLit = 8,
    kShadingModeDeferredLitAndZFill = 9,
    kShadingModeDeferredLitEmissive = 10,
    kShadingModeDeferredLitEmissiveAndZFill = 11,
    kShadingModeDeferredDecalTransparent = 12,
    kShadingModeFwdLitOpaque = 13,
    kShadingModeSceneMask = 14,
    kShadingModeImpostorMaps = 15,
    kShadingModeFastCheap = 16,
    kShadingModeWireframe = 17,
    kShadingModeDebugMisc = 18,
};

// User-facing shading modes, in the order of ToString's name table; the first
// is "Lit". Enumerator names are not in the reference map.
enum RndUserShadingMode : unsigned int {
    kUserShadingModeLit = 0,
};

// Name not in the reference map.
constexpr unsigned int kNumUserShadingModes = 32;

const char* ToString(RndUserShadingMode mode);  // 0x645E40

// Returns the mode whose name matches exactly, or -1. Name not in the
// reference map.
RndUserShadingMode UserShadingModeFromString(const char* name);  // 0x645E80
