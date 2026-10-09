#pragma once

#include <cstdint>

// IEEE half-precision float, used by the compressed vertex layouts.
class Half {
public:
    // Converts by truncating the mantissa; out-of-range values become
    // infinity, NaN stays NaN.
    void Set(float value);  // 0x1179780

    std::uint16_t mValue;  // Name not in the reference map.
};

static_assert(sizeof(Half) == 2);
