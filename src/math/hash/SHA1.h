#pragma once

#include <cstdint>

// 208-byte SHA-1 context. Only the members the recovered code calls are
// declared.
class CSHA1 {
public:
    // The 20-byte hash. The map names the type; the field name is not in
    // the reference map.
    struct Digest {
        std::uint8_t mData[20];
    };

    CSHA1();  // 0x117B510
    ~CSHA1();  // 0x117B560

    void Reset();
    void Update(const void* data, unsigned long size);  // 0x117C640
    // Pads the message and returns the hash, which the context keeps.
    const Digest& Final();  // 0x117CB90

private:
    std::uint8_t mState[208];  // Name not in the reference map.
};

static_assert(sizeof(CSHA1) == 208);
