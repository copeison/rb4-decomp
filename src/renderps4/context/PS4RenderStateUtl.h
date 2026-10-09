#pragma once

#include "renderps4/system/gnm_adapters.h"

// Builders that turn engine render-state enums into Gnm hardware state.
//
// Only InitSampler is reconstructed. The map also lists InitBlendControl,
// InitDepthStencilControl, InitPrimitiveSetup, InitStencilControl and
// InitStencilOpControl; their counterparts are still undefined helpers in the
// orbis context state code, whose signatures differ from the map's.
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
void InitSampler(GnmSampler& sampler, WrapMode wrap, unsigned int filter);

}  // namespace PS4RenderStateUtl
