#pragma once

#include <cstddef>

namespace rb4 {

struct RenderFrameOwner;

struct RenderFrameOwnerList {
    RenderFrameOwner** items;
    std::size_t count;
};

static_assert(sizeof(RenderFrameOwnerList) == 16);

}  // namespace rb4
