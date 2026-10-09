#include "render/resources/shaders/shader_parameter_registry.h"

#include "core/memory/engine_memory.h"
#include "render/resources/names/render_resource_name.h"

namespace rb4 {

namespace {

RenderShaderParameterRecord& append_record(
    RenderShaderParameterRegistry& registry) {
    if (registry.end == registry.capacity) {
        const auto old_count = registry.begin == nullptr
            ? std::size_t{0}
            : static_cast<std::size_t>(registry.end - registry.begin);
        const auto new_count = old_count == 0 ? 1 : old_count * 2;
        auto* records = static_cast<RenderShaderParameterRecord*>(
            engine_allocate_sized(
                new_count * sizeof(RenderShaderParameterRecord)));

        for (std::size_t index = 0; index < old_count; ++index) {
            const auto& source = registry.begin[index];
            auto& destination = records[index];
            render_resource_name_construct(destination.name, source.name.text);
            destination.first_value = source.first_value;
            destination.last_value_exclusive = source.last_value_exclusive;
            destination.shifted_mask = source.shifted_mask;
            destination.bit_offset = source.bit_offset;
            destination.enabled = source.enabled;
            for (auto& byte : destination.reserved_33) {
                byte = 0;
            }
        }
        for (auto* record = registry.begin;
             record != registry.end;
             ++record) {
            render_resource_name_destruct(record->name);
        }
        if (registry.begin != nullptr) {
            const auto byte_count = static_cast<std::size_t>(
                reinterpret_cast<std::uint8_t*>(registry.capacity) -
                reinterpret_cast<std::uint8_t*>(registry.begin));
            engine_deallocate_sized(registry.begin, byte_count);
        }

        registry.begin = records;
        registry.end = records + old_count;
        registry.capacity = records + new_count;
    }

    auto& record = *registry.end;
    ++registry.end;
    return record;
}

}  // namespace

// Reconstructed from eboot.elf at 0x63C3F0.
void render_shader_parameter_registry_add(
    RenderShaderParameterBinding* binding,
    RenderShaderParameterRegistry* registry,
    const void* parameter_name,
    std::uint32_t first_value,
    std::uint32_t last_value_exclusive) {
    auto value_count_minus_one =
        last_value_exclusive + ~first_value;
    std::uint32_t bit_count = 0;
    std::uint32_t mask = 0;
    while (value_count_minus_one != 0) {
        value_count_minus_one >>= 1;
        ++bit_count;
        mask = mask * 2 + 1;
    }

    auto& record = append_record(*registry);
    render_resource_name_construct(
        record.name,
        static_cast<const char*>(parameter_name));
    record.first_value = first_value;
    record.last_value_exclusive = last_value_exclusive;
    record.shifted_mask = mask << registry->bit_count;
    record.bit_offset = registry->bit_count;
    record.enabled = registry->enabled;
    for (auto& byte : record.reserved_33) {
        byte = 0;
    }
    registry->bit_count += bit_count;

    binding->first_value = record.first_value;
    binding->last_value_exclusive = record.last_value_exclusive;
    binding->shifted_mask = record.shifted_mask;
    binding->bit_offset = record.bit_offset;
    binding->enabled = record.enabled;
    for (auto& byte : binding->reserved_17) {
        byte = 0;
    }
}

// Reconstructed from eboot.elf at 0x63C550.
RenderShaderParameterBinding* render_shader_parameter_registry_add_ternary(
    RenderShaderParameterBinding* binding,
    RenderShaderParameterRegistry* registry,
    const void* parameter_name) {
    render_shader_parameter_registry_add(
        binding,
        registry,
        parameter_name,
        0,
        2);
    return binding;
}

// Reconstructed from eboot.elf at 0x63C380. Enabled bindings live in the high
// 32 bits of the permutation key.
std::uint32_t render_shader_parameter_binding_value(
    const RenderShaderParameterBinding& binding,
    std::uint64_t key) {
    std::uint64_t mask = binding.shifted_mask;
    if (binding.enabled) {
        mask <<= 32;
    }
    const auto field = (key & mask) >> binding.bit_offset;
    const auto value = binding.enabled
        ? static_cast<std::uint32_t>(field >> 32)
        : static_cast<std::uint32_t>(field);
    return binding.first_value + value;
}

}  // namespace rb4
