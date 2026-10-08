#include "render/core/debug/render_gpu_stat_block.h"

#include <cstdint>

namespace rb4 {

namespace {

constexpr std::size_t kRenderGpuStatBlockOffset = 3584;

}  // namespace

RenderGpuStatBlock& render_system_gpu_stat_block(RenderSystem& system) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&system);
    return *reinterpret_cast<RenderGpuStatBlock*>(
        bytes + kRenderGpuStatBlockOffset);
}

}  // namespace rb4
