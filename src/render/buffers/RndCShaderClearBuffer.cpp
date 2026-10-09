#include "render/buffers/RndCShaderClearBuffer.h"

#include <cstring>

#include "render/buffers/RndComputeBuffer.h"
#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Thread-group widths of the 2D and 3D clears, set by a static initializer
// at 0x637BF0; the first is unused. The 1D clear uses groups of 64. Names
// not in the reference map.
int gClearUnknown = -1;     // 0x1AAB6E0
int gClearGroupSize2D = 8;  // 0x1AAB6E4
int gClearGroupSize3D = 4;  // 0x1AAB6E8
constexpr int kClearGroupSize1D = 64;
constexpr unsigned long kComputeKey = 4;

unsigned int NumGroups(int count, int groupSize) {
    const int groups = count / groupSize;
    return static_cast<unsigned int>(groups + (groups * groupSize < count ? 1 : 0));
}

}  // namespace

// Reconstructed from eboot.elf at 0x637210.
RndCShaderClearBuffer::RndCShaderClearBuffer()
    : mNumericType{},
      mTextureType{},
      mClearColor(-1),
      mCBufferSize(0),
      mUintBuffer(-1),
      mUintTex1D(-1),
      mUintTex2D(-1),
      mFloat4Buffer(-1),
      mFloat4Tex1D(-1),
      mFloat4Tex2D(-1) {}

RndCShaderClearBuffer::~RndCShaderClearBuffer() {}

// Reconstructed from eboot.elf at 0x6372B0.
unsigned long RndCShaderClearBuffer::Select(RndContext& context, Params& params) {
    RndShaderResource* target;
    int textureType;
    unsigned long slot = static_cast<unsigned long>(-1);
    const auto numericType = params.mNumericType;
    if (params.mTexture != nullptr) {
        target = params.mTexture;
        textureType = params.mTexture->_GetTypeImpl();
        if (textureType == RndTextureBase::kTexture1D) {
            if (numericType == kShaderNumericFloat4) {
                slot = mFloat4Tex1D;
            } else if (numericType == kShaderNumericUInt) {
                slot = mUintTex1D;
            }
        } else if (textureType == RndTextureBase::kTexture2D) {
            if (numericType == kShaderNumericFloat4) {
                slot = mFloat4Tex2D;
            } else if (numericType == kShaderNumericUInt) {
                slot = mUintTex2D;
            }
        }
    } else {
        target = params.mBuffer;
        textureType = -1;
        if (numericType == kShaderNumericFloat4) {
            slot = mFloat4Buffer;
        } else if (numericType == kShaderNumericUInt) {
            slot = mUintBuffer;
        }
    }

    RndShaderKeyGroup keys{};
    keys.mKeys[kComputeKey] = mNumericType.SetValue(0, numericType);
    keys.mKeys[kComputeKey] = mTextureType.SetValue(
        keys.mKeys[kComputeKey], static_cast<unsigned int>(textureType));
    _SelectShaderCollection(context, keys);

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    std::memcpy(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mClearColor),
        &params.mClearValue,
        sizeof(params.mClearValue));
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    if (target != nullptr) {
        target->Select(context, kShaderProgramCompute, slot, RndShaderResource::kSelectReadWrite, 0);
    }
    return slot;
}

// Reconstructed from eboot.elf at 0x6374F0.
void RndCShaderClearBuffer::Dispatch(RndContext& context, Params& params) {
    int width;
    int height;
    int depth;
    int dimensions;
    if (params.mTexture != nullptr) {
        auto* texture = params.mTexture;
        width = static_cast<int>(texture->mBaseDesc.mWidth);
        height = static_cast<int>(texture->mBaseDesc.mHeight);
        depth = static_cast<int>(texture->mBaseDesc.mDepth);
        const int textureType = texture->_GetTypeImpl();
        dimensions = textureType == RndTextureBase::kTexture1D   ? 1
            : textureType == RndTextureBase::kTexture2D           ? 2
                                                                  : 0;
    } else {
        width = static_cast<int>(params.mBuffer->mDesc.mNumElements);
        height = 1;
        depth = 1;
        dimensions = 1;
    }
    Select(context, params);

    unsigned int groupsX = 0;
    unsigned int groupsY = 0;
    unsigned int groupsZ = 0;
    switch (dimensions & 3) {
    case 3:
        groupsX = NumGroups(width, gClearGroupSize3D);
        groupsY = NumGroups(height, gClearGroupSize3D);
        groupsZ = NumGroups(depth, gClearGroupSize3D);
        break;
    case 2:
        groupsX = NumGroups(width, gClearGroupSize2D);
        groupsY = NumGroups(height, gClearGroupSize2D);
        groupsZ = 1;
        break;
    case 1:
        groupsX = NumGroups(width, kClearGroupSize1D);
        groupsY = 1;
        groupsZ = 1;
        break;
    }
    (void)gClearUnknown;
    context._DispatchComputeImpl(groupsX, groupsY, groupsZ);
}

const char* RndCShaderClearBuffer::_GetClassNameImpl() const {
    return "RndCShaderClearBuffer";
}

const char* RndCShaderClearBuffer::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/ClearBuffer.hlsl";
}

void RndCShaderClearBuffer::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    auto& compute = defines.GetDefines(kShaderProgramCompute);
    mNumericType = compute.Add(Symbol("HX_NUMERIC_TYPE"), 0, 17);
    mTextureType = compute.Add(Symbol("HX_TEXTURE_TYPE"), -1, 8);

    fixedDefines.Add(Symbol("HX_NUMERIC_TYPE_UINT"), kShaderNumericUInt);
    fixedDefines.Add(Symbol("HX_NUMERIC_TYPE_FLOAT4"), kShaderNumericFloat4);
    fixedDefines.Add(Symbol("HX_TEXTURE_TYPE_INVALID"), -1);
    fixedDefines.Add(Symbol("HX_TEXTURE_TYPE_1D"), RndTextureBase::kTexture1D);
    fixedDefines.Add(Symbol("HX_TEXTURE_TYPE_2D"), RndTextureBase::kTexture2D);

    mClearColor = cbuffer.AddConstant(kShaderNumericFloat4, "gClearColor");
    mCBufferSize = cbuffer.mSize;
    mUintBuffer = resources.AddComputeBufferWritable(
        "gUintBuffer", 0, kShaderNumericUInt, kShaderProgramCompute);
    mUintTex1D = resources.AddTextureWritable(
        "gUintTex1D",
        RndTextureBase::kTexture1D,
        kShaderProgramCompute,
        kShaderNumericUInt);
    mUintTex2D = resources.AddTextureWritable(
        "gUintTex2D",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericUInt);
    mFloat4Buffer = resources.AddComputeBufferWritable(
        "gFloat4Buffer", 0, kShaderNumericFloat4, kShaderProgramCompute);
    mFloat4Tex1D = resources.AddTextureWritable(
        "gFloat4Tex1D",
        RndTextureBase::kTexture1D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mFloat4Tex2D = resources.AddTextureWritable(
        "gFloat4Tex2D",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x637B60. Compute permutations require a
// uint or float4 numeric type and an invalid, 1D, or 2D texture type.
bool RndCShaderClearBuffer::_UsesShaderKeyImpl(
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
