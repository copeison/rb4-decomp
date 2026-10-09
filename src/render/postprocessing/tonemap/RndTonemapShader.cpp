#include "render/postprocessing/tonemap/RndTonemapShader.h"

// Reconstructed from eboot.elf at 0x4ADB80.
RndTonemapShader::RndTonemapShader() {}

RndTonemapShader::~RndTonemapShader() {}

// Reconstructed from eboot.elf at 0x4ADBB0.
void RndTonemapShader::Select(RndContext& context, const Params& params) {
    _Select(context, params);
}

const char* RndTonemapShader::_GetShaderFilePath() const {
    return "../../system/data/shaders/TonemapPS.hlsl";
}

const char* RndTonemapShader::_GetClassNameImpl() const {
    return "RndTonemapShader";
}

int RndTonemapShader::_GetShaderStages() const {
    return kShaderStagesVertexPixel;
}
