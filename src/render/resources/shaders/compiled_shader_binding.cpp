#include "render/resources/shaders/compiled_shader_objects.h"

#include <cstdint>

#include "render/context/RndContext.h"
#include "render/shaders/RndShaderProgram.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/system/render_system_state.h"

namespace rb4 {

namespace {

// Table at 0x12A9D10: the variant program bit and key slot for each stage.
constexpr std::uint32_t kStageProgramBits[kRenderShaderStageCount] = {
    0, 1, 1, 2, 3, 4,
};

bool has_program(std::int32_t variant, std::uint32_t bit) {
    return ((static_cast<std::uint32_t>(variant) >> bit) & 1U) != 0;
}

RndShaderProgram* find_object(
    const RenderManagedObjectArray& objects,
    std::uint64_t key) {
    auto** first = objects.begin;
    auto count = objects.end - objects.begin;
    while (count > 0) {
        const auto half = count / 2;
        const auto* middle =
            reinterpret_cast<const RndShaderProgram*>(first[half]);
        if (middle->mKey < key) {
            first += half + 1;
            count -= half + 1;
        } else {
            count = half;
        }
    }
    if (first == objects.end) {
        return nullptr;
    }
    return reinterpret_cast<RndShaderProgram*>(*first);
}

}  // namespace

// Reconstructed from eboot.elf at 0x63B500. Stages the variant uses are marked
// active and stages it no longer uses are unbound. Each used stage then binds
// the compiled object whose permutation key matches, stamping the current
// frame epoch. A missing or uninitialized object fails the bind; the original
// only looks up stage and shading-mode names for a stripped diagnostic there.
bool render_compiled_shader_objects_bind(
    RenderManagedObjectArray (&objects)[kRenderShaderStageCount],
    RndContext& context,
    std::int32_t variant,
    const std::uint64_t (&keys)[kRenderShaderProgramKeyCount]) {
    auto& active = (context).mActiveShaderStages;
    for (std::uint32_t stage = 0; stage < kRenderShaderStageCount; ++stage) {
        const auto stage_bit = static_cast<std::uint8_t>(1U << stage);
        if (has_program(variant, kStageProgramBits[stage])) {
            active = static_cast<std::uint8_t>(active | stage_bit);
        } else if ((active & stage_bit) != 0) {
            context.DeactivateShaderProgramType(static_cast<RndShaderProgramType>(stage));
        }
    }

    const auto frame_epoch = static_cast<std::int64_t>(
        render_system_core_state(*render_system_instance()).frame_epoch);
    for (std::uint32_t stage = 0; stage < kRenderShaderStageCount; ++stage) {
        const auto bit = kStageProgramBits[stage];
        if (!has_program(variant, bit)) {
            continue;
        }
        const auto key = keys[bit];
        auto* shader = find_object(objects[stage], key);
        if (shader == nullptr || shader->mKey != key ||
            !shader->mCreated) {
            return false;
        }
        shader->mLastSelectFrame = frame_epoch;
        shader->_SelectImpl(context);
    }
    return true;
}

}  // namespace rb4
