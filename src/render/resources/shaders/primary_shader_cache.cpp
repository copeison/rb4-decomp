#include "render/resources/shaders/primary_shader_cache.h"

#include <cstddef>
#include <cstdint>

#include "render/core/platform/render_platform_config.h"
#include "render/core/system/render_system_globals.h"
#include "render/resources/shaders/primary_shader_resource.h"
#include "render/resources/shaders/shader_permutations.h"
#include "render/resources/shaders/shader_source_hash.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kGeometryStage = 3;
constexpr std::uint32_t kLayoutHashedStageCount = 5;
constexpr std::size_t kActivePlatformConfig = 7;
constexpr std::uint32_t kSixSliceFeature = 0x4;

void append_word(std::uint32_t& hash, std::uint32_t value) {
    for (std::uint32_t shift = 0; shift < 32; shift += 8) {
        render_shader_source_hash_append_byte(
            hash, static_cast<std::uint8_t>(value >> shift));
    }
}

void append_key(std::uint32_t& hash, std::uint64_t key) {
    for (std::uint32_t shift = 0; shift < 64; shift += 8) {
        render_shader_source_hash_append_byte(
            hash, static_cast<std::uint8_t>(key >> shift));
    }
}

struct PermutationHashContext {
    std::uint32_t stage;
    std::uint32_t* hash;
    RenderPrimaryShaderResource* shader;
};

// Reconstructed from eboot.elf at 0x639640, the call operator of the functor
// whose dispatch is at 0x192EFC8. Only one- and six-slice permutations are
// hashed. Six slices need shader and platform support; single-slice geometry
// programs must be declared by the shader. The shader's own validator has the
// final say.
void hash_permutation(
    void* context,
    const RenderShaderPermutationDefine*,
    std::size_t,
    std::uint64_t key) {
    auto& state = *static_cast<PermutationHashContext*>(context);
    auto& shader = *state.shader;
    const auto slices = render_shader_parameter_binding_value(
        shader.render_target_slice_binding, key);
    if (slices != 1 && slices != 6) {
        return;
    }
    if (slices >= 2) {
        if (!shader.dispatch->supports_render_target_slices(&shader)) {
            return;
        }
        auto& platform = render_system_platform_config_at(
            *render_system_instance(), kActivePlatformConfig);
        if ((platform.feature_flags & kSixSliceFeature) == 0) {
            return;
        }
    } else if (state.stage == kGeometryStage &&
               !shader.dispatch->uses_geometry_program(&shader)) {
        return;
    }
    if (!shader.dispatch->validate_permutation(&shader, state.stage, key)) {
        return;
    }
    append_key(*state.hash, key);
}

}  // namespace

// Reconstructed from eboot.elf at 0x638D70. Hashes everything that shapes the
// compiled permutation set: the generated constant definitions, every
// parameter registry's names and ranges, and the keys of each valid
// permutation for the variant's graphics stages. Compute permutations are not
// part of this hash.
std::uint32_t render_primary_shader_layout_hash(
    RenderPrimaryShaderResource& shader) {
    auto hash = kShaderSourceHashBasis;
    render_shader_constant_registry_accumulate_source_hash(
        *shader.constants, hash);

    for (const auto& registry : shader.parameters->registries) {
        append_word(
            hash, static_cast<std::uint32_t>(registry.end - registry.begin));
        for (auto* record = registry.begin; record != registry.end; ++record) {
            render_shader_source_hash_append(hash, record->name.text);
            append_word(hash, record->first_value);
            append_word(hash, record->last_value_exclusive);
        }
    }

    const auto variant = shader.dispatch->variant(&shader);
    for (std::uint32_t stage = 0; stage < kLayoutHashedStageCount; ++stage) {
        if (!render_shader_stage_enabled(variant, stage)) {
            continue;
        }
        PermutationHashContext context{stage, &hash, &shader};
        render_shader_enumerate_stage_permutations(
            *shader.parameters, stage, hash_permutation, &context);
    }
    return hash;
}

}  // namespace rb4
