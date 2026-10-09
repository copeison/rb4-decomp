#include "renderps4/buffers/PS4ParticleBuffer.h"

#include <cstring>

#include "math/transform/Transform.h"
#include "render/meshes/RndMesh.h"
#include "renderps4/context/PS4Context.h"
#include "renderps4/system/PS4Device.h"
#include "renderps4/system/PS4RenderUtl.h"

namespace {

constexpr unsigned long kVertexBytesPerParticle = 208;
constexpr unsigned long kIndexBytesPerParticle = 12;
constexpr unsigned int kIndicesPerParticle = 6;
constexpr const char* kAllocationName = "ParticleBuffer";
constexpr int kGpuAlignment = 4;

}  // namespace

// Reconstructed from eboot.elf at 0x8E2AA0. The vertex descriptors are
// left uninitialized until _CreateBuffers fills them.
PS4ParticleBuffer::PS4ParticleBuffer(unsigned long numParticles, const char* name)
    : RndParticleBuffer(numParticles, name),
      mVertexStorage{nullptr, nullptr},
      mActiveBank(0),
      mBufferMask(0),
      mUnknown332(0),
      mIndices(nullptr) {
    _CreateBuffers(numParticles);
}

// Inlined into the constructor at 0x8E2AA0. Each particle is a quad of four
// vertices drawn as two triangles. Both vertex banks and the indices are
// allocated from the "gpu" heap.
void PS4ParticleBuffer::_CreateBuffers(unsigned long numParticles) {
    MemFree(mVertexStorage[0]);
    mVertexStorage[0] = nullptr;
    MemFree(mVertexStorage[1]);
    mVertexStorage[1] = nullptr;
    MemFree(mIndices);
    mIndices = nullptr;

    for (unsigned int bank = 0; bank < 2; ++bank) {
        static long sGpuHeap = MemFindHeap("gpu");
        MemPushHeap(sGpuHeap);
        mVertexStorage[bank] = MemAlloc(
            kVertexBytesPerParticle * numParticles, kAllocationName, kGpuAlignment);
        MemPopHeap();
        PS4RenderUtl::InitializeVertexBuffers(
            mVertexBuffers[bank],
            mVertexStorage[bank],
            mBufferMask,
            static_cast<unsigned int>(4 * numParticles),
            *RndVertexInterpreter::GetInstance(kVertexParticle));
    }

    static long sGpuHeap = MemFindHeap("gpu");
    MemPushHeap(sGpuHeap);
    mIndices = static_cast<unsigned short*>(MemAlloc(
        kIndexBytesPerParticle * numParticles, kAllocationName, kGpuAlignment));
    MemPopHeap();

    auto* indices = mIndices;
    for (unsigned long particle = 0; particle < numParticles; ++particle) {
        const auto base = static_cast<unsigned short>(particle * 4);
        *indices++ = base;
        *indices++ = static_cast<unsigned short>(base + 1);
        *indices++ = static_cast<unsigned short>(base + 2);
        *indices++ = base;
        *indices++ = static_cast<unsigned short>(base + 2);
        *indices++ = static_cast<unsigned short>(base + 3);
    }
}

// Reconstructed from eboot.elf at 0x8E2D30. The deleting destructor at
// 0x8E2D80 releases the buffer through MemFree.
PS4ParticleBuffer::~PS4ParticleBuffer() {
    gPS4Device->DeferredDelete(mVertexStorage[0]);
    gPS4Device->DeferredDelete(mVertexStorage[1]);
    gPS4Device->DeferredDelete(mIndices);
}

void PS4ParticleBuffer::_UpdateBuffer(RndContext& context) {
    mActiveBank = (mActiveBank & 1U) == 0 ? 1 : 0;
    _FillVertexBuffer(context, mVertexStorage[mActiveBank]);
}

// Reconstructed from eboot.elf at 0x8E2E10. The particles carry their own
// world positions, so one instance record holds the identity transforms and
// the first instance's parameters. The binary copies mParams[1] from
// Vector4::sZero (0x1B5D268).
void PS4ParticleBuffer::_DrawBatchImpl(
    RndContext& context,
    const VectorAdapter<RndInstanceData>& instances) {
    _UpdateBuffer(context);
    if (mNumActive == 0) {
        return;
    }

    auto& ps4 = static_cast<PS4Context&>(context);
    for (unsigned int stream = 0; stream < RndVertexInterpreter::kNumStreams; ++stream) {
        const auto* buffer = (mBufferMask & (1U << stream)) != 0
            ? &mVertexBuffers[mActiveBank][stream]
            : &gPS4Device->mDefaultVertexDescs[stream];
        ps4._ActiveGfxContext().setVertexBuffers(
            sce::Gnm::kShaderStageVs, stream, 1, buffer);
    }

    const auto& xfm = Transform::sID;
    const auto& normal = Hmx::Matrix3::sID;
    RndInstanceData instance;
    instance.mXfm[0][0] = xfm.m.x.x;
    instance.mXfm[0][1] = xfm.m.y.x;
    instance.mXfm[0][2] = xfm.m.z.x;
    instance.mXfm[0][3] = xfm.v.x;
    instance.mXfm[1][0] = xfm.m.x.y;
    instance.mXfm[1][1] = xfm.m.y.y;
    instance.mXfm[1][2] = xfm.m.z.y;
    instance.mXfm[1][3] = xfm.v.y;
    instance.mXfm[2][0] = xfm.m.x.z;
    instance.mXfm[2][1] = xfm.m.y.z;
    instance.mXfm[2][2] = xfm.m.z.z;
    instance.mXfm[2][3] = xfm.v.z;
    instance.mNormalXfm[0][0] = normal.x.x;
    instance.mNormalXfm[0][1] = normal.y.x;
    instance.mNormalXfm[0][2] = normal.z.x;
    instance.mNormalXfm[1][0] = normal.x.y;
    instance.mNormalXfm[1][1] = normal.y.y;
    instance.mNormalXfm[1][2] = normal.z.y;
    instance.mNormalXfm[2][0] = normal.x.z;
    instance.mNormalXfm[2][1] = normal.y.z;
    instance.mNormalXfm[2][2] = normal.z.z;
    instance.mPackedState = 0;
    std::memcpy(instance.mParams[0], instances.mData[0].mParams[0], sizeof(instance.mParams[0]));
    std::memset(instance.mParams[1], 0, sizeof(instance.mParams[1]));

    auto& gfx = ps4._ActiveGfxContext();
    auto* uploaded = static_cast<RndInstanceData*>(gfx.allocateFromCommandBuffer(
        sizeof(RndInstanceData), sce::Gnm::kEmbeddedDataAlignment4));
    sce::Gnm::Buffer buffers[PS4RenderUtl::kNumInstanceStreams];
    PS4RenderUtl::InitializeInstanceBuffer(buffers, uploaded, 1);
    *uploaded = instance;
    gfx.setVertexBuffers(
        sce::Gnm::kShaderStageVs,
        RndVertexInterpreter::kNumStreams,
        PS4RenderUtl::kNumInstanceStreams,
        buffers);

    ps4.SetupDraw(RndPrimitive::kTriangles);
    const auto numIndices = static_cast<unsigned int>(kIndicesPerParticle * mNumActive);
    gfx.setIndexSize(sce::Gnm::kIndexSize16);
    gfx.drawIndex(numIndices, mIndices);
    _UpdateStats(context);
}
