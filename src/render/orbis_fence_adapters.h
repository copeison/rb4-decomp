#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct OrbisFence;

void* render_allocate(std::size_t size);
std::uint32_t* orbis_allocate_fence_value(
    std::size_t size,
    const char* name,
    std::uint32_t alignment);
void orbis_fence_set_value_storage(
    OrbisFence& fence,
    std::uint32_t* value);

}  // namespace rb4
