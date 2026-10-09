#include "render/postprocessing/tonemap/RndCShaderTonemap.h"

#include "render/context/RndContext.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Thread-group widths, set by a static initializer at 0x6DB050; only the 2D
// width is used. Names not in the reference map.
int gTonemapUnknown = -1;     // 0x1AB1948
int gTonemapGroupSize2D = 8;  // 0x1AB194C
int gTonemapGroupSize3D = 4;  // 0x1AB1950

unsigned int NumGroups(int count, int groupSize) {
    const int groups = count / groupSize;
    return static_cast<unsigned int>(groups + (groups * groupSize < count ? 1 : 0));
}

}  // namespace

// Reconstructed from eboot.elf at 0x6DAF60.
RndCShaderTonemap::RndCShaderTonemap() {}

RndCShaderTonemap::~RndCShaderTonemap() {}

// Reconstructed from eboot.elf at 0x6DAF90.
void RndCShaderTonemap::Dispatch(RndContext& context, const Params& params) {
    _Select(context, params);
    const auto& desc = params.mOutput->mBaseDesc;
    (void)gTonemapUnknown;
    (void)gTonemapGroupSize3D;
    context._DispatchComputeImpl(
        NumGroups(static_cast<int>(desc.mWidth), gTonemapGroupSize2D),
        NumGroups(static_cast<int>(desc.mHeight), gTonemapGroupSize2D),
        1);
}

const char* RndCShaderTonemap::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/TonemapCS.hlsl";
}

const char* RndCShaderTonemap::_GetClassNameImpl() const {
    return "RndCShaderTonemap";
}

int RndCShaderTonemap::_GetShaderStages() const {
    return kShaderStagesCompute;
}
