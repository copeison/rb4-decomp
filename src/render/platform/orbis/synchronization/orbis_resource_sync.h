#pragma once

#include <cstdint>

class PS4Context;

namespace rb4 {

struct OrbisResourceSignal {
    const void* resource = nullptr;
    volatile std::uint32_t* label = nullptr;
    std::uint64_t render_epoch = 0;
};

static_assert(sizeof(OrbisResourceSignal) == 24);

void orbis_render_context_signal_resource(
    PS4Context& context,
    const void* resource,
    volatile std::uint32_t*& shared_label);
void orbis_render_context_wait_for_resource(
    PS4Context& context,
    const void* resource);

}  // namespace rb4
