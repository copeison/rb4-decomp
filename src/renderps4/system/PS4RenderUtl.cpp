#include "renderps4/system/PS4RenderUtl.h"

#include "render/meshes/RndMesh.h"
#include "render/meshes/RndVertexInterpreter.h"
#include "render/textures/RndPixelFormat.h"
#include "renderps4/context/PS4RenderStateUtl.h"
#include "renderps4/context/PS4Context.h"

namespace {

using WrapMode = PS4RenderStateUtl::WrapMode;

sce::Gnm::DataFormat TwoComponentFormat(RndVertexDataType type) {
    switch (type) {
    case kVertexDataFloat32:
        return sce::Gnm::kDataFormatR32G32Float;
    case kVertexDataFloat16:
        return sce::Gnm::kDataFormatR16G16Float;
    case kVertexDataUNorm8:
        return sce::Gnm::kDataFormatR8G8Unorm;
    case kVertexDataUNorm16:
        return sce::Gnm::kDataFormatR16G16Unorm;
    case kVertexDataSNorm8:
        return sce::Gnm::kDataFormatR8G8Snorm;
    case kVertexDataSNorm16:
        return sce::Gnm::kDataFormatR16G16Snorm;
    case kVertexDataUInt8:
        return sce::Gnm::kDataFormatR8G8Uint;
    case kVertexDataUInt16:
        return sce::Gnm::kDataFormatR16G16Uint;
    default:
        return sce::Gnm::kDataFormatInvalid;
    }
}

sce::Gnm::DataFormat FourComponentFormat(RndVertexDataType type) {
    switch (type) {
    case kVertexDataFloat32:
        return sce::Gnm::kDataFormatR32G32B32A32Float;
    case kVertexDataFloat16:
        return sce::Gnm::kDataFormatR16G16B16A16Float;
    case kVertexDataUNorm8:
        return sce::Gnm::kDataFormatR8G8B8A8Unorm;
    case kVertexDataUNorm16:
        return sce::Gnm::kDataFormatR16G16B16A16Unorm;
    case kVertexDataSNorm8:
        return sce::Gnm::kDataFormatR8G8B8A8Snorm;
    case kVertexDataSNorm16:
        return sce::Gnm::kDataFormatR16G16B16A16Snorm;
    case kVertexDataUInt8:
        return sce::Gnm::kDataFormatR8G8B8A8Uint;
    case kVertexDataUInt16:
        return sce::Gnm::kDataFormatR16G16B16A16Uint;
    default:
        return sce::Gnm::kDataFormatInvalid;
    }
}

sce::Gnm::DataFormat StreamDataFormat(const RndVertexInterpreter::StreamLayout& stream) {
    switch (stream.mNumComponents) {
    case 2:
        return TwoComponentFormat(stream.mType);
    case 3:
        if (stream.mType == kVertexDataFloat32) {
            return sce::Gnm::kDataFormatR32G32B32Float;
        }
        return sce::Gnm::kDataFormatInvalid;
    case 4:
        return FourComponentFormat(stream.mType);
    default:
        return sce::Gnm::kDataFormatInvalid;
    }
}

unsigned int DataTypeSize(RndVertexDataType type) {
    switch (type) {
    case kVertexDataFloat32:
        return 4;
    case kVertexDataFloat16:
    case kVertexDataUNorm16:
    case kVertexDataSNorm16:
    case kVertexDataUInt16:
        return 2;
    case kVertexDataUNorm8:
    case kVertexDataSNorm8:
    case kVertexDataUInt8:
        return 1;
    default:
        return 0;
    }
}

void InitReadOnlyVertexBuffer(
    sce::Gnm::Buffer& buffer,
    const void* data,
    sce::Gnm::DataFormat format,
    unsigned int stride,
    unsigned int numElements) {
    buffer.initAsVertexBuffer(const_cast<void*>(data), format, stride, numElements);
    buffer.setResourceMemoryType(sce::Gnm::kResourceMemoryTypeRO);
}

constexpr unsigned long kSamplerSlotCount = 16;

bool ShouldSelectSampler(unsigned long slot, unsigned int flags) {
    return slot < kSamplerSlotCount &&
        (flags & PS4RenderUtl::kSelectNoSampler) == 0;
}

void SelectSampledTexture(
    RndContext& context,
    sce::Gnm::ShaderStage stage,
    unsigned long slot,
    sce::Gnm::Texture* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags) {
    auto& ps4 = static_cast<PS4Context&>(context);
    const auto index = static_cast<unsigned int>(slot);
    if (ShouldSelectSampler(slot, flags)) {
        ps4._BindGraphicsTextureSampler(stage, index, static_cast<WrapMode>(wrap), filter);
    }
    ps4._BindGraphicsTexture(stage, index, texture);
}

}  // namespace

// Reconstructed from eboot.elf at 0x8E1770.
sce::Gnm::PrimitiveType PS4RenderUtl::GetPrimitiveType(RndPrimitive primitive) {
    constexpr sce::Gnm::PrimitiveType kPrimitiveTypes[] = {
        sce::Gnm::kPrimitiveTypePointList,
        sce::Gnm::kPrimitiveTypeLineList,
        sce::Gnm::kPrimitiveTypeLineStrip,
        sce::Gnm::kPrimitiveTypeTriList,
        sce::Gnm::kPrimitiveTypeTriStrip,
    };
    const auto index = static_cast<unsigned int>(primitive);
    if (index >= sizeof(kPrimitiveTypes) / sizeof(kPrimitiveTypes[0])) {
        return sce::Gnm::kPrimitiveTypeTriStrip;
    }
    return kPrimitiveTypes[index];
}

// Reconstructed from eboot.elf at 0x8E1790. The binary looks the name up for
// an assertion compiled out of this build.
sce::Gnm::DataFormat PS4RenderUtl::GetDataFormat(int dataFormat) {
    switch (dataFormat) {
    case 0:  // R_UNorm8
        return sce::Gnm::kDataFormatR8Unorm;
    case 1:  // RG_UNorm8
        return sce::Gnm::kDataFormatR8G8Unorm;
    case 6:  // RGBA_UNorm8
        return sce::Gnm::kDataFormatR8G8B8A8Unorm;
    case 7:  // RGBA_UNorm8_sRGB
        return sce::Gnm::kDataFormatR8G8B8A8UnormSrgb;
    case 10:  // RGBA_UInt8
        return sce::Gnm::kDataFormatR8G8B8A8Uint;
    case 11:  // BGRA_UNorm8
        return sce::Gnm::kDataFormatB8G8R8A8Unorm;
    case 12:  // BGRA_UNorm8_sRGB
    case 14:  // BGRX_UNorm8_sRGB
        return sce::Gnm::kDataFormatB8G8R8A8UnormSrgb;
    case 13:  // BGRX_UNorm8
        return sce::Gnm::kDataFormatB8G8R8X8Unorm;
    case 15:  // R_UNorm16
        return sce::Gnm::kDataFormatR16Unorm;
    case 16:  // R_Float16
        return sce::Gnm::kDataFormatR16Float;
    case 17:  // RG_UNorm16
        return sce::Gnm::kDataFormatR16G16Unorm;
    case 18:  // RG_Float16
        return sce::Gnm::kDataFormatR16G16Float;
    case 20:  // RGBA_UNorm16
        return sce::Gnm::kDataFormatR16G16B16A16Unorm;
    case 21:  // RGBA_Float16
        return sce::Gnm::kDataFormatR16G16B16A16Float;
    case 22:  // R_Float32
        return sce::Gnm::kDataFormatR32Float;
    case 23:  // RG_Float32
        return sce::Gnm::kDataFormatR32G32Float;
    case 24:  // RGBA_Float32
        return sce::Gnm::kDataFormatR32G32B32A32Float;
    case 25:  // RGBA_UNorm1010102
        return sce::Gnm::kDataFormatR10G10B10A2Unorm;
    case 26:  // RGB_Float111110
        return sce::Gnm::kDataFormatR11G11B10Float;
    case 27:  // BC1
    case 29:  // BC1A
        return sce::Gnm::kDataFormatBc1Unorm;
    case 28:  // BC1_sRGB
    case 30:  // BC1A_sRGB
        return sce::Gnm::kDataFormatBc1UnormSrgb;
    case 31:  // BC2
        return sce::Gnm::kDataFormatBc2Unorm;
    case 32:  // BC2_sRGB
        return sce::Gnm::kDataFormatBc2UnormSrgb;
    case 33:  // BC3
        return sce::Gnm::kDataFormatBc3Unorm;
    case 34:  // BC3_sRGB
        return sce::Gnm::kDataFormatBc3UnormSrgb;
    case 35:  // BC4U
        return sce::Gnm::kDataFormatBc4Unorm;
    case 36:  // BC4S
        return sce::Gnm::kDataFormatBc4Snorm;
    case 37:  // BC5U
        return sce::Gnm::kDataFormatBc5Unorm;
    case 38:  // BC5S
        return sce::Gnm::kDataFormatBc5Snorm;
    case 39:  // BC6HU
        return sce::Gnm::kDataFormatBc6Uf16;
    case 40:  // BC6HS
        return sce::Gnm::kDataFormatBc6Sf16;
    case 41:  // BC7
    case 43:  // BC7A
        return sce::Gnm::kDataFormatBc7Unorm;
    case 42:  // BC7_sRGB
    case 44:  // BC7A_sRGB
        return sce::Gnm::kDataFormatBc7UnormSrgb;
    default:
        (void)RndDataFormatName(dataFormat);
        return sce::Gnm::kDataFormatInvalid;
    }
}

// Reconstructed from eboot.elf at 0x8E1820.
sce::GpuAddress::SurfaceType PS4RenderUtl::GetSurfaceType(const RndPixelFormat& format) {
    if (format.mUsage == kTextureUsageDepth) {
        return sce::GpuAddress::kSurfaceTypeDepthOnlyTarget;
    }
    return (format.mFlags & kPixelFormatRenderTarget) != 0
        ? sce::GpuAddress::kSurfaceTypeColorTarget
        : sce::GpuAddress::kSurfaceTypeTextureFlat;
}

// Reconstructed from eboot.elf at 0x8E1840.
void PS4RenderUtl::InitializeVertexBuffers(
    sce::Gnm::Buffer* buffers,
    const void* data,
    unsigned int& mask,
    unsigned int numVerts,
    const RndVertexInterpreter& interpreter) {
    const auto* bytes = static_cast<const unsigned char*>(data);
    for (unsigned int stream = 0; stream < RndVertexInterpreter::kNumStreams; ++stream) {
        if (!interpreter.UsesStream(stream)) {
            continue;
        }

        const auto layout = interpreter.GetStreamLayout(stream);
        const auto elementSize =
            static_cast<unsigned int>(layout.mNumComponents * DataTypeSize(layout.mType));
        const auto stride = numVerts == 1 ? 0 : interpreter.mStride;
        const auto numElements = numVerts == 1 ? elementSize : numVerts;
        InitReadOnlyVertexBuffer(
            buffers[stream],
            bytes + layout.mOffset,
            StreamDataFormat(layout),
            static_cast<unsigned int>(stride),
            numElements);
        mask |= 1U << stream;
    }
}

// Reconstructed from eboot.elf at 0x8E1BD0.
void PS4RenderUtl::InitializeInstanceBuffer(
    sce::Gnm::Buffer* buffers,
    const RndInstanceData* data,
    unsigned int numInstances) {
    constexpr unsigned long offsets[kNumInstanceStreams] = {
        0, 16, 32, 48, 60, 72, 84, 88, 104,
    };
    const sce::Gnm::DataFormat formats[kNumInstanceStreams] = {
        sce::Gnm::kDataFormatR32G32B32A32Float,
        sce::Gnm::kDataFormatR32G32B32A32Float,
        sce::Gnm::kDataFormatR32G32B32A32Float,
        sce::Gnm::kDataFormatR32G32B32Float,
        sce::Gnm::kDataFormatR32G32B32Float,
        sce::Gnm::kDataFormatR32G32B32Float,
        sce::Gnm::kDataFormatR32Uint,
        sce::Gnm::kDataFormatR32G32B32A32Float,
        sce::Gnm::kDataFormatR32G32B32A32Float,
    };
    constexpr unsigned int sizes[kNumInstanceStreams] = {
        16, 16, 16, 12, 12, 12, 4, 16, 16,
    };

    const auto* bytes = reinterpret_cast<const unsigned char*>(data);
    for (unsigned int stream = 0; stream < kNumInstanceStreams; ++stream) {
        const auto stride = numInstances == 1
            ? 0U
            : static_cast<unsigned int>(sizeof(RndInstanceData));
        const auto numElements = numInstances == 1 ? sizes[stream] : numInstances;
        InitReadOnlyVertexBuffer(
            buffers[stream], bytes + offsets[stream], formats[stream], stride, numElements);
    }
}

// Reconstructed from eboot.elf at 0x8E1EE0.
void PS4RenderUtl::SelectTextureForVS(
    RndContext& context,
    unsigned long slot,
    sce::Gnm::Texture* texture,
    unsigned int wrap,
    unsigned int filter) {
    SelectSampledTexture(
        context, sce::Gnm::kShaderStageVs, slot, texture, wrap, filter, 0);
}

// Reconstructed from eboot.elf at 0x8E1F90.
void PS4RenderUtl::SelectTextureForHS(
    RndContext& context,
    unsigned long slot,
    sce::Gnm::Texture* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags) {
    SelectSampledTexture(
        context, sce::Gnm::kShaderStageHs, slot, texture, wrap, filter, flags);
}

// Reconstructed from eboot.elf at 0x8E2040. Domain textures bind to the
// Gnm local stage.
void PS4RenderUtl::SelectTextureForDS(
    RndContext& context,
    unsigned long slot,
    sce::Gnm::Texture* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags) {
    SelectSampledTexture(
        context, sce::Gnm::kShaderStageLs, slot, texture, wrap, filter, flags);
}

// Reconstructed from eboot.elf at 0x8E20F0.
void PS4RenderUtl::SelectTextureForGS(
    RndContext& context,
    unsigned long slot,
    sce::Gnm::Texture* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags) {
    SelectSampledTexture(
        context, sce::Gnm::kShaderStageGs, slot, texture, wrap, filter, flags);
}

// Reconstructed from eboot.elf at 0x8E21A0.
void PS4RenderUtl::SelectTextureForPS(
    RndContext& context,
    unsigned long slot,
    sce::Gnm::Texture* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags) {
    if ((flags & kSelectWritable) != 0) {
        static_cast<PS4Context&>(context)._BindGraphicsRwTexture(
            sce::Gnm::kShaderStagePs, static_cast<unsigned int>(slot), texture);
        return;
    }
    SelectSampledTexture(
        context, sce::Gnm::kShaderStagePs, slot, texture, wrap, filter, flags);
}

// Reconstructed from eboot.elf at 0x8E2290.
void PS4RenderUtl::SelectTextureForCS(
    RndContext& context,
    unsigned long slot,
    sce::Gnm::Texture* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags) {
    auto& ps4 = static_cast<PS4Context&>(context);
    const auto index = static_cast<unsigned int>(slot);
    const bool computeQueue = ps4._UsesComputeQueue();
    if ((flags & kSelectWritable) != 0) {
        if (computeQueue) {
            ps4._BindComputeRwTexture(index, texture);
        } else {
            ps4._BindGraphicsRwTexture(sce::Gnm::kShaderStageCs, index, texture);
        }
        return;
    }

    if (computeQueue) {
        ps4._BindComputeTexture(index, texture);
        if (ShouldSelectSampler(slot, flags)) {
            ps4._BindComputeTextureSampler(index, static_cast<WrapMode>(wrap), filter);
        }
        return;
    }

    SelectSampledTexture(
        context, sce::Gnm::kShaderStageCs, slot, texture, wrap, filter, flags);
}
