#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct OrbisRenderContext;
struct OrbisRenderSystem;

void orbis_release_retired_allocation(
    OrbisRenderSystem& system,
    std::size_t index);
void orbis_erase_retired_allocation(
    OrbisRenderSystem& system,
    std::size_t index);
void orbis_enqueue_retired_allocation(
    OrbisRenderSystem& system,
    void* allocation,
    std::uint64_t frame);

}  // namespace rb4
