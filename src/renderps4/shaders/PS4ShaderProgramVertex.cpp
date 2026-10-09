#include "renderps4/shaders/PS4ShaderProgramVertex.h"

#include <algorithm>
#include <cstring>
#include <gnmx/fetchshaderhelper.h>
#include <gnmx/shader_parser.h>

#include "render/shaders/RndShaderCompiler.h"
#include "renderps4/context/PS4Context.h"
#include "renderps4/system/PS4Device.h"

namespace {

constexpr const char* kAllocationName = "VShader";
constexpr int kShaderCodeAlignment = 0x100;
constexpr int kShaderDataAlignment = 4;
// Input semantics below this index come from the per-vertex streams; the
// rest come from the per-instance streams. Name not in the reference map.
constexpr unsigned int kNumVertexSemantics = 8;

}  // namespace

// Reconstructed from eboot.elf at 0x8E46E0.
PS4ShaderProgramVertex::PS4ShaderProgramVertex()
    : mVsShader(nullptr),
      mEsShader(nullptr),
      mVsShaderData(nullptr),
      mEsShaderData(nullptr),
      mShaderCode(nullptr) {}

// Reconstructed from eboot.elf at 0x8E4720. The deleting destructor at 0x8E4750
// releases the program through MemFree.
PS4ShaderProgramVertex::~PS4ShaderProgramVertex() {
    Free();
}

// Reconstructed from eboot.elf at 0x8E4790. The shader binary is read into
// temporary memory and parsed; the shader code goes to the "gpu" heap.
bool PS4ShaderProgramVertex::_CreateImpl(BinStream& stream) {
    RndShaderCompilerBlob blob;
    unsigned int saved;
    MemPushTemp(saved, true, true);
    blob.Load(stream);
    MemPopTemp(saved);

    sce::Gnmx::ShaderInfo info;
    sce::Gnmx::parseShader(&info, blob.mData);

    static long sGpuHeap = MemFindHeap("gpu");
    MemPushHeap(sGpuHeap);
    mShaderCode = MemAlloc(info.m_gpuShaderCodeSize, kAllocationName, kShaderCodeAlignment);
    MemPopHeap();

    const auto vsSize = info.m_vsShader->computeSize();
    const auto esSize = info.m_esShader->computeSize();
    mVsShaderData = MemAlloc(vsSize, kAllocationName, kShaderDataAlignment);
    mEsShaderData = MemAlloc(esSize, kAllocationName, kShaderDataAlignment);
    std::memcpy(mShaderCode, info.m_gpuShaderCode, info.m_gpuShaderCodeSize);
    std::memcpy(mVsShaderData, info.m_vsShader, vsSize);
    std::memcpy(mEsShaderData, info.m_esShader, esSize);

    auto* vs = static_cast<sce::Gnmx::VsShader*>(mVsShaderData);
    mVsShader = vs;
    vs->patchShaderGpuAddress(mShaderCode);
    auto* es = static_cast<sce::Gnmx::EsShader*>(mEsShaderData);
    mEsShader = es;
    es->patchShaderGpuAddress(mShaderCode);

    // Fetch per-vertex semantics by vertex index and the rest by instance.
    unsigned long numVertexSemantics = 0;
    const auto* semantics = vs->getInputSemanticTable();
    for (unsigned int index = 0; index < vs->m_numInputSemantics; ++index) {
        numVertexSemantics += semantics[index].m_semantic < kNumVertexSemantics;
    }
    const unsigned long numElements =
        std::max(vs->m_numInputSemantics, es->m_numInputSemantics);
    MemPushTemp(saved, true, true);
    auto* instancing = new sce::Gnm::FetchShaderInstancingMode[numElements];
    MemPopTemp(saved);
    for (unsigned long index = 0; index < numElements; ++index) {
        instancing[index] = index < numVertexSemantics
            ? sce::Gnm::kFetchShaderUseVertexIndex
            : sce::Gnm::kFetchShaderUseInstanceId;
    }

    static long sFetchHeap = MemFindHeap("gpu");
    MemPushHeap(sFetchHeap);
    mVsFetchShader = MemAlloc(
        sce::Gnmx::computeVsFetchShaderSize(vs), kAllocationName, kShaderDataAlignment);
    mEsFetchShader = MemAlloc(
        sce::Gnmx::computeEsFetchShaderSize(es), kAllocationName, kShaderDataAlignment);
    MemPopHeap();
    sce::Gnmx::generateVsFetchShader(
        mVsFetchShader,
        &mShaderModifier,
        vs,
        instancing,
        static_cast<unsigned int>(numElements));
    sce::Gnmx::generateEsFetchShader(
        mEsFetchShader,
        &mShaderModifier,
        es,
        instancing,
        static_cast<unsigned int>(numElements));
    delete[] instancing;

    blob.Free();
    return true;
}

// Reconstructed from eboot.elf at 0x8E4D80. With a geometry shader active
// the program runs as the ES stage and the VS stage is cleared; otherwise the
// ES stage is cleared.
void PS4ShaderProgramVertex::_SelectImpl(RndContext& context) {
    auto& gfx = static_cast<PS4Context&>(context)._ActiveGfxContext();
    if ((context.mActiveShaderStages & (1U << kShaderProgramGeometry)) != 0) {
        gfx.setVsShader(nullptr, mShaderModifier, nullptr);
        gfx.setEsShader(mEsShader, mShaderModifier, mEsFetchShader);
    } else {
        gfx.setVsShader(mVsShader, mShaderModifier, mVsFetchShader);
        gfx.setEsShader(nullptr, 0, nullptr);
    }
}

// Reconstructed from eboot.elf at 0x8E4ED0. Only the shader pointers are
// cleared.
void PS4ShaderProgramVertex::_FreeImpl() {
    gPS4Device->DeferredDelete(mVsFetchShader);
    gPS4Device->DeferredDelete(mEsFetchShader);
    gPS4Device->DeferredDelete(mVsShaderData);
    gPS4Device->DeferredDelete(mEsShaderData);
    gPS4Device->DeferredDelete(mShaderCode);
    mVsShader = nullptr;
    mEsShader = nullptr;
}

// Reconstructed from eboot.elf at 0x8E4F30.
RndShaderProgramType PS4ShaderProgramVertex::_GetTypeImpl() const {
    return kShaderProgramVertex;
}
