#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderShaderParameterRecord {
    std::uint8_t resource_name[40];
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
    std::uint32_t bit_offset;
    std::uint32_t shifted_mask;
    std::uint32_t shift;
    bool enabled;
    std::uint8_t reserved_17[3];
};

static_assert(sizeof(RenderShaderParameterRecord) == 40);
static_assert(sizeof(RenderShaderParameterRegistry) == 40);
static_assert(sizeof(RenderShaderParameterRegistrySet) == 240);
static_assert(sizeof(RenderShaderParameterBinding) == 20);

inline void render_shader_parameter_registry_set_construct(
    RenderShaderParameterRegistrySet& parameters) {
    parameters = {};
    parameters.registries[0].enabled = true;
}

}  // namespace rb4
