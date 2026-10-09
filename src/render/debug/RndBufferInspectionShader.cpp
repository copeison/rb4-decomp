#include "render/debug/RndBufferInspectionShader.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x6B5FA0.
RndBufferInspectionShader::RndBufferInspectionShader()
    : mTexMode{},
      mBufferDisplayMode(-1),
      mAuxDims(-1),
      mTextureArrayParams(-1),
      mTint(-1),
      mDiscardPadPixels(-1),
      mBufferInspectionDepthFrac(-1),
      mStencilParams(-1),
      mLightTileCounts(-1),
      mCBufferSize(0),
      mTexture1D(-1),
      mTexture2D(-1),
      mTexture3D(-1),
      mTextureArray1D(-1),
      mTextureArray2D(-1),
      mTextureCube(-1),
      mTextureArrayCube(-1),
      mTexture2DRTSliced(-1),
      mTextureArray2DRTSliced(-1),
      mStencilBuffer(-1),
      mTexture2DNoSampler(-1),
      mComputeBuffer2D(-1),
      mLightIds(-1),
      mLightIdRanges(-1) {}

// Reconstructed from eboot.elf at 0x6B6020.
RndBufferInspectionShader::~RndBufferInspectionShader() {}

// Reconstructed from eboot.elf at 0x6B7280.
const char* RndBufferInspectionShader::_GetClassNameImpl() const {
    return "RndBufferInspectionShader";
}

// Reconstructed from eboot.elf at 0x6B6850.
const char* RndBufferInspectionShader::_GetShaderFilePath() const {
    return "../../system/data/shaders/BufferInspection.hlsl";
}

// Reconstructed from eboot.elf at 0x6B6860. The tile sizes come from the
// device's settings, which must exist.
void RndBufferInspectionShader::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    const RndConfig* settings = TheRndDevice()->mSettings;
    fixedDefines.Add(Symbol("HX_LIGHT_TILE_SIZE"), static_cast<int>(settings->mLightTileSize));
    fixedDefines.Add(
        Symbol("HX_LIGHT_TILE_DEPTH_SLICES"),
        static_cast<int>(settings->mLightTileDepthSlices));

    mTexMode = defines.GetDefines(kShaderProgramPixel).Add(Symbol("HX_BUFFER_TEXMODE"), 0, 24);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_1D"), 0);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_2D"), 1);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_3D"), 2);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_ARRAY1D_ELEM"), 3);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_ARRAY1D_ALL"), 4);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_ARRAY2D_ELEM"), 5);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_ARRAY2D_ALL"), 6);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_CUBE"), 7);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_ARRAYCUBE_ALL"), 8);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_2D_NOSAMPLER"), 9);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_2D_RTSLICED"), 10);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_ARRAY2D_RTSLICED"), 11);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_DEPTH"), 12);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_DEPTH_RTSLICED"), 13);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_DEPTH_ARRAY_ELEM"), 14);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_STENCIL"), 15);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_LIGHT_TILE_COORD"), 16);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_TILED_LIGHTS_INTERP_DIFFUSE"), 17);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_TILED_LIGHTS_INTERP_SPECULAR"), 18);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_COMPUTE_2D_FLOAT4"), 19);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_TILED_LIGHTING_OVERDRAW"), 20);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_DEPTH_TILED_LIGHTING_OVERDRAW"), 21);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_BOTHEYES_TILED_LIGHTING_OVERDRAW"), 22);
    fixedDefines.Add(Symbol("HX_BUFFER_TEXMODE_BOTHEYES_DEPTH_TILED_LIGHTING_OVERDRAW"), 23);

    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_RGBA"), 0);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_RGBX"), 1);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_RED"), 2);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_GREEN"), 3);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_BLUE"), 4);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_ALPHA"), 5);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_NORMAL_MAP"), 6);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_DEPTH"), 7);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_STENCIL"), 8);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_GBUFFER_PIXELNORMAL"), 9);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_GBUFFER_VERTEXNORMAL"), 10);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_FUNCTIONTABLE"), 11);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_TILED_LIGHTS_OVERDRAW"), 12);
    fixedDefines.Add(Symbol("HX_BUFFER_DISPMODE_TILED_LIGHT_PROBES_OVERDRAW"), 13);

    mBufferDisplayMode = cbuffer.AddConstant(kShaderNumericFloat, "gBufferDisplayMode");
    mAuxDims = cbuffer.AddConstant(kShaderNumericFloat2, "gAuxDims");
    mTextureArrayParams = cbuffer.AddConstant(kShaderNumericFloat3, "gTextureArrayParams");
    mTint = cbuffer.AddConstant(kShaderNumericFloat4, "gTint");
    mDiscardPadPixels = cbuffer.AddConstant(kShaderNumericFloat, "gDiscardPadPixels");
    mBufferInspectionDepthFrac =
        cbuffer.AddConstant(kShaderNumericFloat, "gBufferInspectionDepthFrac");
    mStencilParams = cbuffer.AddConstant(kShaderNumericFloat2, "gStencilParams");
    mLightTileCounts = cbuffer.AddConstant(kShaderNumericFloat2, "gLightTileCounts");
    mMaxOverdraw = cbuffer.AddConstant(kShaderNumericFloat2, "gMaxOverdraw");
    mCBufferSize = cbuffer.mSize;

    mTexture1D = resources.AddTexture(
        "gTexture1D",
        "gTex1DSampler",
        RndTextureBase::kTexture1D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTexture2D = resources.AddTexture(
        "gTexture2D",
        "gTex2DSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTexture3D = resources.AddTexture(
        "gTexture3D",
        "gTex3DSampler",
        RndTextureBase::kTexture3D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTextureArray1D = resources.AddTexture(
        "gTextureArray1D",
        "gTexArray1DSampler",
        RndTextureBase::kTextureArray1D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTextureArray2D = resources.AddTexture(
        "gTextureArray2D",
        "gTexArray2DSampler",
        RndTextureBase::kTextureArray2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTextureCube = resources.AddTexture(
        "gTextureCube",
        "gTexCubeSampler",
        RndTextureBase::kTextureCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTextureArrayCube = resources.AddTexture(
        "gTextureArrayCube",
        "gTexArrayCubeSampler",
        RndTextureBase::kTextureArrayCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTexture2DRTSliced = resources.AddTexture2DRTSliced(
        "gTexture2DRTSliced",
        "gTex2DRTSlicedSampler",
        RndTextureBase::kTexture2D,
        kShaderNumericFloat4);
    mTextureArray2DRTSliced = resources.AddTexture2DRTSliced(
        "gTextureArray2DRTSliced",
        "gTexArray2DRTSlicedSampler",
        RndTextureBase::kTextureArray2D,
        kShaderNumericFloat4);
    mStencilBuffer = resources.AddTexture(
        "gStencilBuffer",
        "gStencilBufferSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericUInt4);
    mTexture2DNoSampler = resources.AddTexture(
        "gTexture2DNoSampler",
        nullptr,
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mComputeBuffer2D = resources.AddComputeBuffer(
        "gComputeBuffer2D", 0, kShaderNumericFloat4, kShaderProgramPixel);
    mLightIds = resources.AddComputeBufferCustomTyped(
        "gLightIds", "Uint2As32", 0, kShaderProgramPixel);
    mLightIdRanges = resources.AddComputeBufferCustomTyped(
        "gLightIdRanges", "CSLightIdRange", 0, kShaderProgramPixel);
}
