#pragma once

#include <gnm/regs.h>
#include <gnm/sampler.h>

#include "render/context/RndContext.h"
#include "render/materials/RndMaterialCom.h"

// Builders that turn engine render-state enums into Gnm hardware state.
namespace PS4RenderStateUtl {

// The engine's texture wrap modes as InitSampler reads them. Stand-in for
// RndTexWrapMode, whose enumerators are not recovered. Name not in the
// reference map.
enum class WrapMode : unsigned int {
    kClamp = 1,
    kWrap = 2,
    kClampOpaqueBlack = 3,
    kClampOpaqueWhite = 4,
    kMirror = 5,
};

// The map has InitSampler(sce::Gnm::Sampler&, RndTexWrapMode,
// RndTexFilterMode); the project models the filter mode as an unsigned int.
void InitSampler(sce::Gnm::Sampler& sampler, WrapMode wrap, unsigned int filter);

// Modes past kDecalLitSourceAlpha leave the control initialized only.
void InitBlendControl(sce::Gnm::BlendControl& control, RndBlendMode mode);  // 0x8EC310
// The map types the modes as RndDepthMode and RndStencilMode; this build
// passes their values.
void InitDepthStencilControl(
    sce::Gnm::DepthStencilControl& control,
    unsigned int depthMode,
    unsigned int stencilMode);  // 0x8EC450
// The map has InitStencilControl(StencilControl&, RndStencilMode, unsigned
// char); this build also passes the read and write masks and ignores the
// mode.
void InitStencilControl(
    sce::Gnm::StencilControl& control,
    unsigned int stencilMode,
    unsigned char reference,
    unsigned char readMask,
    unsigned char writeMask);  // 0x8EC5A0
void InitStencilOpControl(sce::Gnm::StencilOpControl& control, unsigned int stencilMode);  // 0x8EC5B0
// The map types the front face and fill mode as RndFrontFace and
// RndFillMode; this build passes their values (front face 1 is
// counter-clockwise, fill mode 1 is solid).
void InitPrimitiveSetup(
    sce::Gnm::PrimitiveSetup& setup,
    unsigned int frontFace,
    RndCullMode cullMode,
    unsigned int fillMode);  // 0x8EC600

}  // namespace PS4RenderStateUtl
