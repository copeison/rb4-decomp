#pragma once

#include <cstddef>

#include "os/memory/PoolAlloc.h"
#include "os/threading/CritSec.h"

// Shared voice pool of the Fusion sampler (audio/FusionVoicePool.o). The
// render targets own one. The class has not been reconstructed; only the
// members AudioRenderTarget calls are declared. The object is 128 bytes.
class FusionVoicePool {
public:
    POOL_OVERLOAD(FusionVoicePool)

    // The map's constructor takes no arguments; this build passes the
    // sample rate.
    explicit FusionVoicePool(float sampleRate);  // 0xA07B0
    ~FusionVoicePool();                          // 0xA08A0

    // Clamped to 1..256; lowers the soft limit to match.
    void SetHardVoiceLimit(unsigned int limit);  // 0xA1010
    // Clamped to the hard limit.
    void SetSoftVoiceLimit(unsigned int limit);  // 0xA0D70

    // Stores the flag at +68. At 0xA1250. Name not in the reference map.
    void SetUnknownFlag(bool flag);
    // Stores the count at +56 and rebuilds what depends on it once the
    // voices exist. At 0xA1260. Name not in the reference map.
    void SetUnknownCount(int count);
    // Creates the voices when their creation is still pending. At 0xA0B00.
    // Name not in the reference map.
    void CreatePendingVoices();

    unsigned char mUnknown0[128];  // Field names are not in the reference map.
};

static_assert(sizeof(FusionVoicePool) == 128);
