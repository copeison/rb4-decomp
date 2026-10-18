#include "render/debug/RndShaderDisplayTextureCube.h"

#include "render/context/RndContext.h"
#include "render/defaults/RndDefaults.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"
#include "render/textures/RndTextureCube.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x63E5A0 and never read. Names not in the reference map.
[[maybe_unused]] int gTextureCubeGroupSizeUnused = -1;  // 0x1AAB77C
[[maybe_unused]] int gTextureCubeGroupSize2D = 8;       // 0x1AAB780
[[maybe_unused]] int gTextureCubeGroupSize3D = 4;       // 0x1AAB784

// Names not in the reference map.
constexpr unsigned long kPixelKey = 3;
constexpr unsigned long kNoArrayIndex = static_cast<unsigned long>(-1);

}  // namespace

// Reconstructed from eboot.elf at 0x63DFD0.
RndShaderDisplayTextureCube::RndShaderDisplayTextureCube()
    : mUseMipLevel{},
      mBlendTextures{},
      mUseTexArray{},
      mColor(-1),
      mMipLevel(-1),
      mBlendAmount(-1),
      mArrayIndices(-1),
      mCBufferSize(0),
      mTexture0(-1),
      mTexture1(-1),
      mTexArray(-1) {}

// Reconstructed from eboot.elf at 0x63E050.
RndShaderDisplayTextureCube::~RndShaderDisplayTextureCube() {}

// Reconstructed from eboot.elf at 0x63E080.
void RndShaderDisplayTextureCube::Select(RndContext& context, Params& params) {
    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    const auto member = [&cbuffer](unsigned long offset) {
        return static_cast<float*>(
            RndShaderDrawUtl::GetCBufferMember(cbuffer, offset));
    };
    *reinterpret_cast<Hmx::Color*>(member(mColor)) = params.mColor;
    *member(mMipLevel) = params.mMipLevel;
    *member(mBlendAmount) = params.mBlendAmount;
    float* const indices = member(mArrayIndices);
    indices[0] = static_cast<float>(params.mArrayIndex0);
    indices[1] = static_cast<float>(params.mArrayIndex1);
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    RndShaderKeyGroup keys{};
    RndShaderKey key = mUseMipLevel.SetValue(0, params.mUseMipLevel);
    key = mUseTexArray.SetValue(key, params.mTexArray != nullptr);
    bool blend = false;
    if (params.mTexArray != nullptr) {
        blend = params.mArrayIndex0 != kNoArrayIndex
            && params.mArrayIndex1 != kNoArrayIndex;
    } else if (params.mTexture0 != nullptr) {
        blend = params.mTexture1 != nullptr;
    }
    keys.mKeys[kPixelKey] = mBlendTextures.SetValue(key, blend);
    _SelectShaderCollection(context, keys);

    if (params.mTexArray != nullptr) {
        params.mTexArray->Select(context, kShaderProgramPixel, mTexArray, 0, 0);
    } else if (params.mTexture0 == nullptr) {
        TheRndDevice()
            ->mDefaults.mTexturesCube[kDefaultTextureError]
            ->Select(context, kShaderProgramPixel, mTexture0, 0, 0);
    } else {
        params.mTexture0->Select(context, kShaderProgramPixel, mTexture0, 0, 0);
        if (params.mTexture1 != nullptr) {
            params.mTexture1->Select(
                context, kShaderProgramPixel, mTexture1, 0, 0);
        }
    }
}

// Reconstructed from eboot.elf at 0x63E370.
const char* RndShaderDisplayTextureCube::_GetShaderFilePath() const {
    return "../../system/data/shaders/DisplayTextureCube.hlsl";
}

// Reconstructed from eboot.elf at 0x63E380.
void RndShaderDisplayTextureCube::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    auto& pixel = defines.GetDefines(kShaderProgramPixel);
    mUseMipLevel = pixel.AddBool(Symbol("HX_USE_MIP_LEVEL"));
    mBlendTextures = pixel.AddBool(Symbol("HX_BLEND_TEXTURES"));
    mUseTexArray = pixel.AddBool(Symbol("HX_USE_TEXARRAY"));

    mColor = cbuffer.AddConstant(kShaderNumericFloat4, "gColor");
    mMipLevel = cbuffer.AddConstant(kShaderNumericFloat, "gMipLevel");
    mBlendAmount = cbuffer.AddConstant(kShaderNumericFloat, "gBlendAmount");
    mArrayIndices = cbuffer.AddConstant(kShaderNumericFloat2, "gArrayIndices");
    mCBufferSize = cbuffer.mSize;

    mTexture0 = resources.AddTexture(
        "gTexture0",
        "gTexSampler0",
        RndTextureBase::kTextureCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTexture1 = resources.AddTexture(
        "gTexture1",
        "gTexSampler1",
        RndTextureBase::kTextureCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTexArray = resources.AddTexture(
        "gTexArray",
        "gTexArraySampler",
        RndTextureBase::kTextureArrayCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x63E590.
const char* RndShaderDisplayTextureCube::_GetClassNameImpl() const {
    return "RndShaderDisplayTextureCube";
}
