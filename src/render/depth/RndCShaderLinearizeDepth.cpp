#include "render/depth/RndCShaderLinearizeDepth.h"

#include "render/context/RndCameraContext.h"
#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"
#include "utl/text/Symbol.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x638090; only the 2D width is used. Names not in the
// reference map.
[[maybe_unused]] int gLinearizeDepthGroupSizeUnused = -1;  // 0x1AAB6EC
int gLinearizeDepthGroupSize2D = 8;                        // 0x1AAB6F0
[[maybe_unused]] int gLinearizeDepthGroupSize3D = 4;       // 0x1AAB6F4

// Each thread converts a 2x2 tile (HX_TILE_SIZE_X, HX_TILE_SIZE_Y). Names
// not in the reference map.
constexpr int kTileSize = 2;
constexpr unsigned long kComputeKey = 4;

int CeilDiv(int count, int divisor) {
    const int quotient = count / divisor;
    return quotient * divisor < count ? quotient + 1 : quotient;
}

}  // namespace

// Reconstructed from eboot.elf at 0x637C10.
RndCShaderLinearizeDepth::RndCShaderLinearizeDepth()
    : mIsOrtho{},
      mDepth(-1),
      mLinearDepth(-1),
      mDimensions(-1),
      mCBufferSize(0) {}

// Reconstructed from eboot.elf at 0x637C70.
RndCShaderLinearizeDepth::~RndCShaderLinearizeDepth() {}

// Reconstructed from eboot.elf at 0x637CA0.
void RndCShaderLinearizeDepth::Dispatch(
    RndContext& context,
    Params& params) {
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Linearize Depth");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    const RndCameraContext& camera = context.mCameras[0];
    const bool ortho = camera.mCamera == nullptr
        || camera.GetCameraSettings().mOrthographic;
    RndShaderKeyGroup keys{};
    keys.mKeys[kComputeKey] = mIsOrtho.SetValue(0, ortho);
    _SelectShaderCollection(context, keys);

    params.mDepth->Select(context, kShaderProgramCompute, mDepth, 0, 0);
    params.mLinearDepth->Select(
        context,
        kShaderProgramCompute,
        mLinearDepth,
        RndShaderResource::kSelectReadWrite,
        0);

    const int width = static_cast<int>(params.mDepth->mBaseDesc.mWidth);
    const int height = static_cast<int>(params.mDepth->mBaseDesc.mHeight);
    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    auto* dimensions = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mDimensions));
    dimensions[0] = static_cast<float>(width);
    dimensions[1] = static_cast<float>(height);
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    const int groupSize = kTileSize * gLinearizeDepthGroupSize2D;
    context._DispatchComputeImpl(
        static_cast<unsigned int>(CeilDiv(width, groupSize)),
        static_cast<unsigned int>(CeilDiv(height, groupSize)),
        1);
}

// Reconstructed from eboot.elf at 0x637F20.
const char* RndCShaderLinearizeDepth::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/LinearizeDepthCompute.hlsl";
}

// Reconstructed from eboot.elf at 0x637F30.
void RndCShaderLinearizeDepth::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    fixedDefines.Add(Symbol("HX_TILE_SIZE_X"), kTileSize);
    fixedDefines.Add(Symbol("HX_TILE_SIZE_Y"), kTileSize);

    mIsOrtho = defines.GetDefines(kShaderProgramCompute)
                   .AddBool(Symbol("HX_IS_ORTHO"));

    mDepth = resources.AddTexture(
        "gDepth",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat);
    mLinearDepth = resources.AddTextureWritable(
        "gLinearDepth",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat);
    mDimensions = cbuffer.AddConstant(kShaderNumericFloat2, "gDimensions");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x638080.
const char* RndCShaderLinearizeDepth::_GetClassNameImpl() const {
    return "RndCShaderLinearizeDepth";
}
