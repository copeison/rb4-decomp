#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndContext;
class RndTextureBase;

// Writes the scene mask into the stencil buffer per light tile. It has only a
// vertex program. Not in the reference map; the name is the binary's
// class-name string. The object spans 0x644FC0-0x6452DF; the vtable is at
// 0x192F4D0.
class RndShaderStencilSceneMask : public RndShader {
public:
    RndShaderStencilSceneMask();             // 0x644FC0
    ~RndShaderStencilSceneMask() override;  // 0x645000, 0x645010

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x6452B0
    const char* _GetShaderFilePath() const override; // slot 3 at 0x6451B0
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x6451C0
    int _GetShaderStages() const override;           // slot 6 at 0x6452A0

    // Binds the unrefined scene mask for the vertex program and selects the
    // shader with the viewport's light-tile counts. Name not in the
    // reference map.
    void Select(
        RndContext& context,
        RndTextureBase& unrefinedMask);  // 0x645030

    // Field names are not in the reference map.
    unsigned long mUnrefinedMask;  // Resource index.
    unsigned long mTileCounts;     // Constant offset and the buffer size.
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndShaderStencilSceneMask, mUnrefinedMask) == 288);
static_assert(offsetof(RndShaderStencilSceneMask, mCBufferSize) == 304);
static_assert(sizeof(RndShaderStencilSceneMask) == 312);
