#pragma once

#include <cstdint>

namespace rb4 {

// Width and height in pixels. The map's signatures use Vector2i for these
// values; the integer vector has not been converted yet.
struct RenderExtent {
    std::uint32_t width = 0;
    std::uint32_t height = 0;

    bool empty() const {
        return width == 0 || height == 0;
    }
};

}  // namespace rb4
