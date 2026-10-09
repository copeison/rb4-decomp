#include "render/shaders/RndShaderError.h"

#include "render/context/RndContext.h"

// Reconstructed from eboot.elf at 0x63E650.
RndShaderError::RndShaderError() : mGeoType{}, mShadingMode{} {}

// Reconstructed from eboot.elf at 0x63E690.
RndShaderError::~RndShaderError() {}

// Reconstructed from eboot.elf at 0x63E830.
const char* RndShaderError::_GetClassNameImpl() const {
    return "RndShaderError";
}

// Reconstructed from eboot.elf at 0x63E730.
const char* RndShaderError::_GetShaderFilePath() const {
    return "../../system/data/shaders/Error.hlsl";
}

// Reconstructed from eboot.elf at 0x63E740.
void RndShaderError::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig&,
    RndShaderResourceConfig&) {
    mGeoType = defines.GetDefines(kShaderProgramVertex)
                   .Add(Symbol("HX_GEO_TYPE"), 0, 2);
    mShadingMode = defines.GetDefines(kShaderProgramPixel)
                       .Add(Symbol("HX_SHADING_MODE"), 0, 19);
}

// Reconstructed from eboot.elf at 0x63E820.
void RndShaderError::_SelectErrorShader(RndContext&) const {}

// Reconstructed from eboot.elf at 0x63E810.
bool RndShaderError::_SupportsRTSlicing() const {
    return true;
}

// Reconstructed from eboot.elf at 0x63E6C0. The geometry type goes in the
// vertex key and the context's shading mode in the pixel key.
void RndShaderError::Select(RndContext& context, RndShaderGeoType geoType) {
    RndShaderKeyGroup keys{};
    keys.mKeys[0] = mGeoType.SetValue(0, static_cast<unsigned int>(geoType));
    keys.mKeys[3] =
        mShadingMode.SetValue(0, static_cast<unsigned int>(context.mShadingMode));
    _SelectShaderCollection(context, keys);
}
