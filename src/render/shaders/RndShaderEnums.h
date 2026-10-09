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
