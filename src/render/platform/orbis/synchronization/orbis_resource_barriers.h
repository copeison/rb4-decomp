#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct OrbisRenderContext;

enum class RenderResourceBarrierType : std::uint32_t {
    kTransition = 0,
    kAliasing = 1,
    kUnorderedAccess = 2,
};

enum class RenderResourceBarrierPhase : std::uint32_t {
    kImmediate = 0,
    kBegin = 1,
    kEnd = 2,
};

enum class RenderResourceState : std::uint32_t {
    kRenderTarget = 0x0004,
    kUnorderedAccess = 0x0008,
    kDepthWrite = 0x0010,
    kStreamOutput = 0x0100,
    kCopyDestination = 0x0400,
    kResolveDestination = 0x1000,
};

struct RenderResourceBarrier {
    RenderResourceBarrierType type =
        RenderResourceBarrierType::kTransition;
    RenderResourceBarrierPhase phase =
        RenderResourceBarrierPhase::kImmediate;
    void* resource = nullptr;
    std::uint64_t subresource = 0;
    RenderResourceState state_before = {};
    RenderResourceState state_after = {};
};

static_assert(sizeof(RenderResourceBarrier) == 32);

void orbis_render_context_resource_barriers(
    OrbisRenderContext& context,
    std::size_t barrier_count,
    const RenderResourceBarrier* barriers);

}  // namespace rb4
