#pragma once

// Temporary bridge while the renderer is converted to its original classes.
// Each alias names a reconstructed type that has not been converted yet; it
// is deleted when that type becomes a real class.

class PS4Context;
class RndContext;

namespace rb4 {
struct OrbisRenderSystem;
struct RenderShaderConstantBlock;
}  // namespace rb4

using RndShaderCBufferConfig = rb4::RenderShaderConstantBlock;

// The PS4 device is still an opaque layout.
using PS4Device = rb4::OrbisRenderSystem;

// PS4Device::DeferredDelete (0x8D83F0) through the global device.
void PS4DeferredDelete(void* allocation);
