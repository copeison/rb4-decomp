#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Draws one renderer buffer for RndBufferInspection. The vtable is at
// 0x1937080. The constructor, the configuration and Select are not
// reconstructed yet; the members are only sized. Field names are not in the
// reference map.
class RndBufferInspectionShader : public RndShader {
public:
    RndBufferInspectionShader();             // 0x6B5FA0
    ~RndBufferInspectionShader() override;   // 0x6B6020, 0x6B6030

    const char* _GetClassNameImpl() const override;   // 0x6B7280
    const char* _GetShaderFilePath() const override;  // 0x6B6850
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // 0x6B6860

    // Built by the constructor at 0x63C250: sixteen zero bytes and a flag.
    unsigned char mUnknown288[24];
    // Constant offsets and resource indices, all -1 until configured.
    unsigned long mUnknown312[8];
    unsigned long mUnknown376;  // Not set by the constructor.
    unsigned long mUnknown384;  // Zero.
    unsigned long mUnknown392[14];
};

static_assert(offsetof(RndBufferInspectionShader, mUnknown288) == 288);
static_assert(offsetof(RndBufferInspectionShader, mUnknown312) == 312);
static_assert(offsetof(RndBufferInspectionShader, mUnknown384) == 384);
static_assert(offsetof(RndBufferInspectionShader, mUnknown392) == 392);
static_assert(sizeof(RndBufferInspectionShader) == 504);
