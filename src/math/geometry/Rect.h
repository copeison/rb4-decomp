#pragma once

namespace Hmx {

// Axis-aligned rectangle: the origin and the size. Field names are not in
// the reference map.
class Rect {
public:
    Rect() = default;
    constexpr Rect(float x_, float y_, float w_, float h_) : x(x_), y(y_), w(w_), h(h_) {}

    float x;
    float y;
    float w;
    float h;
};

static_assert(sizeof(Rect) == 16);

}  // namespace Hmx
