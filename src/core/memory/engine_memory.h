#pragma once

#include <cstddef>

namespace rb4 {

void* render_allocate(std::size_t size);
void* render_allocate_named(
    std::size_t size,
    const char* name,
    std::size_t alignment);
void* engine_allocate_sized(std::size_t size);
void render_release(void* allocation);
void render_free(void* allocation);
void engine_deallocate_sized(void* allocation, std::size_t size);

}  // namespace rb4
