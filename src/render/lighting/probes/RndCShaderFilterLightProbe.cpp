#include "render/lighting/probes/RndCShaderFilterLightProbe.h"

#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDefines.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"
#include "utl/text/Symbol.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x6D7110; only the 2D width is used. Names not in the
// reference map.
[[maybe_unused]] int gFilterLightProbeGroupSizeUnused = -1;  // 0x1AB1884
int gFilterLightProbeGroupSize2D = 8;                        // 0x1AB1888
[[maybe_unused]] int gFilterLightProbeGroupSize3D = 4;       // 0x1AB188C

// Names not in the reference map.
constexpr unsigned long kComputeKey = 4;
// Passed with every bind of this pass; no other draw passes it, and the
// platform selects of this build do not test it. Weak evidence: the shader
// declares the cube textures as 2D arrays, so the flag likely asks for that
// view.
constexpr unsigned int kSelectAsArray = 8;

}  // namespace

// Reconstructed from eboot.elf at 0x6D6CA0.
RndCShaderFilterLightProbe::RndCShaderFilterLightProbe()
    : mProbeTextureType{},
      mCBufferSize(0),
      mTextureSize(-1),
      mSmoothness(-1),
      mSrcTexture(-1),
      mDstTexture(-1) {}

// Reconstructed from eboot.elf at 0x6D6D00.
RndCShaderFilterLightProbe::~RndCShaderFilterLightProbe() {}

// Reconstructed from eboot.elf at 0x6D6D30. Mips smaller than a thread
// group take one group.
void RndCShaderFilterLightProbe::Dispatch(
    RndContext& context,
    Params& params) {
    RndShaderKeyGroup keys{};
    keys.mKeys[kComputeKey] = mProbeTextureType.SetValue(
        0, static_cast<unsigned int>(params.mTextureType));
    _SelectShaderCollection(context, keys);

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    *static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mTextureSize)) =
        static_cast<float>(params.mSize);
    *static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mSmoothness)) =
        params.mSmoothness;
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    params.mSrc->Select(
        context, kShaderProgramCompute, mSrcTexture, kSelectAsArray, 0);
    params.mHelper->Select(
        context, kShaderProgramCompute, mHelperTexture, kSelectAsArray, 0);
    params.mDst->Select(
        context,
        kShaderProgramCompute,
        mDstTexture,
        RndShaderResource::kSelectReadWrite | kSelectAsArray,
        params.mDstMip);

    const auto groupSize =
        static_cast<unsigned int>(gFilterLightProbeGroupSize2D);
    const unsigned int groups =
        groupSize > params.mSize ? 1 : params.mSize / groupSize;
    context._DispatchComputeImpl(groups, groups, 1);
}

// Reconstructed from eboot.elf at 0x6D6F30.
const char* RndCShaderFilterLightProbe::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/FilterLightProbe.hlsl";
}

// Reconstructed from eboot.elf at 0x6D6F40. The samplers are unnamed
// (gNullStr).
void RndCShaderFilterLightProbe::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    fixedDefines.Add(Symbol("HX_PROBE_TEXTURE_DIFFUSE"), 0);
    fixedDefines.Add(Symbol("HX_PROBE_TEXTURE_SPECULAR"), 1);
    mProbeTextureType = defines.GetDefines(kShaderProgramCompute)
                            .Add(Symbol("HX_PROBE_TEXTURE_TYPE"), 0, 2);

    mTextureSize = cbuffer.AddConstant(kShaderNumericFloat, "gTextureSize");
    mSmoothness = cbuffer.AddConstant(kShaderNumericFloat, "gSmoothness");
    mCBufferSize = cbuffer.mSize;

    mSrcTexture = resources.AddTexture(
        "gSrcTexture",
        "",
        RndTextureBase::kTextureArray2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mHelperTexture = resources.AddTexture(
        "gHelperTexture",
        "",
        RndTextureBase::kTextureArray2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDstTexture = resources.AddTextureWritable(
        "gDstTexture",
        RndTextureBase::kTextureArray2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x6D70E0.
RndCShaderFilterLightProbe::Params::Params()
    : mTextureType(-1),
      mSize(0),
      mDstMip(0),
      mSmoothness(0.0F),
      mSrc(nullptr),
      mHelper(nullptr),
      mDst(nullptr) {}

// Reconstructed from eboot.elf at 0x6D7100.
const char* RndCShaderFilterLightProbe::_GetClassNameImpl() const {
    return "RndCShaderFilterLightProbe";
}
