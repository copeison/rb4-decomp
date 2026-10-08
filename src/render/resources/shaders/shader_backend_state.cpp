#include "render/resources/shaders/shader_backend_state.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "core/memory/engine_memory.h"
#include "core/types/symbol.h"

namespace rb4 {

namespace {

void release_binding_array(RenderShaderBackendBindingArray& array) {
    if (array.begin != nullptr) {
        const auto byte_count = static_cast<std::size_t>(
            reinterpret_cast<std::uint8_t*>(array.capacity) -
            reinterpret_cast<std::uint8_t*>(array.begin));
        engine_deallocate_sized(array.begin, byte_count);
    }
    array = {};
}

}  // namespace

void render_shader_backend_state_construct(RenderShaderBackendState& state) {
    state = {};
}

void render_shader_backend_state_destruct(RenderShaderBackendState& state) {
    for (std::size_t index = 24; index != 0; --index) {
        release_binding_array(state.binding_arrays[index - 1]);
    }
    for (auto& count : state.stage_resource_counts) {
        count = 0;
    }
}

// Reconstructed from eboot.elf at 0x643260.
std::uint64_t render_shader_backend_add_texture_binding(
    RenderShaderBackendState& state,
    const char* resource_name,
    const char* sampler_name,
    std::uint32_t resource_dimension,
    std::uint32_t stage,
    std::uint32_t stage_mask) {
    constexpr std::size_t kStageCount = 6;
    if (stage >= kStageCount) {
        return 0;
    }

    const Symbol resource_symbol(resource_name);
    const Symbol sampler_symbol(sampler_name);
    static const Symbol empty_symbol("");

    std::uint64_t resource_index = 0;
    constexpr std::uint32_t kBindingArrayGroups[]{0, 6, 12};
    for (const auto array_offset : kBindingArrayGroups) {
        const auto& array = state.binding_arrays[stage + array_offset];
        resource_index += static_cast<std::uint64_t>(
            array.begin == nullptr ? 0 : array.end - array.begin);
    }

    const auto array_offset = sampler_symbol.value() == empty_symbol.value()
        ? 6U
        : 0U;
    auto& array = state.binding_arrays[stage + array_offset];
    const auto size = array.begin == nullptr
        ? std::size_t{0}
        : static_cast<std::size_t>(array.end - array.begin);
    const auto capacity = array.begin == nullptr
        ? std::size_t{0}
        : static_cast<std::size_t>(array.capacity - array.begin);
    if (size == capacity) {
        const auto new_capacity = size == 0 ? std::size_t{1} : size * 2;
        auto* replacement = static_cast<RenderShaderBackendBinding*>(
            engine_allocate_sized(
                new_capacity * sizeof(RenderShaderBackendBinding)));
        if (size != 0) {
            std::memmove(
                replacement,
                array.begin,
                size * sizeof(RenderShaderBackendBinding));
        }
        if (array.begin != nullptr) {
            engine_deallocate_sized(
                array.begin,
                capacity * sizeof(RenderShaderBackendBinding));
        }
        array.begin = replacement;
        array.end = replacement + size;
        array.capacity = replacement + new_capacity;
    }

    auto& binding = *array.end++;
    binding = {};
    binding.resource_dimension = resource_dimension;
    binding.stage_mask = stage_mask;
    binding.binding_index = -1;
    binding.resource_name = resource_symbol.value();
    binding.sampler_name = sampler_symbol.value();
    binding.default_sampler_name = empty_symbol.value();
    binding.resource_index = resource_index;
    binding.stage_resource_count = state.stage_resource_counts[stage]++;
    return resource_index;
}

}  // namespace rb4
