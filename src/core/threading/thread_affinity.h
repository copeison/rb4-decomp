#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

constexpr std::size_t kEngineProcessorCount = 6;

// View of the fields consumed from the engine's 0x210-byte affinity record.
struct ThreadAffinityGroup {
    std::uint8_t reserved_0[16];
    std::uint32_t stack_size;
    std::uint8_t reserved_20[4];
    std::int64_t primary_processor;
    std::int32_t priority;
    std::uint8_t reserved_36[4];
    std::uint64_t additional_processor_mask;
};

static_assert(offsetof(ThreadAffinityGroup, stack_size) == 16);
static_assert(offsetof(ThreadAffinityGroup, primary_processor) == 24);
static_assert(offsetof(ThreadAffinityGroup, priority) == 32);
static_assert(offsetof(ThreadAffinityGroup, additional_processor_mask) == 40);

std::uint64_t thread_affinity_build_cpu_mask(
    std::int64_t primary_processor,
    std::uint64_t additional_processor_mask);

}  // namespace rb4
