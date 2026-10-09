#include "render/debug/RndShaderRenderTestSimple.h"

#include <cstring>

#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"

// Reconstructed from eboot.elf at 0x642500.
RndShaderRenderTestSimple::RndShaderRenderTestSimple()
    : mUseVertexColor{}, mUseCBufferColor{}, mColor(-1), mCBufferSize(0) {}

// Reconstructed from eboot.elf at 0x642550.
RndShaderRenderTestSimple::~RndShaderRenderTestSimple() {}

// Reconstructed from eboot.elf at 0x6427A0.
const char* RndShaderRenderTestSimple::_GetClassNameImpl() const {
    return "RndShaderRenderTestSimple";
}

// Reconstructed from eboot.elf at 0x6426B0.
const char* RndShaderRenderTestSimple::_GetShaderFilePath() const {
    return "../../system/data/shaders/RenderTestSimple.hlsl";
}

// Reconstructed from eboot.elf at 0x6426C0. The vertex-color permutation is
// registered with the global defines and the constant-buffer color
// permutation with the pixel defines.
void RndShaderRenderTestSimple::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig&) {
    mUseVertexColor =
        defines.GetGlobalDefines().AddBool(Symbol("HX_USE_VERTEX_COLOR"));
    mUseCBufferColor = defines.GetDefines(kShaderProgramPixel)
                           .AddBool(Symbol("HX_USE_CBUFFER_COLOR"));
    mColor = cbuffer.AddConstant(kShaderNumericFloat4, "gColor");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x642580. Name not in the reference map.
// The color constant is uploaded only for the constant-buffer-color
// permutation. Vertex color is a global permutation and is written into every
// program key.
void RndShaderRenderTestSimple::Select(
    RndContext& context,
    const Params& params) {
    constexpr unsigned long kPixelKey = 3;
    if (params.mUseCBufferColor) {
        auto& buffer =
            RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
        std::memcpy(
            RndShaderDrawUtl::GetCBufferMember(buffer, mColor),
            params.mColor,
            sizeof(params.mColor));
        RndShaderDrawUtl::CommitCBuffer(buffer, context, mCBufferSize);
    }

    const auto globalField = static_cast<RndShaderKey>(
        ((params.mUseVertexColor ? 1U : 0U) -
         static_cast<unsigned int>(mUseVertexColor.mFirst))
        << mUseVertexColor.mShift) << 32;
    RndShaderKeyGroup keys;
    for (auto& key : keys.mKeys) {
        key = globalField;
    }
    keys.mKeys[kPixelKey] = mUseCBufferColor.SetValue(
        globalField, params.mUseCBufferColor ? 1U : 0U);
    _SelectShaderCollection(context, keys);
}
