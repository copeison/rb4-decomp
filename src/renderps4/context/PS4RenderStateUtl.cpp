#include "renderps4/context/PS4RenderStateUtl.h"

namespace PS4RenderStateUtl {

// Reconstructed from eboot.elf at 0x8EC6A0, called by
// PS4Context::_SetSamplerImpl.
void InitSampler(sce::Gnm::Sampler& sampler, WrapMode wrap, unsigned int filter) {
    sampler = {
        {0, 0x00FFF000, 0x05000000, 0},
    };

    switch (wrap) {
    case WrapMode::kClamp:
        sampler.m_regs[0] = 0x92;
        break;
    case WrapMode::kWrap:
        break;
    case WrapMode::kClampOpaqueBlack:
        sampler.m_regs[0] = 0x1B6;
        sampler.m_regs[3] = 0x40000000;
        break;
    case WrapMode::kClampOpaqueWhite:
        sampler.m_regs[0] = 0x1B6;
        sampler.m_regs[3] = 0x80000000;
        break;
    case WrapMode::kMirror:
        sampler.m_regs[0] = 0x49;
        break;
    }

    switch (filter) {
    case 1:
        sampler.m_regs[2] = 0x05000000;
        break;
    case 2:
        sampler.m_regs[2] = 0x06500000;
        break;
    case 3:
        sampler.m_regs[2] = 0x0A500000;
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        sampler.m_regs[0] |= (filter - 3) << 9;
        sampler.m_regs[2] = 0x0AF00000;
        break;
    default:
        break;
    }
}

// Reconstructed from eboot.elf at 0x8EC310.
void InitBlendControl(sce::Gnm::BlendControl& control, RndBlendMode mode) {
    control.init();
    switch (mode) {
    case RndBlendMode::kSourceAlpha:
        control.setBlendEnable(true);
        control.setColorEquation(
            sce::Gnm::kBlendMultiplierSrcAlpha,
            sce::Gnm::kBlendFuncAdd,
            sce::Gnm::kBlendMultiplierOneMinusSrcAlpha);
        break;
    case RndBlendMode::kSourceAlphaAdd:
    case RndBlendMode::kDecalLitSourceAlpha:
        control.setBlendEnable(true);
        control.setColorEquation(
            sce::Gnm::kBlendMultiplierSrcAlpha, sce::Gnm::kBlendFuncAdd, sce::Gnm::kBlendMultiplierOne);
        break;
    case RndBlendMode::kPremultipliedAlpha:
        control.setBlendEnable(true);
        control.setColorEquation(
            sce::Gnm::kBlendMultiplierOne,
            sce::Gnm::kBlendFuncAdd,
            sce::Gnm::kBlendMultiplierOneMinusSrcAlpha);
        break;
    case RndBlendMode::kScreen:
        control.setBlendEnable(true);
        control.setColorEquation(
            sce::Gnm::kBlendMultiplierOneMinusDestColor,
            sce::Gnm::kBlendFuncAdd,
            sce::Gnm::kBlendMultiplierOne);
        break;
    case RndBlendMode::kDestination:
        control.setBlendEnable(true);
        control.setColorEquation(
            sce::Gnm::kBlendMultiplierZero, sce::Gnm::kBlendFuncAdd, sce::Gnm::kBlendMultiplierOne);
        break;
    case RndBlendMode::kSource:
        control.setBlendEnable(false);
        control.setColorEquation(
            sce::Gnm::kBlendMultiplierOne, sce::Gnm::kBlendFuncAdd, sce::Gnm::kBlendMultiplierZero);
        break;
    case RndBlendMode::kAdd:
        control.setBlendEnable(true);
        control.setColorEquation(
            sce::Gnm::kBlendMultiplierOne, sce::Gnm::kBlendFuncAdd, sce::Gnm::kBlendMultiplierOne);
        break;
    case RndBlendMode::kSubtract:
        control.setBlendEnable(true);
        control.setColorEquation(
            sce::Gnm::kBlendMultiplierOne, sce::Gnm::kBlendFuncSubtract, sce::Gnm::kBlendMultiplierOne);
        break;
    case RndBlendMode::kMultiply:
        control.setBlendEnable(true);
        control.setColorEquation(
            sce::Gnm::kBlendMultiplierDestColor, sce::Gnm::kBlendFuncAdd, sce::Gnm::kBlendMultiplierZero);
        break;
    case RndBlendMode::kLighten:
        control.setBlendEnable(true);
        control.setColorEquation(
            sce::Gnm::kBlendMultiplierOne, sce::Gnm::kBlendFuncMax, sce::Gnm::kBlendMultiplierOne);
        break;
    case RndBlendMode::kDarken:
        control.setBlendEnable(true);
        control.setColorEquation(
            sce::Gnm::kBlendMultiplierOne, sce::Gnm::kBlendFuncMin, sce::Gnm::kBlendMultiplierOne);
        break;
    default:
        break;
    }
}

// Reconstructed from eboot.elf at 0x8EC450. The depth modes test with
// reversed depth (greater-or-equal); the stencil modes pick the stencil test.
void InitDepthStencilControl(
    sce::Gnm::DepthStencilControl& control,
    unsigned int depthMode,
    unsigned int stencilMode) {
    control.init();
    switch (depthMode) {
    case 0:
        control.setDepthEnable(false);
        break;
    case 1:
        control.setDepthEnable(true);
        control.setDepthControl(sce::Gnm::kDepthControlZWriteEnable, sce::Gnm::kCompareFuncGreaterEqual);
        break;
    case 2:
        control.setDepthEnable(true);
        control.setDepthControl(sce::Gnm::kDepthControlZWriteDisable, sce::Gnm::kCompareFuncEqual);
        break;
    case 3:
        control.setDepthEnable(true);
        control.setDepthControl(sce::Gnm::kDepthControlZWriteDisable, sce::Gnm::kCompareFuncGreaterEqual);
        break;
    case 4:
        control.setDepthEnable(true);
        control.setDepthControl(sce::Gnm::kDepthControlZWriteEnable, sce::Gnm::kCompareFuncAlways);
        break;
    default:
        break;
    }
    switch (stencilMode) {
    case 0:
        control.setStencilEnable(false);
        break;
    case 1:
        control.setStencilEnable(true);
        control.setStencilFunction(sce::Gnm::kCompareFuncAlways);
        break;
    case 2:
    case 3:
        control.setStencilEnable(true);
        control.setStencilFunction(sce::Gnm::kCompareFuncEqual);
        break;
    case 4:
        control.setStencilEnable(true);
        control.setStencilFunction(sce::Gnm::kCompareFuncNotEqual);
        break;
    case 5:
        control.setStencilEnable(true);
        control.setStencilFunction(sce::Gnm::kCompareFuncLessEqual);
        break;
    default:
        break;
    }
}

// Reconstructed from eboot.elf at 0x8EC5A0. The reference is both the test
// and the replacement value.
void InitStencilControl(
    sce::Gnm::StencilControl& control,
    unsigned int,
    unsigned char reference,
    unsigned char readMask,
    unsigned char writeMask) {
    control.m_mask = readMask;
    control.m_writeMask = writeMask;
    control.m_testVal = reference;
    control.m_opVal = reference;
}

// Reconstructed from eboot.elf at 0x8EC5B0. Modes 1 and 3 write the
// reference where the stencil and depth tests pass; modes 2, 4 and 5 only
// test.
void InitStencilOpControl(sce::Gnm::StencilOpControl& control, unsigned int stencilMode) {
    control.init();
    switch (stencilMode) {
    case 2:
    case 4:
    case 5:
        control.setStencilOps(sce::Gnm::kStencilOpKeep, sce::Gnm::kStencilOpKeep, sce::Gnm::kStencilOpKeep);
        break;
    case 1:
    case 3:
        control.setStencilOps(
            sce::Gnm::kStencilOpKeep, sce::Gnm::kStencilOpReplaceOp, sce::Gnm::kStencilOpKeep);
        break;
    default:
        break;
    }
}

// Reconstructed from eboot.elf at 0x8EC600.
void InitPrimitiveSetup(
    sce::Gnm::PrimitiveSetup& setup,
    unsigned int frontFace,
    RndCullMode cullMode,
    unsigned int fillMode) {
    setup.init();
    setup.setFrontFace(
        frontFace != 1 ? sce::Gnm::kPrimitiveSetupFrontFaceCw : sce::Gnm::kPrimitiveSetupFrontFaceCcw);
    switch (cullMode) {
    case kCullFront:
        setup.setCullFace(sce::Gnm::kPrimitiveSetupCullFaceFront);
        break;
    case kCullBack:
        setup.setCullFace(sce::Gnm::kPrimitiveSetupCullFaceBack);
        break;
    case kCullNone:
        setup.setCullFace(sce::Gnm::kPrimitiveSetupCullFaceNone);
        break;
    default:
        break;
    }
    if (fillMode == 1) {
        setup.setPolygonMode(sce::Gnm::kPrimitiveSetupPolygonModeFill, sce::Gnm::kPrimitiveSetupPolygonModeFill);
    } else if (fillMode == 0) {
        setup.setPolygonMode(sce::Gnm::kPrimitiveSetupPolygonModeLine, sce::Gnm::kPrimitiveSetupPolygonModeLine);
    }
}

}  // namespace PS4RenderStateUtl
