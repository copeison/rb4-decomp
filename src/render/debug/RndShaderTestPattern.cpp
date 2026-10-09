#include "render/debug/RndShaderTestPattern.h"

#include <cstring>

#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"

// Reconstructed from eboot.elf at 0x6452E0.
RndShaderTestPattern::RndShaderTestPattern()
    : mColor0(-1), mColor1(-1), mNumTiles(-1), mCBufferSize(0) {}

// Reconstructed from eboot.elf at 0x6453F0.
RndShaderTestPattern::~RndShaderTestPattern() {}

// Reconstructed from eboot.elf at 0x6455B0.
const char* RndShaderTestPattern::_GetClassNameImpl() const {
    return "RndShaderTestPattern";
}

// Reconstructed from eboot.elf at 0x645530.
const char* RndShaderTestPattern::_GetShaderFilePath() const {
    return "../../system/data/shaders/TestPattern.hlsl";
}

// Reconstructed from eboot.elf at 0x645540.
void RndShaderTestPattern::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig&) {
    mColor0 = cbuffer.AddConstant(kShaderNumericFloat4, "gColor0");
    mColor1 = cbuffer.AddConstant(kShaderNumericFloat4, "gColor1");
    mNumTiles = cbuffer.AddConstant(kShaderNumericFloat2, "gNumTiles");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x645420. Name not in the reference map.
void RndShaderTestPattern::Select(RndContext& context, const Params& params) {
    auto& buffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    std::memcpy(
        RndShaderDrawUtl::GetCBufferMember(buffer, mColor0),
        params.mColor0,
        sizeof(params.mColor0));
    std::memcpy(
        RndShaderDrawUtl::GetCBufferMember(buffer, mColor1),
        params.mColor1,
        sizeof(params.mColor1));
    std::memcpy(
        RndShaderDrawUtl::GetCBufferMember(buffer, mNumTiles),
        params.mNumTiles,
        sizeof(params.mNumTiles));
    RndShaderDrawUtl::CommitCBuffer(buffer, context, mCBufferSize);
    RndShaderKeyGroup keys{};
    _SelectShaderCollection(context, keys);
}
