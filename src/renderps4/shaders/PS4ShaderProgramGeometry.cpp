#include "renderps4/shaders/PS4ShaderProgramGeometry.h"

#include <cstring>
#include <gnmx/shader_parser.h>

#include "render/shaders/RndShaderCompiler.h"
#include "renderps4/context/PS4Context.h"
#include "renderps4/system/PS4Device.h"

namespace {

constexpr const char* kAllocationName = "GShader";
constexpr int kShaderCodeAlignment = 0x100;
constexpr int kShaderDataAlignment = 4;

}  // namespace

// Reconstructed from eboot.elf at 0x8E40A0.
PS4ShaderProgramGeometry::PS4ShaderProgramGeometry()
    : mGsShader(nullptr),
      mShaderData(nullptr),
      mGsShaderCode(nullptr),
      mCopyShaderCode(nullptr) {}

// Reconstructed from eboot.elf at 0x8E40D0. The deleting destructor at 0x8E4100
// releases the program through MemFree.
PS4ShaderProgramGeometry::~PS4ShaderProgramGeometry() {
    Free();
}

// Reconstructed from eboot.elf at 0x8E4140. The shader binary is read into
// temporary memory and parsed; both code blocks go to the "gpu" heap.
bool PS4ShaderProgramGeometry::_CreateImpl(BinStream& stream) {
    RndShaderCompilerBlob blob;
    unsigned int saved;
    MemPushTemp(saved, true, true);
    blob.Load(stream);
    MemPopTemp(saved);

    sce::Gnmx::ShaderInfo gsInfo;
    sce::Gnmx::ShaderInfo vsInfo;
    sce::Gnmx::parseGsShader(&gsInfo, &vsInfo, blob.mData);

    static long sGpuHeap = MemFindHeap("gpu");
    MemPushHeap(sGpuHeap);
    mGsShaderCode = MemAlloc(gsInfo.m_gpuShaderCodeSize, kAllocationName, kShaderCodeAlignment);
    mCopyShaderCode = MemAlloc(vsInfo.m_gpuShaderCodeSize, kAllocationName, kShaderCodeAlignment);
    MemPopHeap();

    mShaderData = MemAlloc(gsInfo.m_gsShader->computeSize(), kAllocationName, kShaderDataAlignment);
    std::memcpy(mGsShaderCode, gsInfo.m_gpuShaderCode, gsInfo.m_gpuShaderCodeSize);
    std::memcpy(mCopyShaderCode, vsInfo.m_gpuShaderCode, vsInfo.m_gpuShaderCodeSize);
    std::memcpy(mShaderData, gsInfo.m_gsShader, gsInfo.m_gsShader->computeSize());

    auto* gs = static_cast<sce::Gnmx::GsShader*>(mShaderData);
    mGsShader = gs;
    gs->patchShaderGpuAddresses(mGsShaderCode, mCopyShaderCode);

    blob.Free();
    return true;
}

// Reconstructed from eboot.elf at 0x8E4330.
void PS4ShaderProgramGeometry::_SelectImpl(RndContext& context) {
    static_cast<PS4Context&>(context)._ActiveGfxContext().setGsVsShaders(mGsShader);
}

// Reconstructed from eboot.elf at 0x8E4350. Only the shader pointer is
// cleared.
void PS4ShaderProgramGeometry::_FreeImpl() {
    gPS4Device->DeferredDelete(mShaderData);
    gPS4Device->DeferredDelete(mGsShaderCode);
    gPS4Device->DeferredDelete(mCopyShaderCode);
    mGsShader = nullptr;
}

// Reconstructed from eboot.elf at 0x8E43A0.
RndShaderProgramType PS4ShaderProgramGeometry::_GetTypeImpl() const {
    return kShaderProgramGeometry;
}
