#include "renderps4/buffers/PS4TransientBuffer.h"

#include <cstring>

#include "os/memory/MemMgr.h"
#include "renderps4/system/PS4Device.h"
#include "renderps4/system/PS4RenderUtl.h"

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
    const auto* interpreter = RndVertexInterpreter::GetInstance(type);
    mData = static_cast<unsigned char*>(
        MemAlloc(numVerts * interpreter->mStride, kAllocationName, 4));
    mBufferMask = 0;
    PS4RenderUtl::InitializeVertexBuffers(
        mVertexBuffers,
        mData,
        mBufferMask,
        static_cast<unsigned int>(numVerts),
        *interpreter);
    mStride = interpreter->mStride;
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
    for (unsigned int stream = 0; stream < RndVertexInterpreter::kNumStreams; ++stream) {
        const auto* desc = (mBufferMask & (1U << stream)) != 0
            ? &mVertexBuffers[stream]
            : &defaults[stream];
        GfxSetVertexBuffers(context, stream, 1, desc);
    }
}
