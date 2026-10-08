#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderSystem;

struct RenderLightingResources {
    void* primary_resource;
    void* secondary_resource;
    float scale_factor;
    std::uint32_t reserved_20;
    std::uint64_t primary_group_size;
    std::uint64_t secondary_group_size;
    void* resource_slots_40[13];
    void* owned_state_144;
    void* owned_state_152;
    void* reserved_state_160;
    void* reserved_state_168;
    void* owned_state_176;
    void* resource_slots_184[4];
    void** primary_array_begin;
    void** primary_array_end;
    void** primary_array_capacity;
    void* primary_array_allocator;
    void** secondary_array_begin;
    void** secondary_array_end;
    void** secondary_array_capacity;
    void* secondary_array_allocator;
    void* resource_slots_280[3];
};

static_assert(offsetof(RenderLightingResources, scale_factor) == 16);
static_assert(offsetof(RenderLightingResources, resource_slots_40) == 40);
static_assert(offsetof(RenderLightingResources, owned_state_144) == 144);
static_assert(
    offsetof(RenderLightingResources, primary_array_begin) == 216);
static_assert(
    offsetof(RenderLightingResources, secondary_array_begin) == 248);
static_assert(offsetof(RenderLightingResources, resource_slots_280) == 280);
static_assert(sizeof(RenderLightingResources) == 304);

RenderLightingResources& render_system_lighting_resources(
    RenderSystem& system);
void render_lighting_resources_construct(RenderLightingResources& resources);
void render_lighting_resources_shutdown(RenderLightingResources& resources);
void render_lighting_resources_destruct(RenderLightingResources& resources);

}  // namespace rb4
