#include "render/debug/RndCShaderRenderTestCompute.h"

#include <cmath>

#include "render/buffers/RndComputeBuffer.h"
#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x6F4250; only the 2D width is used. Names not in the
// reference map.
[[maybe_unused]] int gRenderTestGroupSizeUnused = -1;  // 0x1AB1FB0
int gRenderTestGroupSize2D = 8;                        // 0x1AB1FB4
[[maybe_unused]] int gRenderTestGroupSize3D = 4;       // 0x1AB1FB8

// Names not in the reference map.
constexpr unsigned long kComputeKey = 4;

int CeilDiv(int count, int divisor) {
    const int quotient = count / divisor;
    return quotient * divisor < count ? quotient + 1 : quotient;
}

// Rounds half away from zero, saturating at the int range.
int RoundToInt(float value) {
    constexpr float kIntLimit = 2147483648.0F;
    if (value > 0.0F) {
        const float rounded = value + 0.5F;
        return rounded >= kIntLimit ? 0x7FFFFFFF : static_cast<int>(rounded);
    }
    const float rounded = value - 0.5F;
    return rounded <= -kIntLimit ? static_cast<int>(0x80000000U)
                                 : static_cast<int>(rounded);
}

}  // namespace

// Reconstructed from eboot.elf at 0x6F3E00.
RndCShaderRenderTestCompute::RndCShaderRenderTestCompute()
    : mWriteToBuffer{},
      mDstTexture(-1),
      mDstBuffer(-1),
      mDimensions(-1),
      mAnimParams(-1),
      mCBufferSize(0) {}

// Reconstructed from eboot.elf at 0x6F3E50.
RndCShaderRenderTestCompute::~RndCShaderRenderTestCompute() {}

// Reconstructed from eboot.elf at 0x6F3E80. A buffer is treated as a square
// of its elements.
void RndCShaderRenderTestCompute::Dispatch(
    RndContext& context,
    const Params& params) {
    RndShaderKeyGroup keys{};
    keys.mKeys[kComputeKey] =
        mWriteToBuffer.SetValue(0, params.mBuffer != nullptr);
    _SelectShaderCollection(context, keys);

    int width;
    int height;
    if (params.mBuffer != nullptr) {
        const auto count =
            static_cast<float>(params.mBuffer->mDesc.mNumElements);
        width = RoundToInt(std::sqrt(count));
        height = width;
        params.mBuffer->Select(
            context,
            kShaderProgramCompute,
            mDstBuffer,
            RndShaderResource::kSelectReadWrite,
            0);
    } else {
        width = static_cast<int>(params.mTexture->mBaseDesc.mWidth);
        height = static_cast<int>(params.mTexture->mBaseDesc.mHeight);
        params.mTexture->Select(
            context,
            kShaderProgramCompute,
            mDstTexture,
            RndShaderResource::kSelectReadWrite,
            0);
    }

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    auto* dimensions = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mDimensions));
    dimensions[0] = static_cast<float>(width);
    dimensions[1] = static_cast<float>(height);
    *static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mAnimParams)) =
        params.mTime;
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    context._DispatchComputeImpl(
        static_cast<unsigned int>(CeilDiv(width, gRenderTestGroupSize2D)),
        static_cast<unsigned int>(CeilDiv(height, gRenderTestGroupSize2D)),
        1);
}

// Reconstructed from eboot.elf at 0x6F4120.
const char* RndCShaderRenderTestCompute::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/RenderTestCompute.hlsl";
}

// Reconstructed from eboot.elf at 0x6F4130.
void RndCShaderRenderTestCompute::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mWriteToBuffer = defines.GetDefines(kShaderProgramCompute)
                         .AddBool(Symbol("HX_WRITE_TO_BUFFER"));
    mDstTexture = resources.AddTextureWritable(
        "gDstTexture",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDstBuffer = resources.AddComputeBufferWritable(
        "gDstBuffer", 0, kShaderNumericFloat4, kShaderProgramCompute);
    mDimensions = cbuffer.AddConstant(kShaderNumericFloat2, "gDimensions");
    mAnimParams = cbuffer.AddConstant(kShaderNumericFloat, "gAnimParams");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x6F4240.
const char* RndCShaderRenderTestCompute::_GetClassNameImpl() const {
    return "RndCShaderRenderTestCompute";
}
