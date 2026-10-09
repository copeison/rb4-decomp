#pragma once

#include <cstdint>

class RndShaderResource;

// Resource transition recorded before a context reuses a resource. Names are
// not in the reference map, which predates resource barriers.
enum class RndResourceBarrierType : std::uint32_t {
    kTransition = 0,
    kAliasing = 1,
    kUnorderedAccess = 2,
};

enum class RndResourceBarrierPhase : std::uint32_t {
    kImmediate = 0,
    kBegin = 1,
    kEnd = 2,
};

enum class RndResourceState : std::uint32_t {
    kRenderTarget = 0x0004,
    kUnorderedAccess = 0x0008,
    kDepthWrite = 0x0010,
    kPixelShaderResource = 0x0080,
    kStreamOutput = 0x0100,
    kCopyDestination = 0x0400,
    kResolveDestination = 0x1000,
};

struct RndResourceBarrier {
    RndResourceBarrierType mType = RndResourceBarrierType::kTransition;
    RndResourceBarrierPhase mPhase = RndResourceBarrierPhase::kImmediate;
    RndShaderResource* mResource = nullptr;
    std::uint64_t mSubresource = 0;  // Mip-major index, or -1 for all.
    RndResourceState mBefore = {};
    RndResourceState mAfter = {};
};

static_assert(sizeof(RndResourceBarrier) == 32);
