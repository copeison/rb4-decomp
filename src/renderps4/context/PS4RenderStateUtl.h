#pragma once

#include "render/platform/orbis/shaders/orbis_shader_state.h"

// Builders that turn engine render-state enums into Gnm hardware state.
//
// Only InitSampler is reconstructed. The map also lists InitBlendControl,
// InitDepthStencilControl, InitPrimitiveSetup, InitStencilControl and
// InitStencilOpControl; their counterparts are still undefined helpers in the
// orbis context state code, whose signatures differ from the map's.
namespace PS4RenderStateUtl {

// The map has InitSampler(sce::Gnm::Sampler&, RndTexWrapMode,
// RndTexFilterMode); the project models the sampler as
// rb4::OrbisSamplerDescriptor, the wrap mode as rb4::OrbisSamplerAddressMode
// and the filter mode as an unsigned int.
void InitSampler(
    rb4::OrbisSamplerDescriptor& sampler,
    rb4::OrbisSamplerAddressMode wrap,
    unsigned int filter);

}  // namespace PS4RenderStateUtl
