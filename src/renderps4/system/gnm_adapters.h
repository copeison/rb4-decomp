#pragma once

// Stand-ins for the Gnm and Gnmx SDK types and calls the PS4 backend uses.
// The SDK has no object file in the reference map, so none of these names
// are in it. The draw-command calls take the graphics context, which the
// project still models as rb4::OrbisRenderCommandContext.

namespace rb4 {
struct OrbisRenderCommandContext;
}

// sce::Gnm::Buffer.
struct GnmBuffer {
    unsigned int mRegisters[4];
};

// sce::Gnm::Sampler.
struct GnmSampler {
    unsigned int mRegisters[4];
};

static_assert(sizeof(GnmBuffer) == 16);
static_assert(sizeof(GnmSampler) == 16);

// sce::Gnm::ShaderStage.
enum class GnmShaderStage : unsigned int {
    kCompute = 0,
    kPixel = 1,
    kVertex = 2,
    kGeometry = 3,
    kHull = 5,
    kLocal = 6,
};

// sce::Gnm::IndexSize.
enum class GnmIndexSize : unsigned int {
    k16Bit = 0,
    k32Bit = 1,
};

// sce::Gnm::PrimitiveType.
enum class GnmPrimitiveType : unsigned int {
    kTriangles = 3,
};

// sce::Gnm::CachePolicy.
enum class GnmCachePolicy : unsigned int {
    kBypass = 2,
};

// sce::Gnm::DataFormat values used by the vertex streams.
enum class GnmDataFormat : unsigned int {
    kInvalid = 0,
    kR32Uint = 0x00004404,
    kR8G8Uint = 0x0002C403,
    kR16G16Uint = 0x0002C405,
    kR8G8Unorm = 0x0022C003,
    kR16G16Unorm = 0x0022C005,
    kR8G8Snorm = 0x0022C103,
    kR16G16Snorm = 0x0022C105,
    kR16G16Float = 0x0022C705,
    kR32G32Float = 0x0022C70B,
    kR32G32B32Float = 0x003AC70D,
    kR8G8B8A8Uint = 0x00FAC40A,
    kR16G16B16A16Uint = 0x00FAC40C,
    kR8G8B8A8Unorm = 0x00FAC00A,
    kR16G16B16A16Unorm = 0x00FAC00C,
    kR8G8B8A8Snorm = 0x00FAC10A,
    kR16G16B16A16Snorm = 0x00FAC10C,
    kR16G16B16A16Float = 0x00FAC70C,
    kR32G32B32A32Float = 0x00FAC70E,
};

// sce::Gnm::ResourceMemoryType.
enum class GnmResourceMemoryType : unsigned int {
    kReadOnly = 0x10,
};

// sce::Gnm::Buffer::initAsVertexBuffer.
void GnmInitAsVertexBuffer(
    GnmBuffer& buffer,
    const void* data,
    GnmDataFormat format,
    unsigned int stride,
    unsigned int numElements);
// sce::Gnm::Buffer::setResourceMemoryType.
void GnmSetResourceMemoryType(GnmBuffer& buffer, GnmResourceMemoryType type);

// sce::Gnmx::GfxContext::setVertexBuffers.
void GfxSetVertexBuffers(
    rb4::OrbisRenderCommandContext& context,
    unsigned int firstSlot,
    unsigned int numSlots,
    const GnmBuffer* buffers);
// sce::Gnmx::GfxContext::allocateFromCommandBuffer.
void* GfxAllocateFromCommandBuffer(
    rb4::OrbisRenderCommandContext& context,
    unsigned long size,
    unsigned long alignment);
// sce::Gnm::DrawCommandBuffer::setNumInstances.
void GfxSetNumInstances(rb4::OrbisRenderCommandContext& context, unsigned int numInstances);
// sce::Gnmx::GfxContext::setPrimitiveType.
void GfxSetPrimitiveType(rb4::OrbisRenderCommandContext& context, GnmPrimitiveType type);
// sce::Gnm::DrawCommandBuffer::setIndexSize.
void GfxSetIndexSize(
    rb4::OrbisRenderCommandContext& context,
    GnmIndexSize size,
    GnmCachePolicy policy);
// sce::Gnmx::GfxContext::prepareDraw (inlined by the SDK into each draw).
void GfxPrepareDraw(rb4::OrbisRenderCommandContext& context);
// sce::Gnm::DrawCommandBuffer::drawIndex.
void GfxDrawIndex(
    rb4::OrbisRenderCommandContext& context,
    unsigned int numIndices,
    const void* indices);
// sce::Gnm::DrawCommandBuffer::drawIndexAuto.
void GfxDrawIndexAuto(rb4::OrbisRenderCommandContext& context, unsigned int numVertices);
// sce::Gnmx::GfxContext::finishDraw (inlined by the SDK into each draw).
void GfxFinishDraw(rb4::OrbisRenderCommandContext& context);
