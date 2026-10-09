#include "renderps4/system/PS4RenderUtl.h"

#include "render/meshes/RndMesh.h"
#include "render/meshes/RndVertexInterpreter.h"
#include "renderps4/context/PS4RenderStateUtl.h"
#include "renderps4/context/PS4Context.h"

namespace {

using WrapMode = PS4RenderStateUtl::WrapMode;

GnmDataFormat TwoComponentFormat(RndVertexDataType type) {
    switch (type) {
    case kVertexDataFloat32:
        return GnmDataFormat::kR32G32Float;
    case kVertexDataFloat16:
        return GnmDataFormat::kR16G16Float;
    case kVertexDataUNorm8:
        return GnmDataFormat::kR8G8Unorm;
    case kVertexDataUNorm16:
        return GnmDataFormat::kR16G16Unorm;
    case kVertexDataSNorm8:
        return GnmDataFormat::kR8G8Snorm;
    case kVertexDataSNorm16:
        return GnmDataFormat::kR16G16Snorm;
    case kVertexDataUInt8:
        return GnmDataFormat::kR8G8Uint;
    case kVertexDataUInt16:
        return GnmDataFormat::kR16G16Uint;
    default:
        return GnmDataFormat::kInvalid;
    }
}

GnmDataFormat FourComponentFormat(RndVertexDataType type) {
    switch (type) {
    case kVertexDataFloat32:
        return GnmDataFormat::kR32G32B32A32Float;
    case kVertexDataFloat16:
        return GnmDataFormat::kR16G16B16A16Float;
    case kVertexDataUNorm8:
        return GnmDataFormat::kR8G8B8A8Unorm;
    case kVertexDataUNorm16:
        return GnmDataFormat::kR16G16B16A16Unorm;
    case kVertexDataSNorm8:
        return GnmDataFormat::kR8G8B8A8Snorm;
    case kVertexDataSNorm16:
        return GnmDataFormat::kR16G16B16A16Snorm;
    case kVertexDataUInt8:
        return GnmDataFormat::kR8G8B8A8Uint;
    case kVertexDataUInt16:
        return GnmDataFormat::kR16G16B16A16Uint;
    default:
        return GnmDataFormat::kInvalid;
    }
}

GnmDataFormat StreamDataFormat(const RndVertexInterpreter::StreamLayout& stream) {
    switch (stream.mNumComponents) {
    case 2:
        return TwoComponentFormat(stream.mType);
    case 3:
        if (stream.mType == kVertexDataFloat32) {
            return GnmDataFormat::kR32G32B32Float;
        }
        return GnmDataFormat::kInvalid;
    case 4:
        return FourComponentFormat(stream.mType);
    default:
        return GnmDataFormat::kInvalid;
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
    GnmBuffer& buffer,
    const void* data,
    GnmDataFormat format,
    unsigned int stride,
    unsigned int numElements) {
    GnmInitAsVertexBuffer(buffer, data, format, stride, numElements);
    GnmSetResourceMemoryType(buffer, GnmResourceMemoryType::kReadOnly);
}

constexpr unsigned long kSamplerSlotCount = 16;

bool ShouldSelectSampler(unsigned long slot, unsigned int flags) {
    return slot < kSamplerSlotCount &&
        (flags & PS4RenderUtl::kSelectNoSampler) == 0;
}

void SelectSampledTexture(
    RndContext& context,
    GnmShaderStage stage,
    unsigned long slot,
    const void* texture,
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

// Reconstructed from eboot.elf at 0x8E1840.
void PS4RenderUtl::InitializeVertexBuffers(
    GnmBuffer* buffers,
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
    GnmBuffer* buffers,
    const RndInstanceData* data,
    unsigned int numInstances) {
    constexpr unsigned long offsets[kNumInstanceStreams] = {
        0, 16, 32, 48, 60, 72, 84, 88, 104,
    };
    constexpr GnmDataFormat formats[kNumInstanceStreams] = {
        GnmDataFormat::kR32G32B32A32Float,
        GnmDataFormat::kR32G32B32A32Float,
        GnmDataFormat::kR32G32B32A32Float,
        GnmDataFormat::kR32G32B32Float,
        GnmDataFormat::kR32G32B32Float,
        GnmDataFormat::kR32G32B32Float,
        GnmDataFormat::kR32Uint,
        GnmDataFormat::kR32G32B32A32Float,
        GnmDataFormat::kR32G32B32A32Float,
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
    const void* texture,
    unsigned int wrap,
    unsigned int filter) {
    SelectSampledTexture(
        context, GnmShaderStage::kVertex, slot, texture, wrap, filter, 0);
}

// Reconstructed from eboot.elf at 0x8E1F90.
void PS4RenderUtl::SelectTextureForHS(
    RndContext& context,
    unsigned long slot,
    const void* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags) {
    SelectSampledTexture(
        context, GnmShaderStage::kHull, slot, texture, wrap, filter, flags);
}

// Reconstructed from eboot.elf at 0x8E2040. Domain textures bind to the
// Gnm local stage.
void PS4RenderUtl::SelectTextureForDS(
    RndContext& context,
    unsigned long slot,
    const void* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags) {
    SelectSampledTexture(
        context, GnmShaderStage::kLocal, slot, texture, wrap, filter, flags);
}

// Reconstructed from eboot.elf at 0x8E20F0.
void PS4RenderUtl::SelectTextureForGS(
    RndContext& context,
    unsigned long slot,
    const void* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags) {
    SelectSampledTexture(
        context, GnmShaderStage::kGeometry, slot, texture, wrap, filter, flags);
}

// Reconstructed from eboot.elf at 0x8E21A0.
void PS4RenderUtl::SelectTextureForPS(
    RndContext& context,
    unsigned long slot,
    const void* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags) {
    if ((flags & kSelectWritable) != 0) {
        static_cast<PS4Context&>(context)._BindGraphicsRwTexture(
            GnmShaderStage::kPixel, static_cast<unsigned int>(slot), texture);
        return;
    }
    SelectSampledTexture(
        context, GnmShaderStage::kPixel, slot, texture, wrap, filter, flags);
}

// Reconstructed from eboot.elf at 0x8E2290.
void PS4RenderUtl::SelectTextureForCS(
    RndContext& context,
    unsigned long slot,
    const void* texture,
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
            ps4._BindGraphicsRwTexture(GnmShaderStage::kCompute, index, texture);
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
        context, GnmShaderStage::kCompute, slot, texture, wrap, filter, flags);
}
