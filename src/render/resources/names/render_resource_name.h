#pragma once

#include <cstddef>

namespace rb4 {

struct RenderResourceName {
    void* dispatch;
    const char* text;
};

static_assert(sizeof(RenderResourceName) == 16);

}  // namespace rb4
