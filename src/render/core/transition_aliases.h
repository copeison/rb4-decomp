#pragma once

// Temporary bridge while the renderer is converted to its original classes.
// Each alias names a reconstructed type that has not been converted yet; it
// is deleted when that type becomes a real class.

class PS4Context;
class RndContext;

namespace rb4 {
struct RenderShaderConstantBlock;
}  // namespace rb4

using RndShaderCBufferConfig = rb4::RenderShaderConstantBlock;
