#include "renderps4/buffers/PS4TransientBuffer.h"

#include <cstring>

#include "os/memory/MemMgr.h"
#include "render/platform/orbis/meshes/orbis_gnm_mesh_api.h"
#include "render/platform/orbis/meshes/orbis_mesh_formats.h"
#include "renderps4/system/PS4Device.h"

namespace {

constexpr const char* kAllocationName = "TransientBuffer";

}  // namespace

// Reconstructed from eboot.elf at 0x8EC7C0.
PS4TransientBuffer::PS4TransientBuffer()
    : mData(nullptr), mStride(0), mCapacity(0), mNumVerts(0) {}

// Reconstructed from eboot.elf at 0x8EC7D0.
PS4TransientBuffer::~PS4TransientBuffer() {
    MemFree(mData);
}

// Reconstructed from eboot.elf at 0x8EC7E0.
void PS4TransientBuffer::Init(RndVertexType type, unsigned long numVerts) {
    const auto* format = rb4::render_mesh_format_descriptor(type);
    mData = static_cast<unsigned char*>(
        MemAlloc(numVerts * format->vertex_stride, kAllocationName, 4));
    mBufferMask = 0;
    rb4::orbis_build_mesh_vertex_descriptors(
        mVertexBuffers,
        mData,
        mBufferMask,
        static_cast<unsigned int>(numVerts),
        *format);
    mStride = format->vertex_stride;
    mCapacity = numVerts;
}

// Reconstructed from eboot.elf at 0x8EC8B0.
void PS4TransientBuffer::Reset() {
    mNumVerts = 0;
}

// Reconstructed from eboot.elf at 0x8EC8C0.
unsigned long PS4TransientBuffer::Write(const void* verts, unsigned long numVerts) {
    std::memcpy(mData + mNumVerts * mStride, verts, numVerts * mStride);
    const auto first = mNumVerts;
    mNumVerts += numVerts;
    return first;
}

// Reconstructed from eboot.elf at 0x8EC910. Streams the format does not use
// bind the device's default descriptors.
void PS4TransientBuffer::Bind(rb4::OrbisRenderCommandContext& context) const {
    const auto* defaults = gPS4Device->mDefaultVertexDescs;
    for (unsigned int stream = 0; stream < rb4::kMeshVertexStreamCount; ++stream) {
        const auto* desc = (mBufferMask & (1U << stream)) != 0
            ? &mVertexBuffers[stream]
            : &defaults[stream];
        rb4::orbis_bind_vertex_buffers(context, stream, 1, desc);
    }
}
