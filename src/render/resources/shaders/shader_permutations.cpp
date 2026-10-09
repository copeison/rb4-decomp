#include "render/resources/shaders/shader_permutations.h"

#include <vector>

#include "core/types/integer_text_adapters.h"
#include "render/resources/shaders/shader_parameter_registry.h"

namespace rb4 {

namespace {

// Stage-to-registry map built by 0x63D100. Hull and domain programs share the
// tessellation registry.
constexpr std::size_t kStageRegistries[] = {1, 2, 2, 3, 4, 5};
constexpr std::size_t kStageCount =
    sizeof(kStageRegistries) / sizeof(*kStageRegistries);

struct Enumeration {
    std::vector<const RenderShaderParameterRecord*> records;
    std::vector<RenderShaderPermutationDefine> defines;
    RenderShaderPermutationVisitor visitor;
    void* context;
};

// Reconstructed from eboot.elf at 0x63C8F0. Each record contributes every
// value in its half-open range. Enabled records occupy the high 32 bits of
// the key, so global parameters never collide with stage parameters.
void enumerate(Enumeration& enumeration, std::size_t depth, std::uint64_t key) {
    if (depth >= enumeration.records.size()) {
        enumeration.visitor(
            enumeration.context,
            enumeration.defines.data(),
            enumeration.defines.size(),
            key);
        return;
    }

    const auto& record = *enumeration.records[depth];
    std::uint64_t mask = record.shifted_mask;
    const auto shift = record.enabled ? 32U : 0U;
    mask <<= shift;
    for (auto value = static_cast<std::int32_t>(record.first_value);
         value < static_cast<std::int32_t>(record.last_value_exclusive);
         ++value) {
        enumeration.defines.push_back(
            {record.name.text, engine_integer_text(value)});
        // The shifted 32-bit field is sign-extended before placement.
        const auto shifted = static_cast<std::int32_t>(
            static_cast<std::uint32_t>(
                value - static_cast<std::int32_t>(record.first_value))
            << record.bit_offset);
        const auto field =
            static_cast<std::uint64_t>(static_cast<std::int64_t>(shifted))
            << shift;
        enumerate(enumeration, depth + 1, (key & ~mask) | field);
        enumeration.defines.pop_back();
    }
}

void append_records(
    Enumeration& enumeration,
    const RenderShaderParameterRegistry& registry) {
    for (auto* record = registry.begin; record != registry.end; ++record) {
        enumeration.records.push_back(record);
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x63E5E0. Bits 0-4 of a shader variant
// select vertex, tessellation, geometry, pixel, and compute programs.
bool render_shader_stage_enabled(std::int32_t variant, std::uint32_t stage) {
    switch (stage) {
    case 0:
        return (variant & 0x01) != 0;
    case 1:
    case 2:
        return (variant & 0x02) != 0;
    case 3:
        return (variant & 0x04) != 0;
    case 4:
        return (variant & 0x08) != 0;
    case 5:
        return (variant & 0x10) != 0;
    default:
        return false;
    }
}

// Reconstructed from eboot.elf at 0x63D100 and 0x63C740. Global parameters
// from the first registry are enumerated before the stage's own parameters.
void render_shader_enumerate_stage_permutations(
    const RenderShaderParameterRegistrySet& parameters,
    std::uint32_t stage,
    RenderShaderPermutationVisitor visitor,
    void* context) {
    Enumeration enumeration{{}, {}, visitor, context};
    append_records(enumeration, parameters.registries[0]);
    if (stage < kStageCount) {
        append_records(
            enumeration, parameters.registries[kStageRegistries[stage]]);
    }
    enumerate(enumeration, 0, 0);
}

}  // namespace rb4
