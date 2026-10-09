#include "render/core/textures/render_data_format.h"

#include <array>
#include <cstddef>

#include "render/system/RndDevice.h"

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

bool data_format_supported(
    std::int32_t data_format,
    std::uint32_t resource_class) {
    if (data_format < 0) {
        return false;
    }
    // The original indexes the device's platform configurations by the
    // resource class and reads their capability masks.
    const auto* supported_words =
        TheRndDevice()->mPlatformConfigs[resource_class].capability_mask;
    const auto format = static_cast<std::uint32_t>(data_format);
    return (supported_words[format >> 6] &
            (std::uint64_t{1} << (format & 63))) != 0;
}

std::uint32_t channel_divisor(std::uint32_t channel_layout) {
    constexpr std::array<std::uint32_t, 12> kDivisors{
        2, 2, 3, 3, 4, 3, 4, 3, 4, 3, 1, 2,
    };
    return channel_layout < kDivisors.size()
        ? kDivisors[channel_layout]
        : 4;
}

bool try_supported_format(
    const RenderDataFormatDescriptor& descriptor,
    std::uint32_t resource_class,
    std::int32_t& result) {
    result = render_data_format_find_exact(descriptor);
    return data_format_supported(result, resource_class);
}

bool try_channel_fallback(
    RenderDataFormatDescriptor& descriptor,
    std::uint32_t resource_class,
    std::int32_t& result) {
    constexpr std::array<std::uint32_t, 6> kFallbacks{
        3, 2, 6, 7, 4, 5,
    };
    if (descriptor.channel_layout < 2 || descriptor.channel_layout > 7) {
        return false;
    }
    descriptor.channel_layout =
        kFallbacks[descriptor.channel_layout - 2];
    return try_supported_format(descriptor, resource_class, result);
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

// Reconstructed from eboot.elf at 0x68E070.
std::int32_t render_data_format_find_exact(
    const RenderDataFormatDescriptor& descriptor) {
    constexpr std::int32_t kInvalidFormat = -1;
    if (descriptor.layout < 1) {
        return kInvalidFormat;
    }

    if (descriptor.variant >= 1) {
        if (descriptor.variant <= 4) {
            return 27 + (descriptor.variant - 1) * 2 +
                (descriptor.layout == 1 ? 0 : 1);
        }
        if (descriptor.variant >= 5 && descriptor.variant <= 10) {
            return descriptor.layout == 1
                ? 30 + descriptor.variant
                : kInvalidFormat;
        }
        if (descriptor.variant == 11 || descriptor.variant == 12) {
            return 19 + descriptor.variant * 2 +
                (descriptor.layout == 1 ? 0 : 1);
        }
        if (descriptor.variant == 13 || descriptor.variant == 14) {
            return descriptor.layout == 1
                ? 32 + descriptor.variant
                : kInvalidFormat;
        }
        if (descriptor.variant >= 15 && descriptor.variant <= 17) {
            return 17 + descriptor.variant * 2 +
                (descriptor.layout == 1 ? 0 : 1);
        }
        if (descriptor.variant >= 18 && descriptor.variant <= 31) {
            return descriptor.variant +
                (descriptor.layout == 1 ? 39 : 53);
        }
        return kInvalidFormat;
    }

    if (descriptor.bit_width == 0 ||
        descriptor.channel_layout == static_cast<std::uint32_t>(-1) ||
        descriptor.numeric_type == static_cast<std::uint32_t>(-1)) {
        return kInvalidFormat;
    }

    const auto layout_one = descriptor.layout == 1;
    switch (descriptor.numeric_type) {
    case 0:
        switch (descriptor.channel_layout) {
        case 0:
            if (!layout_one) return kInvalidFormat;
            if (descriptor.bit_width == 16) return 1;
            if (descriptor.bit_width == 32) return 17;
            return kInvalidFormat;
        case 2:
            if (descriptor.bit_width == 24) return layout_one ? 2 : 3;
            return layout_one && descriptor.bit_width == 48
                ? 19 : kInvalidFormat;
        case 3:
            return descriptor.bit_width == 24
                ? (layout_one ? 4 : 5) : kInvalidFormat;
        case 4:
            if (descriptor.bit_width == 32) return layout_one ? 6 : 7;
            return layout_one && descriptor.bit_width == 64
                ? 20 : kInvalidFormat;
        case 5:
            return descriptor.bit_width == 32
                ? (layout_one ? 8 : 9) : kInvalidFormat;
        case 6:
            return descriptor.bit_width == 32
                ? (layout_one ? 11 : 12) : kInvalidFormat;
        case 7:
            return descriptor.bit_width == 32
                ? (layout_one ? 13 : 14) : kInvalidFormat;
        case 10:
            if (!layout_one) return kInvalidFormat;
            if (descriptor.bit_width == 8) return 0;
            if (descriptor.bit_width == 16) return 15;
            return kInvalidFormat;
        case 11:
            if (!layout_one) return kInvalidFormat;
            if (descriptor.bit_width == 24) return 54;
            if (descriptor.bit_width == 32) return 55;
            return kInvalidFormat;
        case 12:
            return layout_one && descriptor.bit_width == 16
                ? 53 : kInvalidFormat;
        default:
            return kInvalidFormat;
        }
    case 1:
        return layout_one && descriptor.channel_layout == 4 &&
                descriptor.bit_width == 32
            ? 25 : kInvalidFormat;
    case 2:
        if (!layout_one) return kInvalidFormat;
        switch (descriptor.channel_layout) {
        case 0:
            if (descriptor.bit_width == 32) return 18;
            if (descriptor.bit_width == 64) return 23;
            return kInvalidFormat;
        case 2:
            return descriptor.bit_width == 32 ? 26 : kInvalidFormat;
        case 4:
            if (descriptor.bit_width == 64) return 21;
            if (descriptor.bit_width == 128) return 24;
            return kInvalidFormat;
        case 10:
            if (descriptor.bit_width == 16) return 16;
            if (descriptor.bit_width == 32) return 22;
            return kInvalidFormat;
        case 11:
            return descriptor.bit_width == 40 ? 56 : kInvalidFormat;
        default:
            return kInvalidFormat;
        }
    case 3:
        return layout_one && descriptor.channel_layout == 4 &&
                descriptor.bit_width == 32
            ? 10 : kInvalidFormat;
    default:
        return kInvalidFormat;
    }
}

// Reconstructed from eboot.elf at 0x68E4D0 and 0x68E550.
std::int32_t render_data_format_resolve(
    const RenderDataFormatDescriptor& descriptor,
    std::uint32_t resource_class) {
    std::int32_t result = -1;
    if (try_supported_format(descriptor, resource_class, result)) {
        return result;
    }
    const auto exact_format = result;

    if (descriptor.variant < -1 || descriptor.variant >= 1) {
        return resource_class == 8 ? 21 : -1;
    }

    auto working = descriptor;
    const auto divisor = channel_divisor(working.channel_layout);
    if (working.channel_layout == 2 || working.channel_layout == 3) {
        working.bit_width = 4 * (working.bit_width / divisor);
        working.channel_layout = working.channel_layout == 2 ? 5 : 7;
        if (try_supported_format(working, resource_class, result)) {
            return result;
        }
    }
    if (try_channel_fallback(working, resource_class, result)) {
        return result;
    }

    working = descriptor;
    if (working.channel_layout == 2 || working.channel_layout == 3) {
        working.bit_width = 4 * (
            working.bit_width / channel_divisor(working.channel_layout));
        working.channel_layout = working.channel_layout == 2 ? 4 : 6;
        if (try_supported_format(working, resource_class, result)) {
            return result;
        }
    }
    if (try_channel_fallback(working, resource_class, result)) {
        return result;
    }

    if (resource_class == 8 && descriptor.numeric_type != 2) {
        working = descriptor;
        working.numeric_type = 2;
        if (try_supported_format(working, resource_class, result)) {
            return result;
        }

        const auto numeric_divisor =
            channel_divisor(working.channel_layout);
        working.bit_width = 4 * (working.bit_width / numeric_divisor);
        if (working.channel_layout == 2) {
            working.channel_layout = 4;
        } else if (working.channel_layout == 3) {
            working.channel_layout = 6;
        }
        if (try_supported_format(working, resource_class, result)) {
            return result;
        }
    }

    if (descriptor.channel_layout == 11) {
        constexpr std::array<std::int32_t, 3> kPackedFormats{54, 55, 56};
        std::int32_t exact_index = exact_format - kPackedFormats.front();
        if (exact_index < 0 || exact_index >=
                static_cast<std::int32_t>(kPackedFormats.size())) {
            exact_index = -1;
        }
        for (std::int32_t index = exact_index + 1;
             index < static_cast<std::int32_t>(kPackedFormats.size());
             ++index) {
            if (data_format_supported(kPackedFormats[index], resource_class)) {
                return kPackedFormats[index];
            }
        }
        for (auto index = exact_index; index > 0; --index) {
            if (data_format_supported(kPackedFormats[index - 1], resource_class)) {
                return kPackedFormats[index - 1];
            }
        }
    }

    return -1;
}

}  // namespace rb4
