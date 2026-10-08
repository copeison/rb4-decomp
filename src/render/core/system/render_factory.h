#pragma once

namespace rb4 {

struct RenderFactory {
    const void* vtable;
};

static_assert(sizeof(RenderFactory) == 8);

}  // namespace rb4
