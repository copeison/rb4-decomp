#pragma once

#include <cstddef>

#include "render/meshes/RndMesh.h"
#include "render/platform/orbis/meshes/orbis_vertex_descriptors.h"

namespace rb4 {
struct OrbisRenderCommandContext;
}

// Per-format vertex buffer for immediate-mode draws. PS4Context embeds two
// banks of eight, one bank per frame; Reset discards a bank's vertices when
// its frame comes round again.
class PS4TransientBuffer {
public:
    PS4TransientBuffer();   // 0x8EC7C0
    ~PS4TransientBuffer();  // 0x8EC7D0

    void Init(RndVertexType type, unsigned long numVerts);  // 0x8EC7E0
    void Reset();                                          // 0x8EC8B0
    // Returns the index of the first written vertex.
    unsigned long Write(const void* verts, unsigned long numVerts);  // 0x8EC8C0
    // The map has Bind(sce::Gnmx::GfxContext&); the project models the
    // GfxContext as rb4::OrbisRenderCommandContext.
    void Bind(rb4::OrbisRenderCommandContext& context) const;  // 0x8EC910

    // Field names are not in the reference map.
    rb4::OrbisBufferDescriptor mVertexBuffers[rb4::kMeshVertexStreamCount];
    unsigned int mBufferMask;
    unsigned int mUnknown132;
    unsigned char* mData;
    unsigned long mStride;
    unsigned long mCapacity;
    unsigned long mNumVerts;
};

static_assert(offsetof(PS4TransientBuffer, mBufferMask) == 128);
static_assert(offsetof(PS4TransientBuffer, mData) == 136);
static_assert(offsetof(PS4TransientBuffer, mStride) == 144);
static_assert(offsetof(PS4TransientBuffer, mCapacity) == 152);
static_assert(offsetof(PS4TransientBuffer, mNumVerts) == 160);
static_assert(sizeof(PS4TransientBuffer) == 168);
