#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

// View of the fields consumed from the engine's 0x210-byte affinity record.
struct ThreadAffinityGroup {
    std::uint8_t prefix[24];
    std::int64_t primary_processor;
    std::uint8_t reserved[8];
    std::uint64_t additional_processor_mask;
};

static_assert(offsetof(ThreadAffinityGroup, primary_processor) == 24);
static_assert(offsetof(ThreadAffinityGroup, additional_processor_mask) == 40);

const ThreadAffinityGroup* thread_affinity_find_group(const char* name);
std::uint64_t thread_affinity_build_cpu_mask(
    std::int64_t primary_processor,
    std::uint64_t additional_processor_mask);

void fmod_load_modules_and_set_thread_affinity();

}  // namespace rb4
