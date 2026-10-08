#include "render/core/textures/render_data_format.h"

#include <array>
#include <cstddef>

namespace rb4 {

namespace {

constexpr RenderDataFormatDescriptor make_format(
    std::uint32_t bit_width,
    std::uint32_t channel_layout,
    std::uint32_t numeric_type,
    std::uint32_t layout,
    std::int32_t variant = 0) {
    return {bit_width, channel_layout, numeric_type, layout, variant};
}

constexpr std::array<std::uint32_t, 31> kVariantBitWidths{
    4, 4, 8, 8, 4, 4, 8, 8, 8, 8, 8, 8, 4, 8, 4, 4,
    8, 8, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0,
};

RenderDataFormatDescriptor make_variant_format(
    std::uint32_t layout,
    std::int32_t variant) {
    const auto index = static_cast<std::size_t>(variant - 1);
    return make_format(
        index < kVariantBitWidths.size() ? kVariantBitWidths[index] : 0,
        static_cast<std::uint32_t>(-1),
        static_cast<std::uint32_t>(-1),
        layout,
        variant);
}

}  // namespace

// Reconstructed from eboot.elf at 0x68DB80.
RenderDataFormatDescriptor render_data_format_describe(
    std::int32_t data_format) {
    switch (data_format) {
    case 0: return make_format(8, 10, 0, 1);
    case 1: return make_format(16, 0, 0, 1);
    case 2: return make_format(24, 2, 0, 1);
    case 3: return make_format(24, 2, 0, 2);
    case 4: return make_format(24, 3, 0, 1);
    case 5: return make_format(24, 3, 0, 2);
    case 6: return make_format(32, 4, 0, 1);
    case 7: return make_format(32, 4, 0, 2);
    case 8: return make_format(32, 5, 0, 1);
    case 9: return make_format(32, 5, 0, 2);
    case 10: return make_format(32, 4, 3, 1);
    case 11: return make_format(32, 6, 0, 1);
    case 12: return make_format(32, 6, 0, 2);
    case 13: return make_format(32, 7, 0, 1);
    case 14: return make_format(32, 7, 0, 2);
    case 15: return make_format(16, 10, 0, 1);
    case 16: return make_format(16, 10, 2, 1);
    case 17: return make_format(32, 0, 0, 1);
    case 18: return make_format(32, 0, 2, 1);
    case 19: return make_format(48, 2, 0, 1);
    case 20: return make_format(64, 4, 0, 1);
    case 21: return make_format(64, 4, 2, 1);
    case 22: return make_format(32, 10, 2, 1);
    case 23: return make_format(64, 0, 2, 1);
    case 24: return make_format(128, 4, 2, 1);
    case 25: return make_format(32, 4, 1, 1);
    case 26: return make_format(32, 2, 2, 1);
    case 53: return make_format(16, 12, 0, 1);
    case 54: return make_format(24, 11, 0, 1);
    case 55: return make_format(32, 11, 0, 1);
    case 56: return make_format(40, 11, 2, 1);
    default:
        break;
    }

    constexpr std::array<std::int32_t, 26> kMiddleVariants{
        1, 1, 2, 2, 3, 3, 4, 4, 5, 6, 7, 8, 9,
        10, 11, 11, 12, 12, 13, 14, 15, 15, 16, 16, 17, 17,
    };
    constexpr std::array<std::uint32_t, 26> kMiddleLayouts{
        1, 2, 1, 2, 1, 2, 1, 2, 1, 1, 1, 1, 1,
        1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 2,
    };
    if (data_format >= 27 && data_format <= 52) {
        const auto index = static_cast<std::size_t>(data_format - 27);
        return make_variant_format(
            kMiddleLayouts[index], kMiddleVariants[index]);
    }
    if (data_format >= 57 && data_format <= 70) {
        return make_variant_format(1, data_format - 39);
    }
    if (data_format >= 71 && data_format <= 84) {
        return make_variant_format(2, data_format - 53);
    }

    return {0, static_cast<std::uint32_t>(-1),
            static_cast<std::uint32_t>(-1),
            static_cast<std::uint32_t>(-1), -1};
}

std::uint32_t render_data_format_bits_per_pixel(
    std::int32_t data_format) {
    return render_data_format_describe(data_format).bit_width;
}

}  // namespace rb4
