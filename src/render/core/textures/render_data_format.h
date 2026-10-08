#pragma once

#include <cstdint>

namespace rb4 {

struct RenderDataFormatDescriptor {
    std::uint32_t channel_width;
    std::uint32_t channel_count;
    std::uint32_t numeric_type;
    std::uint32_t layout;
    std::int32_t variant;
};

static_assert(sizeof(RenderDataFormatDescriptor) == 20);

}  // namespace rb4
