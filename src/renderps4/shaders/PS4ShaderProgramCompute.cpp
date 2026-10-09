#include "renderps4/shaders/PS4ShaderProgramCompute.h"

#include <cstring>
#include <gnmx/shader_parser.h>

#include "render/shaders/RndShaderCompiler.h"
#include "renderps4/context/PS4Context.h"
#include "renderps4/system/PS4Device.h"

namespace {

constexpr const char* kAllocationName = "CShader";
constexpr int kShaderCodeAlignment = 0x100;
constexpr int kShaderDataAlignment = 4;

}  // namespace

// Reconstructed from eboot.elf at 0x8E3D20.
PS4ShaderProgramCompute::PS4ShaderProgramCompute() {}

// Reconstructed from eboot.elf at 0x8E3D50. The deleting destructor at 0x8E3D80
// releases the program through MemFree.
PS4ShaderProgramCompute::~PS4ShaderProgramCompute() {
    Free();
}

// Reconstructed from eboot.elf at 0x8E3DC0. The shader binary is read into
// temporary memory and parsed; the shader code goes to the "gpu" heap.
bool PS4ShaderProgramCompute::_CreateImpl(BinStream& stream) {
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

    const auto size = info.m_csShader->computeSize();
    mShaderData = MemAlloc(size, kAllocationName, kShaderDataAlignment);
    std::memcpy(mShaderCode, info.m_gpuShaderCode, info.m_gpuShaderCodeSize);
    std::memcpy(mShaderData, info.m_csShader, size);

    auto* cs = static_cast<sce::Gnmx::CsShader*>(mShaderData);
    mCsShader = cs;
    cs->patchShaderGpuAddress(mShaderCode);

    blob.Free();
    return true;
}

// Reconstructed from eboot.elf at 0x8E3F40. On a compute context the global
// constant buffers are reselected after the shader is bound.
void PS4ShaderProgramCompute::_SelectImpl(RndContext& context) {
    auto& ps4 = static_cast<PS4Context&>(context);
    if (context.mActivePipe == 1) {
        ps4._ActiveComputeContext().setCsShader(mCsShader);
        context._ReselectGlobalCBuffers();
    } else if (context.mActivePipe == 0) {
        ps4._ActiveGfxContext().setCsShader(mCsShader);
    }
}

// Reconstructed from eboot.elf at 0x8E4030. Only the shader pointer is
// cleared.
void PS4ShaderProgramCompute::_FreeImpl() {
    gPS4Device->DeferredDelete(mShaderData);
    gPS4Device->DeferredDelete(mShaderCode);
    mCsShader = nullptr;
}

// Reconstructed from eboot.elf at 0x8E4070.
RndShaderProgramType PS4ShaderProgramCompute::_GetTypeImpl() const {
    return kShaderProgramCompute;
}
