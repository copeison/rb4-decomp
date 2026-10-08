#pragma once

#include <cstdint>

namespace rb4 {

struct RenderDataFormatDescriptor {
    std::uint32_t bit_width;
    std::uint32_t channel_layout;
    std::uint32_t numeric_type;
    std::uint32_t layout;
    std::int32_t variant;
};

static_assert(sizeof(RenderDataFormatDescriptor) == 20);

RenderDataFormatDescriptor render_data_format_describe(
    std::int32_t data_format);
std::uint32_t render_data_format_bits_per_pixel(
    std::int32_t data_format);

}  // namespace rb4
