#pragma once

#include <cstddef>
#include <cstdint>

#include "render/resources/names/render_resource_name.h"

namespace rb4 {

struct RenderShaderParameterRecord {
    RenderResourceName name;
    std::uint32_t first_value;
    std::uint32_t last_value_exclusive;
    std::uint32_t shifted_mask;
    std::uint32_t bit_offset;
    bool enabled;
    std::uint8_t reserved_33[7];
};

struct RenderShaderParameterRegistry {
    RenderShaderParameterRecord* begin;
    RenderShaderParameterRecord* end;
    RenderShaderParameterRecord* capacity;
    void* allocator;
    std::uint32_t bit_count;
    bool enabled;
    std::uint8_t reserved_37[3];
};

struct RenderShaderParameterRegistrySet {
    RenderShaderParameterRegistry registries[6];
};

struct RenderShaderParameterBinding {
    std::uint32_t first_value;
    std::uint32_t last_value_exclusive;
    std::uint32_t shifted_mask;
    std::uint32_t bit_offset;
    bool enabled;
    std::uint8_t reserved_17[3];
};

static_assert(sizeof(RenderShaderParameterRecord) == 40);
static_assert(offsetof(RenderShaderParameterRecord, first_value) == 16);
static_assert(offsetof(RenderShaderParameterRecord, enabled) == 32);
static_assert(sizeof(RenderShaderParameterRegistry) == 40);
static_assert(sizeof(RenderShaderParameterRegistrySet) == 240);
static_assert(sizeof(RenderShaderParameterBinding) == 20);

inline void render_shader_parameter_registry_set_construct(
    RenderShaderParameterRegistrySet& parameters) {
    parameters = {};
    parameters.registries[0].enabled = true;
}

void render_shader_parameter_registry_add(
    RenderShaderParameterBinding* binding,
    RenderShaderParameterRegistry* registry,
    const void* parameter_name,
    std::uint32_t first_value,
    std::uint32_t last_value_exclusive);
std::uint32_t render_shader_parameter_binding_value(
    const RenderShaderParameterBinding& binding,
    std::uint64_t key);
RenderShaderParameterBinding* render_shader_parameter_registry_add_ternary(
    RenderShaderParameterBinding* binding,
    RenderShaderParameterRegistry* registry,
    const void* parameter_name);

}  // namespace rb4
