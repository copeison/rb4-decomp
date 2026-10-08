#include "orbis_fence.h"

#include <cstddef>
#include <cstdint>

#include "orbis_fence_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisFenceSize = 24;
constexpr const char* kFenceAllocationName = "PS4Fence";

}  // namespace

// Reconstructed from eboot.elf at 0x8D85C0.
OrbisFence* orbis_create_fence() {
    auto* storage = render_allocate(kOrbisFenceSize);
    auto* fence = reinterpret_cast<OrbisFence*>(storage);
    orbis_fence_construct(*fence);
    return fence;
}

// Reconstructed from eboot.elf at 0x8E1570.
void orbis_fence_construct(OrbisFence& fence) {
    auto* value = orbis_allocate_fence_value(
        sizeof(std::uint32_t), kFenceAllocationName, 4);
    *value = 0;
    orbis_fence_set_value_storage(fence, value);
}

}  // namespace rb4
