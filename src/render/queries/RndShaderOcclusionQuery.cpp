#include "render/queries/RndShaderOcclusionQuery.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"

// Reconstructed from eboot.elf at 0x5F8F20.
RndShaderOcclusionQuery::RndShaderOcclusionQuery()
    : mCoverageId(-1), mCBufferSize(0), mCoverage(-1) {}

// Reconstructed from eboot.elf at 0x5F8F90.
RndShaderOcclusionQuery::~RndShaderOcclusionQuery() {}

// Reconstructed from eboot.elf at 0x5F90F0.
const char* RndShaderOcclusionQuery::_GetClassNameImpl() const {
    return "RndShaderOcclusionQuery";
}

// Reconstructed from eboot.elf at 0x5F9070.
const char* RndShaderOcclusionQuery::_GetShaderFilePath() const {
    return "../../system/data/shaders/OcclusionQuery.hlsl";
}

// Reconstructed from eboot.elf at 0x5F9080. The coverage id is declared as
// a float constant.
void RndShaderOcclusionQuery::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mCoverageId = cbuffer.AddConstant(kShaderNumericFloat, "gCoverageId");
    mCBufferSize = cbuffer.mSize;
    mCoverage = resources.AddComputeBufferWritable(
        "gCoverage", 0, kShaderNumericUInt, kShaderProgramPixel);
}
