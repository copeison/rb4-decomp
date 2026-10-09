#include "renderps4/shaders/PS4ShaderProgramPixel.h"

#include <cstring>
#include <gnmx/shader_parser.h>

#include "render/shaders/RndShaderCompiler.h"
#include "renderps4/context/PS4Context.h"
#include "renderps4/system/PS4Device.h"

namespace {

constexpr const char* kAllocationName = "PShader";
constexpr int kShaderCodeAlignment = 0x100;
constexpr int kShaderDataAlignment = 4;

}  // namespace

// Reconstructed from eboot.elf at 0x8E43D0.
PS4ShaderProgramPixel::PS4ShaderProgramPixel()
    : mPsShader(nullptr),
      mShaderData(nullptr),
      mShaderCode(nullptr) {}

// Reconstructed from eboot.elf at 0x8E4410. The deleting destructor at 0x8E4440
// releases the program through MemFree.
PS4ShaderProgramPixel::~PS4ShaderProgramPixel() {
    Free();
}

// Reconstructed from eboot.elf at 0x8E4480. The shader binary is read into
// temporary memory and parsed; the shader code goes to the "gpu" heap.
bool PS4ShaderProgramPixel::_CreateImpl(BinStream& stream) {
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

    const auto size = info.m_psShader->computeSize();
    mShaderData = MemAlloc(size, kAllocationName, kShaderDataAlignment);
    std::memcpy(mShaderCode, info.m_gpuShaderCode, info.m_gpuShaderCodeSize);
    std::memcpy(mShaderData, info.m_psShader, size);

    auto* ps = static_cast<sce::Gnmx::PsShader*>(mShaderData);
    mPsShader = ps;
    ps->patchShaderGpuAddress(mShaderCode);

    blob.Free();
    return true;
}

// Reconstructed from eboot.elf at 0x8E4600. Binding a pixel shader turns
// color-buffer writes back on.
void PS4ShaderProgramPixel::_SelectImpl(RndContext& context) {
    auto& ps4 = static_cast<PS4Context&>(context);
    ps4._ActiveGfxContext().setPsShader(mPsShader);
    ps4.SetCbEnabled(true);
}

// Reconstructed from eboot.elf at 0x8E4670. Only the shader pointer is
// cleared.
void PS4ShaderProgramPixel::_FreeImpl() {
    gPS4Device->DeferredDelete(mShaderData);
    gPS4Device->DeferredDelete(mShaderCode);
    mPsShader = nullptr;
}

// Reconstructed from eboot.elf at 0x8E46B0.
RndShaderProgramType PS4ShaderProgramPixel::_GetTypeImpl() const {
    return kShaderProgramPixel;
}
