#pragma once

namespace rb4 {

struct RenderResourceName;

void render_resource_name_construct(
    RenderResourceName& name,
    const char* text);
void render_resource_name_destruct(void* name);

}  // namespace rb4
