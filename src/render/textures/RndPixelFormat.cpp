#include "render/textures/RndPixelFormat.h"

#include <array>
#include <cstddef>
#include <cstdint>

#include "render/system/RndDevice.h"

namespace {

constexpr RndDataFormatInfo make_format(
    std::uint32_t bitsPerPixel,
    std::uint32_t order,
    std::uint32_t storage,
    std::uint32_t gamma,
    std::int32_t compression = 0) {
    return {bitsPerPixel, order, storage, gamma, compression};
}

constexpr std::array<std::uint32_t, 31> kVariantBitWidths{
    4, 4, 8, 8, 4, 4, 8, 8, 8, 8, 8, 8, 4, 8, 4, 4,
    8, 8, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0,
};

RndDataFormatInfo make_variant_format(
    std::uint32_t gamma,
    std::int32_t compression) {
    const auto index = static_cast<std::size_t>(compression - 1);
    return make_format(
        index < kVariantBitWidths.size() ? kVariantBitWidths[index] : 0,
        static_cast<std::uint32_t>(-1),
        static_cast<std::uint32_t>(-1),
        gamma,
        compression);
}

bool data_format_supported(
    int dataFormat,
    HxPlatform platform) {
    if (dataFormat < 0) {
        return false;
    }
    // Each format has one bit in the platform's capability mask.
    const auto* supported_words =
        TheRndDevice()->mCapabilities[platform].mCapabilityMask;
    const auto format = static_cast<std::uint32_t>(dataFormat);
    return (supported_words[format >> 6] &
            (std::uint64_t{1} << (format & 63))) != 0;
}

std::uint32_t channel_divisor(std::uint32_t order) {
    constexpr std::array<std::uint32_t, 12> kDivisors{
        2, 2, 3, 3, 4, 3, 4, 3, 4, 3, 1, 2,
    };
    return order < kDivisors.size()
        ? kDivisors[order]
        : 4;
}

bool try_supported_format(
    const RndDataFormatInfo& descriptor,
    HxPlatform platform,
    std::int32_t& result) {
    result = RndFindDataFormat(descriptor);
    return data_format_supported(result, platform);
}

bool try_channel_fallback(
    RndDataFormatInfo& descriptor,
    HxPlatform platform,
    std::int32_t& result) {
    constexpr std::array<std::uint32_t, 6> kFallbacks{
        3, 2, 6, 7, 4, 5,
    };
    if (descriptor.mOrder < 2 || descriptor.mOrder > 7) {
        return false;
    }
    descriptor.mOrder =
        kFallbacks[descriptor.mOrder - 2];
    return try_supported_format(descriptor, platform, result);
}

}  // namespace

// Reconstructed from eboot.elf at 0x68DB80.
RndDataFormatInfo RndGetDataFormatInfo(
    int dataFormat) {
    switch (dataFormat) {
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
    if (dataFormat >= 27 && dataFormat <= 52) {
        const auto index = static_cast<std::size_t>(dataFormat - 27);
        return make_variant_format(
            kMiddleLayouts[index], kMiddleVariants[index]);
    }
    if (dataFormat >= 57 && dataFormat <= 70) {
        return make_variant_format(1, dataFormat - 39);
    }
    if (dataFormat >= 71 && dataFormat <= 84) {
        return make_variant_format(2, dataFormat - 53);
    }

    return {0, static_cast<std::uint32_t>(-1),
            static_cast<std::uint32_t>(-1),
            static_cast<std::uint32_t>(-1), -1};
}

// Reconstructed from eboot.elf at 0x68E070.
std::int32_t RndFindDataFormat(
    const RndDataFormatInfo& descriptor) {
    constexpr std::int32_t kInvalidFormat = -1;
    if (descriptor.mGamma < 1) {
        return kInvalidFormat;
    }

    if (descriptor.mCompression >= 1) {
        if (descriptor.mCompression <= 4) {
            return 27 + (descriptor.mCompression - 1) * 2 +
                (descriptor.mGamma == 1 ? 0 : 1);
        }
        if (descriptor.mCompression >= 5 && descriptor.mCompression <= 10) {
            return descriptor.mGamma == 1
                ? 30 + descriptor.mCompression
                : kInvalidFormat;
        }
        if (descriptor.mCompression == 11 || descriptor.mCompression == 12) {
            return 19 + descriptor.mCompression * 2 +
                (descriptor.mGamma == 1 ? 0 : 1);
        }
        if (descriptor.mCompression == 13 || descriptor.mCompression == 14) {
            return descriptor.mGamma == 1
                ? 32 + descriptor.mCompression
                : kInvalidFormat;
        }
        if (descriptor.mCompression >= 15 && descriptor.mCompression <= 17) {
            return 17 + descriptor.mCompression * 2 +
                (descriptor.mGamma == 1 ? 0 : 1);
        }
        if (descriptor.mCompression >= 18 && descriptor.mCompression <= 31) {
            return descriptor.mCompression +
                (descriptor.mGamma == 1 ? 39 : 53);
        }
        return kInvalidFormat;
    }

    if (descriptor.mBitsPerPixel == 0 ||
        descriptor.mOrder == static_cast<std::uint32_t>(-1) ||
        descriptor.mStorage == static_cast<std::uint32_t>(-1)) {
        return kInvalidFormat;
    }

    const auto layout_one = descriptor.mGamma == 1;
    switch (descriptor.mStorage) {
    case 0:
        switch (descriptor.mOrder) {
        case 0:
            if (!layout_one) return kInvalidFormat;
            if (descriptor.mBitsPerPixel == 16) return 1;
            if (descriptor.mBitsPerPixel == 32) return 17;
            return kInvalidFormat;
        case 2:
            if (descriptor.mBitsPerPixel == 24) return layout_one ? 2 : 3;
            return layout_one && descriptor.mBitsPerPixel == 48
                ? 19 : kInvalidFormat;
        case 3:
            return descriptor.mBitsPerPixel == 24
                ? (layout_one ? 4 : 5) : kInvalidFormat;
        case 4:
            if (descriptor.mBitsPerPixel == 32) return layout_one ? 6 : 7;
            return layout_one && descriptor.mBitsPerPixel == 64
                ? 20 : kInvalidFormat;
        case 5:
            return descriptor.mBitsPerPixel == 32
                ? (layout_one ? 8 : 9) : kInvalidFormat;
        case 6:
            return descriptor.mBitsPerPixel == 32
                ? (layout_one ? 11 : 12) : kInvalidFormat;
        case 7:
            return descriptor.mBitsPerPixel == 32
                ? (layout_one ? 13 : 14) : kInvalidFormat;
        case 10:
            if (!layout_one) return kInvalidFormat;
            if (descriptor.mBitsPerPixel == 8) return 0;
            if (descriptor.mBitsPerPixel == 16) return 15;
            return kInvalidFormat;
        case 11:
            if (!layout_one) return kInvalidFormat;
            if (descriptor.mBitsPerPixel == 24) return 54;
            if (descriptor.mBitsPerPixel == 32) return 55;
            return kInvalidFormat;
        case 12:
            return layout_one && descriptor.mBitsPerPixel == 16
                ? 53 : kInvalidFormat;
        default:
            return kInvalidFormat;
        }
    case 1:
        return layout_one && descriptor.mOrder == 4 &&
                descriptor.mBitsPerPixel == 32
            ? 25 : kInvalidFormat;
    case 2:
        if (!layout_one) return kInvalidFormat;
        switch (descriptor.mOrder) {
        case 0:
            if (descriptor.mBitsPerPixel == 32) return 18;
            if (descriptor.mBitsPerPixel == 64) return 23;
            return kInvalidFormat;
        case 2:
            return descriptor.mBitsPerPixel == 32 ? 26 : kInvalidFormat;
        case 4:
            if (descriptor.mBitsPerPixel == 64) return 21;
            if (descriptor.mBitsPerPixel == 128) return 24;
            return kInvalidFormat;
        case 10:
            if (descriptor.mBitsPerPixel == 16) return 16;
            if (descriptor.mBitsPerPixel == 32) return 22;
            return kInvalidFormat;
        case 11:
            return descriptor.mBitsPerPixel == 40 ? 56 : kInvalidFormat;
        default:
            return kInvalidFormat;
        }
    case 3:
        return layout_one && descriptor.mOrder == 4 &&
                descriptor.mBitsPerPixel == 32
            ? 10 : kInvalidFormat;
    default:
        return kInvalidFormat;
    }
}

// Reconstructed from eboot.elf at 0x68E4D0 and 0x68E550.
std::int32_t RndFindSupportedDataFormat(
    const RndDataFormatInfo& descriptor,
    HxPlatform platform) {
    std::int32_t result = -1;
    if (try_supported_format(descriptor, platform, result)) {
        return result;
    }
    const auto exact_format = result;

    if (descriptor.mCompression < -1 || descriptor.mCompression >= 1) {
        return platform == kPlatformAndroid ? 21 : -1;
    }

    auto working = descriptor;
    const auto divisor = channel_divisor(working.mOrder);
    if (working.mOrder == 2 || working.mOrder == 3) {
        working.mBitsPerPixel = 4 * (working.mBitsPerPixel / divisor);
        working.mOrder = working.mOrder == 2 ? 5 : 7;
        if (try_supported_format(working, platform, result)) {
            return result;
        }
    }
    if (try_channel_fallback(working, platform, result)) {
        return result;
    }

    working = descriptor;
    if (working.mOrder == 2 || working.mOrder == 3) {
        working.mBitsPerPixel = 4 * (
            working.mBitsPerPixel / channel_divisor(working.mOrder));
        working.mOrder = working.mOrder == 2 ? 4 : 6;
        if (try_supported_format(working, platform, result)) {
            return result;
        }
    }
    if (try_channel_fallback(working, platform, result)) {
        return result;
    }

    if (platform == kPlatformAndroid && descriptor.mStorage != 2) {
        working = descriptor;
        working.mStorage = 2;
        if (try_supported_format(working, platform, result)) {
            return result;
        }

        const auto numeric_divisor =
            channel_divisor(working.mOrder);
        working.mBitsPerPixel = 4 * (working.mBitsPerPixel / numeric_divisor);
        if (working.mOrder == 2) {
            working.mOrder = 4;
        } else if (working.mOrder == 3) {
            working.mOrder = 6;
        }
        if (try_supported_format(working, platform, result)) {
            return result;
        }
    }

    if (descriptor.mOrder == 11) {
        constexpr std::array<std::int32_t, 3> kPackedFormats{54, 55, 56};
        std::int32_t exact_index = exact_format - kPackedFormats.front();
        if (exact_index < 0 || exact_index >=
                static_cast<std::int32_t>(kPackedFormats.size())) {
            exact_index = -1;
        }
        for (std::int32_t index = exact_index + 1;
             index < static_cast<std::int32_t>(kPackedFormats.size());
             ++index) {
            if (data_format_supported(kPackedFormats[index], platform)) {
                return kPackedFormats[index];
            }
        }
        for (auto index = exact_index; index > 0; --index) {
            if (data_format_supported(kPackedFormats[index - 1], platform)) {
                return kPackedFormats[index - 1];
            }
        }
    }

    return -1;
}

// Reconstructed from eboot.elf at 0x68F200; the table is at 0x1934360.
const char* RndDataFormatName(int dataFormat) {
    static const char* const kNames[] = {
        "Invalid", "R_UNorm8", "RG_UNorm8", "RGB_UNorm8", "RGB_UNorm8_sRGB",
        "BGR_UNorm8", "BGR_UNorm8_sRGB", "RGBA_UNorm8", "RGBA_UNorm8_sRGB",
        "RGBX_UNorm8", "RGBX_UNorm8_sRGB", "RGBA_UInt8", "BGRA_UNorm8",
        "BGRA_UNorm8_sRGB", "BGRX_UNorm8", "BGRX_UNorm8_sRGB", "R_UNorm16",
        "R_Float16", "RG_UNorm16", "RG_Float16", "RGB_UNorm16", "RGBA_UNorm16",
        "RGBA_Float16", "R_Float32", "RG_Float32", "RGBA_Float32",
        "RGBA_UNorm1010102", "RGB_Float111110", "BC1", "BC1_sRGB", "BC1A",
        "BC1A_sRGB", "BC2", "BC2_sRGB", "BC3", "BC3_sRGB", "BC4U", "BC4S",
        "BC5U", "BC5S", "BC6HU", "BC6HS", "BC7", "BC7_sRGB", "BC7A",
        "BC7A_sRGB", "ETC2R", "ETC2RG", "ETC2RGB", "ETC2RGB_sRGB", "ETC2RGBA1",
        "ETC2RGBA1_sRGB", "ETC2RGBA", "ETC2RGBA_sRGB", "Depth_UNorm16",
        "Depth_UNorm16_Stencil_UInt8", "Depth_UNorm24_Stencil_UInt8",
        "Depth_Float32_Stencil_UInt8", "ASTC_4x4", "ASTC_5x4", "ASTC_5x5",
        "ASTC_6x5", "ASTC_6x6", "ASTC_8x5", "ASTC_8x6", "ASTC_8x8",
        "ASTC_10x5", "ASTC_10x6", "ASTC_10x8", "ASTC_10x10", "ASTC_12x10",
        "ASTC_12x12", "ASTC_4x4_sRGB", "ASTC_5x4_sRGB", "ASTC_5x5_sRGB",
        "ASTC_6x5_sRGB", "ASTC_6x6_sRGB", "ASTC_8x5_sRGB", "ASTC_8x6_sRGB",
        "ASTC_8x8_sRGB", "ASTC_10x5_sRGB", "ASTC_10x6_sRGB", "ASTC_10x8_sRGB",
        "ASTC_10x10_sRGB", "ASTC_12x10_sRGB", "ASTC_12x12_sRGB",
    };
    const auto index = static_cast<unsigned int>(dataFormat + 1);
    if (index >= sizeof(kNames) / sizeof(kNames[0])) {
        return nullptr;
    }
    return kNames[index];
}
