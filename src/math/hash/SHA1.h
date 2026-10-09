#pragma once

#include <cstdint>

// 208-byte SHA-1 context. Only destruction is reached by recovered code.
class CSHA1 {
public:
    CSHA1();
    ~CSHA1();  // 0x117B560

    void Reset();

private:
    std::uint8_t mState[208];  // Name not in the reference map.
};

static_assert(sizeof(CSHA1) == 208);
