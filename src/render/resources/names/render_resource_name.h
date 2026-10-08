#pragma once

#include <cstddef>

namespace rb4 {

struct RenderResourceName {
    void* dispatch;
    const char* text;
};

static_assert(sizeof(RenderResourceName) == 16);

void render_resource_name_construct(
    RenderResourceName& name,
    const char* text);
void render_resource_name_destruct(RenderResourceName& name);

}  // namespace rb4
