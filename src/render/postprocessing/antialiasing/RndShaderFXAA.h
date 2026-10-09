#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndTextureBase;

// FXAA antialiasing pass. The vtable is at 0x192ECD8.
class RndShaderFXAA : public RndShader {
public:
    RndShaderFXAA();             // 0x6364F0
    ~RndShaderFXAA() override;   // 0x636540, 0x636550

    const char* _GetClassNameImpl() const override;   // 0x636770
    const char* _GetShaderFilePath() const override;  // 0x6366F0
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // 0x636700

    // 16-byte parameter block for the FXAA draw. Name and field names are
    // not in the reference map.
    struct Params {
        RndTextureBase* mSource;
        // The FXAA stage sets both to 1 (0x63371C); Select never reads
        // them.
        float mReserved8[2];
    };

    // Binds the source, uploads the reciprocal of its size, and selects
    // the programs.
    void Select(RndContext& context, const Params& params);  // 0x636570

    // Field names are not in the reference map.
    unsigned long mTargetDimensionsRcp;  // Constant offset and the buffer size.
    unsigned long mCBufferSize;
    unsigned long mSrcTex;               // Resource index.
};

static_assert(sizeof(RndShaderFXAA::Params) == 16);

static_assert(offsetof(RndShaderFXAA, mTargetDimensionsRcp) == 288);
static_assert(offsetof(RndShaderFXAA, mSrcTex) == 304);
static_assert(sizeof(RndShaderFXAA) == 312);
