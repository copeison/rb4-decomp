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
