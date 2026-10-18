#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndContext;
class RndTextureBase;

// Filters a captured light probe into its diffuse or specular cube, one mip
// at a time (render/RndCShaderFilterLightProbe.o, 0x6D6CA0-0x6D712F). The
// vtable is at 0x1938F90. RndLightGlobals owns the instance.
class RndCShaderFilterLightProbe : public RndShaderCompute {
public:
    // Field names are not in the reference map.
    struct Params {
        Params();  // 0x6D70E0

        // HX_PROBE_TEXTURE_DIFFUSE (0) or HX_PROBE_TEXTURE_SPECULAR (1); -1
        // until set.
        int mTextureType;
        // The destination mip's width and height, in texels.
        unsigned int mSize;
        // The destination mip, passed with the destination's bind. Weak
        // evidence: the platform selects of this build ignore it.
        unsigned int mDstMip;
        float mSmoothness;
        RndTextureBase* mSrc;
        RndTextureBase* mHelper;
        RndTextureBase* mDst;
    };

    // The constructor leaves mHelperTexture unset; _InitConfigImpl assigns
    // it.
    RndCShaderFilterLightProbe();  // 0x6D6CA0
    // The complete destructor is folded into RndShaderCompute's.
    ~RndCShaderFilterLightProbe() override;  // 0x6D6D00, 0x6D6D10

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x6D7100
    const char* _GetShaderFilePath() const override; // slot 3 at 0x6D6F30
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x6D6F40

    // Filters the source, with the helper texture, into the destination
    // mip, one thread per destination texel.
    void Dispatch(RndContext& context, Params& params);  // 0x6D6D30

    // Field names are not in the reference map.
    RndShaderDefInfo mProbeTextureType;
    unsigned long mCBufferSize;
    unsigned long mTextureSize;    // Constant offsets.
    unsigned long mSmoothness;
    unsigned long mSrcTexture;     // Resource indices.
    unsigned long mHelperTexture;
    unsigned long mDstTexture;
};

static_assert(offsetof(RndCShaderFilterLightProbe::Params, mSrc) == 16);
static_assert(sizeof(RndCShaderFilterLightProbe::Params) == 40);

static_assert(offsetof(RndCShaderFilterLightProbe, mProbeTextureType) == 288);
static_assert(offsetof(RndCShaderFilterLightProbe, mCBufferSize) == 312);
static_assert(offsetof(RndCShaderFilterLightProbe, mDstTexture) == 352);
static_assert(sizeof(RndCShaderFilterLightProbe) == 360);
