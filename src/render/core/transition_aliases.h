#pragma once

// Temporary bridge while the renderer is converted to its original classes.
// Each alias names a reconstructed type that has not been converted yet; it
// is deleted when that type becomes a real class.

namespace rb4 {
struct OrbisRenderContext;
struct OrbisRenderSystem;
struct RenderContext;
struct RenderShaderConstantBlock;
}  // namespace rb4

using RndContext = rb4::RenderContext;
using RndShaderCBufferConfig = rb4::RenderShaderConstantBlock;

// The PS4 context and device are still opaque layouts. PS4 classes reach them
// through these aliases and AsPS4Context.
using PS4Context = rb4::OrbisRenderContext;
using PS4Device = rb4::OrbisRenderSystem;

inline PS4Context& AsPS4Context(RndContext& context) {
    return reinterpret_cast<PS4Context&>(context);
}

// PS4Device::DeferredDelete (0x8D83F0) through the global device.
void PS4DeferredDelete(void* allocation);
