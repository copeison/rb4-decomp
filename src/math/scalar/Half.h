#pragma once

#include <cstdint>

// IEEE half-precision float, used by the compressed vertex layouts.
class Half {
public:
    // Converts by truncating the mantissa. NaN stays NaN and infinity stays
    // infinity. Overflowing and fully underflowing values do not saturate:
    // the binary stores infinity or the sign and then overwrites it with the
    // truncated bits.
    void Set(float value);  // 0x1179780

    std::uint16_t mValue;  // Name not in the reference map.
};

static_assert(sizeof(Half) == 2);
