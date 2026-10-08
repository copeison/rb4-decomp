#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct OrbisRenderContext;
struct OrbisRenderSystem;

void orbis_flush_active_frame(OrbisRenderSystem& system);

void orbis_lock_retired_allocations(OrbisRenderSystem& system);
void orbis_unlock_retired_allocations(OrbisRenderSystem& system);
std::size_t orbis_retired_allocation_count(
    const OrbisRenderSystem& system);
std::uint64_t orbis_retired_allocation_frame(
    const OrbisRenderSystem& system,
    std::size_t index);
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
