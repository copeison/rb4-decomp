#include "core/threading/thread_affinity.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x2590B0.
std::uint64_t thread_affinity_build_cpu_mask(
    std::int64_t primary_processor,
    std::uint64_t additional_processor_mask) {
    auto mask = additional_processor_mask;
    if (primary_processor != -1) {
        mask |= std::uint64_t{1} << primary_processor;
    }
    if (mask != 0) {
        return mask;
    }

    return (std::uint64_t{1} << kEngineProcessorCount) - 1;
}

}  // namespace rb4
