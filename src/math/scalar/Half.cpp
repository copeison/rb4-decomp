#include "math/scalar/Half.h"

#include <cstring>

// Reconstructed from eboot.elf at 0x1179780. The stores for the saturating
// cases fall through to the general store, as in the binary.
void Half::Set(float value) {
    int bits;
    std::memcpy(&bits, &value, sizeof(bits));

    int sign = (bits >> 16) & 0x8000;
    int exponent = ((bits >> 23) & 0xFF) - (127 - 15);
    int mantissa = bits & 0x7FFFFF;

    if (exponent <= 0) {
        if (exponent < -10) {
            mValue = static_cast<std::uint16_t>(sign);
        }
        mantissa = static_cast<int>(
            static_cast<unsigned int>(mantissa | 0x800000) >> (1 - exponent));
        mValue = static_cast<std::uint16_t>(sign | (mantissa >> 13));
    } else if (exponent == 0xFF - (127 - 15)) {
        if (mantissa == 0) {
            mValue = static_cast<std::uint16_t>(sign | 0x7C00);
        } else {
            mantissa >>= 13;
            mValue = static_cast<std::uint16_t>(
                sign | 0x7C00 | mantissa | (mantissa == 0));
        }
    } else {
        if (exponent > 30) {
            mValue = static_cast<std::uint16_t>(sign | 0x7C00);
        }
        mValue = static_cast<std::uint16_t>(
            sign | (exponent << 10) | (mantissa >> 13));
    }
}

// Reconstructed from eboot.elf at 0x1179810.
float Half::ToFloat() const {
    const unsigned int bits = mValue;
    const unsigned int sign = (bits >> 15) << 31;
    const unsigned int exponent = (bits >> 10) & 0x1F;
    unsigned int mantissa = bits & 0x3FF;

    unsigned int result;
    if (exponent == 0x1F) {
        result = mantissa != 0 ? (bits << 13) | sign | 0x7F800000 : sign | 0x7F800000;
    } else if (exponent == 0) {
        if (mantissa == 0) {
            result = sign;
        } else {
            int denormalExponent = 1;
            do {
                mantissa <<= 1;
                --denormalExponent;
            } while ((mantissa & 0x400) == 0);
            mantissa &= 0x3FF;
            result = sign | (mantissa << 13) |
                (static_cast<unsigned int>(denormalExponent + (127 - 15)) << 23);
        }
    } else {
        result = sign | (mantissa << 13) | ((exponent + (127 - 15)) << 23);
    }

    float value;
    std::memcpy(&value, &result, sizeof(value));
    return value;
}
