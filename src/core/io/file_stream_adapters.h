#pragma once

#include <cstdint>

namespace rb4 {

struct StreamChecksum;

// SHA-1 backed stream checksum update at 0x367C50.
void stream_checksum_update(
    StreamChecksum* checksum,
    const void* data,
    std::int64_t size);
// SHA-1 context reset at 0x117B560.
void sha1_context_reset(void* context);

}  // namespace rb4
