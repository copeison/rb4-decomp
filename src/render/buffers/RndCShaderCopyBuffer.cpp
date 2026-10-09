#include "render/buffers/RndCShaderCopyBuffer.h"

#include "render/buffers/RndComputeBuffer.h"
#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Thread-group widths of the 2D and 3D copies, set by a static initializer
// at 0x6F3DE0; the first is unused. The 1D copy uses groups of 64. Names
// not in the reference map.
int gCopyUnknown = -1;     // 0x1AB1FA4
int gCopyGroupSize2D = 8;  // 0x1AB1FA8
int gCopyGroupSize3D = 4;  // 0x1AB1FAC
constexpr int kCopyGroupSize1D = 64;
constexpr unsigned long kComputeKey = 4;

unsigned int NumGroups(int count, int groupSize) {
    const int groups = count / groupSize;
    return static_cast<unsigned int>(groups + (groups * groupSize < count ? 1 : 0));
}

}  // namespace

// Reconstructed from eboot.elf at 0x6F3550.
RndCShaderCopyBuffer::RndCShaderCopyBuffer()
    : mNumericType{},
      mTextureType{},
      mSrcUintBuffer(-1),
      mSrcUintTex1D(-1),
      mSrcUintTex2D(-1),
      mSrcFloat4Buffer(-1),
      mSrcFloat4Tex1D(-1),
      mSrcFloat4Tex2D(-1),
      mDestUintBuffer(-1),
      mDestUintTex1D(-1),
      mDestUintTex2D(-1),
      mDestFloat4Buffer(-1),
      mDestFloat4Tex1D(-1),
      mDestFloat4Tex2D(-1) {}

RndCShaderCopyBuffer::~RndCShaderCopyBuffer() {}

// Reconstructed from eboot.elf at 0x6F35E0. A buffer source leaves the
// destination unset in the binary, which then writes through a null pointer;
// only texture sources reach it in practice.
void RndCShaderCopyBuffer::Dispatch(RndContext& context, Params& params) {
    RndShaderResource* source;
    RndShaderResource* dest = nullptr;
    int width;
    int height;
    int depth;
    int textureType;
    int dimensions;
    unsigned long sourceSlot = static_cast<unsigned long>(-1);
    unsigned long destSlot = static_cast<unsigned long>(-1);
    const auto numericType = params.mNumericType;
    if (params.mSrcTexture != nullptr) {
        auto* texture = params.mSrcTexture;
        source = texture;
        dest = params.mDestTexture;
        width = static_cast<int>(texture->mBaseDesc.mWidth);
        height = static_cast<int>(texture->mBaseDesc.mHeight);
        depth = static_cast<int>(texture->mBaseDesc.mDepth);
        textureType = texture->_GetTypeImpl();
        if (textureType == RndTextureBase::kTexture1D) {
            dimensions = 1;
            if (numericType == kShaderNumericFloat4) {
                sourceSlot = mSrcFloat4Tex1D;
                destSlot = mDestFloat4Tex1D;
            } else if (numericType == kShaderNumericUInt) {
                sourceSlot = mSrcUintTex1D;
                destSlot = mDestUintTex1D;
            }
        } else if (textureType == RndTextureBase::kTexture2D) {
            dimensions = 2;
            if (numericType == kShaderNumericFloat4) {
                sourceSlot = mSrcFloat4Tex2D;
                destSlot = mDestFloat4Tex2D;
            } else if (numericType == kShaderNumericUInt) {
                sourceSlot = mSrcUintTex2D;
                destSlot = mDestUintTex2D;
            }
        } else {
            dimensions = 0;
        }
    } else {
        auto* buffer = params.mSrcBuffer;
        source = buffer;
        width = static_cast<int>(buffer->mDesc.mNumElements);
        height = 1;
        depth = 1;
        textureType = -1;
        dimensions = 1;
        if (numericType == kShaderNumericFloat4) {
            sourceSlot = mSrcFloat4Buffer;
            destSlot = mDestFloat4Buffer;
        } else if (numericType == kShaderNumericUInt) {
            sourceSlot = mSrcUintBuffer;
            destSlot = mDestUintBuffer;
        }
    }

    RndShaderKeyGroup keys{};
    keys.mKeys[kComputeKey] = mNumericType.SetValue(0, numericType);
    keys.mKeys[kComputeKey] = mTextureType.SetValue(
        keys.mKeys[kComputeKey], static_cast<unsigned int>(textureType));
    _SelectShaderCollection(context, keys);

    source->Select(context, kShaderProgramCompute, sourceSlot, 0, 0);
    dest->Select(context, kShaderProgramCompute, destSlot, RndShaderResource::kSelectReadWrite, 0);

    unsigned int groupsX = 0;
    unsigned int groupsY = 0;
    unsigned int groupsZ = 0;
    switch (dimensions & 3) {
    case 3:
        groupsX = NumGroups(width, gCopyGroupSize3D);
        groupsY = NumGroups(height, gCopyGroupSize3D);
        groupsZ = NumGroups(depth, gCopyGroupSize3D);
        break;
    case 2:
        groupsX = NumGroups(width, gCopyGroupSize2D);
        groupsY = NumGroups(height, gCopyGroupSize2D);
        groupsZ = 1;
        break;
    case 1:
        groupsX = NumGroups(width, kCopyGroupSize1D);
        groupsY = 1;
        groupsZ = 1;
        break;
    }
    (void)gCopyUnknown;
    context._DispatchComputeImpl(groupsX, groupsY, groupsZ);
}

const char* RndCShaderCopyBuffer::_GetClassNameImpl() const {
    return "RndCShaderCopyBuffer";
}

const char* RndCShaderCopyBuffer::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/CopyBuffer.hlsl";
}

void RndCShaderCopyBuffer::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig&,
    RndShaderResourceConfig& resources) {
    auto& compute = defines.GetDefines(kShaderProgramCompute);
    mNumericType = compute.Add(Symbol("HX_NUMERIC_TYPE"), 0, 17);
    mTextureType = compute.Add(Symbol("HX_TEXTURE_TYPE"), -1, 8);

    fixedDefines.Add(Symbol("HX_NUMERIC_TYPE_UINT"), kShaderNumericUInt);
    fixedDefines.Add(Symbol("HX_NUMERIC_TYPE_FLOAT4"), kShaderNumericFloat4);
    fixedDefines.Add(Symbol("HX_TEXTURE_TYPE_INVALID"), -1);
    fixedDefines.Add(Symbol("HX_TEXTURE_TYPE_1D"), RndTextureBase::kTexture1D);
    fixedDefines.Add(Symbol("HX_TEXTURE_TYPE_2D"), RndTextureBase::kTexture2D);

    mSrcUintBuffer = resources.AddComputeBuffer(
        "gSrcUintBuffer", 0, kShaderNumericUInt, kShaderProgramCompute);
    mSrcUintTex1D = resources.AddTexture(
        "gSrcUintTex1D",
        "gSrcUintTex1DSampler",
        RndTextureBase::kTexture1D,
        kShaderProgramCompute,
        kShaderNumericUInt);
    mSrcUintTex2D = resources.AddTexture(
        "gSrcUintTex2D",
        "gSrcUintTex2DSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericUInt);
    mSrcFloat4Buffer = resources.AddComputeBuffer(
        "gSrcFloat4Buffer", 0, kShaderNumericFloat4, kShaderProgramCompute);
    mSrcFloat4Tex1D = resources.AddTexture(
        "gSrcFloat4Tex1D",
        "gSrcFloat4Tex1DSampler",
        RndTextureBase::kTexture1D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mSrcFloat4Tex2D = resources.AddTexture(
        "gSrcFloat4Tex2D",
        "gSrcFloat4Tex2DSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDestUintBuffer = resources.AddComputeBufferWritable(
        "gDestUintBuffer", 0, kShaderNumericUInt, kShaderProgramCompute);
    mDestUintTex1D = resources.AddTextureWritable(
        "gDestUintTex1D",
        RndTextureBase::kTexture1D,
        kShaderProgramCompute,
        kShaderNumericUInt);
    mDestUintTex2D = resources.AddTextureWritable(
        "gDestUintTex2D",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericUInt);
    mDestFloat4Buffer = resources.AddComputeBufferWritable(
        "gDestFloat4Buffer", 0, kShaderNumericFloat4, kShaderProgramCompute);
    mDestFloat4Tex1D = resources.AddTextureWritable(
        "gDestFloat4Tex1D",
        RndTextureBase::kTexture1D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDestFloat4Tex2D = resources.AddTextureWritable(
        "gDestFloat4Tex2D",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x6F3D50. Compute permutations require a
// uint or float4 numeric type and an invalid, 1D, or 2D texture type.
bool RndCShaderCopyBuffer::_UsesShaderKeyImpl(
    RndShaderProgramType type,
    RndShaderKey key) const {
    if (type != kShaderProgramCompute) {
        return true;
    }
    const auto numericType = mNumericType.GetValue(key);
    if (numericType != kShaderNumericFloat4 &&
        numericType != kShaderNumericUInt) {
        return false;
    }
    return mTextureType.GetValue(key) + 1U < 3U;
}
