#include "render/queries/RndShaderDisplayOcclusionQueryCoverage.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"

// Reconstructed from eboot.elf at 0x5F8D90.
RndShaderDisplayOcclusionQueryCoverage::RndShaderDisplayOcclusionQueryCoverage()
    : mCoverage(-1) {}

// Reconstructed from eboot.elf at 0x5F8DC0.
RndShaderDisplayOcclusionQueryCoverage::~RndShaderDisplayOcclusionQueryCoverage() {}

// Reconstructed from eboot.elf at 0x5F8EF0.
const char* RndShaderDisplayOcclusionQueryCoverage::_GetClassNameImpl() const {
    return "RndShaderDisplayOcclusionQueryCoverage";
}

// Reconstructed from eboot.elf at 0x5F8EA0.
const char* RndShaderDisplayOcclusionQueryCoverage::_GetShaderFilePath() const {
    return "../../system/data/shaders/DisplayOcclusionQueryCoverage.hlsl";
}

// Reconstructed from eboot.elf at 0x5F8EB0.
void RndShaderDisplayOcclusionQueryCoverage::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig&,
    RndShaderResourceConfig& resources) {
    mCoverage = resources.AddComputeBuffer(
        "gCoverage", 0, kShaderNumericUInt, kShaderProgramPixel);
}
