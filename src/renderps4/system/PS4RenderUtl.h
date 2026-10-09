#pragma once

#include <gnm/buffer.h>
#include <gnm/texture.h>

class RndContext;
class RndVertexInterpreter;
struct RndInstanceData;
enum class RndPrimitive : unsigned int;

// Gnm helpers shared by the PS4 resources.
namespace PS4RenderUtl {

// Unknown primitives draw as triangle strips. The map has
// GetPrimitiveType(RndContext::Primitive); this build's enum is RndPrimitive.
sce::Gnm::PrimitiveType GetPrimitiveType(RndPrimitive primitive);  // 0x8E1770

// Instance data occupies the nine vertex streams after the mesh streams.
// Name not in the reference map.
constexpr unsigned int kNumInstanceStreams = 9;

// Builds a read-only vertex buffer for each stream the layout uses and sets
// its bit in the mask. A single vertex gets a zero stride so every vertex
// reads it. The map has InitializeVertexBuffers(sce::Gnm::Buffer*, void*,
// unsigned int&, unsigned long, RndVertexInterpreter const&); the binary
// takes the count as unsigned int.
void InitializeVertexBuffers(
    sce::Gnm::Buffer* buffers,
    const void* data,
    unsigned int& mask,
    unsigned int numVerts,
    const RndVertexInterpreter& interpreter);  // 0x8E1840
// Builds the nine instance streams. The map has
// InitializeInstanceBuffer(sce::Gnm::Buffer*, void*, unsigned long); the
// binary takes the count as unsigned int.
void InitializeInstanceBuffer(
    sce::Gnm::Buffer* buffers,
    const RndInstanceData* data,
    unsigned int numInstances);               // 0x8E1BD0

// Texture selection for each shader stage. The map has
// SelectTextureFor*(RndContext&, unsigned long, sce::Gnm::Texture*,
// RndTexWrapMode, RndTexFilterMode); this build adds a flags argument to
// every stage but the vertex stage reads none. Slots below 16 also get a
// sampler unless the flags suppress it.
void SelectTextureForVS(
    RndContext& context,
    unsigned long slot,
    sce::Gnm::Texture* texture,
    unsigned int wrap,
    unsigned int filter);                 // 0x8E1EE0
void SelectTextureForHS(
    RndContext& context,
    unsigned long slot,
    sce::Gnm::Texture* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags);                  // 0x8E1F90
void SelectTextureForDS(
    RndContext& context,
    unsigned long slot,
    sce::Gnm::Texture* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags);                  // 0x8E2040
void SelectTextureForGS(
    RndContext& context,
    unsigned long slot,
    sce::Gnm::Texture* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags);                  // 0x8E20F0
// Writable textures bind without a sampler.
void SelectTextureForPS(
    RndContext& context,
    unsigned long slot,
    sce::Gnm::Texture* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags);                  // 0x8E21A0
// Uses the compute queue's own tables when the context records compute work.
void SelectTextureForCS(
    RndContext& context,
    unsigned long slot,
    sce::Gnm::Texture* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags);                  // 0x8E2290

// Flags understood by the selects. Names not in the reference map.
constexpr unsigned int kSelectWritable = 1U << 0;
// Binds a depth texture's stencil plane.
constexpr unsigned int kSelectStencilPlane = 1U << 2;
constexpr unsigned int kSelectNoSampler = 1U << 4;

}  // namespace PS4RenderUtl
