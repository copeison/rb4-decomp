#include "render/resources/lighting/render_lighting_resources.h"

#include <cstddef>
#include <cstdint>

#include "os/memory/MemMgr.h"
#include "utl/containers/Std.h"
#include "render/resources/lighting/render_lighting_resources_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kLightingResourcesOffset = 3256;

struct RenderLightingResourceDispatch {
    void* reserved_0;
    void (*release_dynamic)(void* resource);
};

void release_dynamic_resource(void*& resource) {
    if (resource == nullptr) {
        return;
    }

    auto* dispatch = *static_cast<RenderLightingResourceDispatch**>(resource);
    dispatch->release_dynamic(resource);
    resource = nullptr;
}

void clear_resource_array(void**& begin, void**& end) {
    std::size_t index = 0;
    while (begin != end) {
        const auto count = static_cast<std::size_t>(end - begin);
        if (index >= count) {
            break;
        }
        release_dynamic_resource(begin[index]);
        ++index;
    }
    end = begin;
}

void release_pointer_array(
    void**& begin,
    void**& end,
    void**& capacity) {
    if (begin != nullptr) {
        const auto byte_count = static_cast<std::size_t>(
            reinterpret_cast<std::uint8_t*>(capacity) -
            reinterpret_cast<std::uint8_t*>(begin));
        HmxAllocator::gStlAllocator.deallocate(begin, byte_count);
    }
    begin = nullptr;
    end = nullptr;
    capacity = nullptr;
}

void release_owned_state(void*& state) {
    if (state != nullptr) {
        render_lighting_owned_state_release(state);
        state = nullptr;
    }
}

}  // namespace

RenderLightingResources& render_system_lighting_resources(
    RenderSystem& system) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&system);
    return *reinterpret_cast<RenderLightingResources*>(
        bytes + kLightingResourcesOffset);
}

// Reconstructed from eboot.elf at 0x47EEE0.
void render_lighting_resources_construct(RenderLightingResources& resources) {
    resources.primary_resource = nullptr;
    resources.secondary_resource = nullptr;
    resources.scale_factor = -1.0F;
    resources.reserved_20 = 0;
    resources.primary_group_size = 16;
    resources.secondary_group_size = 8;
    for (auto*& resource : resources.resource_slots_40) {
        resource = nullptr;
    }
    resources.owned_state_144 = nullptr;
    resources.owned_state_152 = nullptr;
    resources.reserved_state_160 = nullptr;
    resources.reserved_state_168 = nullptr;
    resources.owned_state_176 = nullptr;
    for (auto*& resource : resources.resource_slots_184) {
        resource = nullptr;
    }
    resources.primary_array_begin = nullptr;
    resources.primary_array_end = nullptr;
    resources.primary_array_capacity = nullptr;
    resources.primary_array_allocator = nullptr;
    resources.secondary_array_begin = nullptr;
    resources.secondary_array_end = nullptr;
    resources.secondary_array_capacity = nullptr;
    resources.secondary_array_allocator = nullptr;
    for (auto*& resource : resources.resource_slots_280) {
        resource = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x47F580.
void render_lighting_resources_shutdown(RenderLightingResources& resources) {
    release_dynamic_resource(resources.primary_resource);
    release_dynamic_resource(resources.secondary_resource);

    for (std::size_t index = 0; index < 8; ++index) {
        release_dynamic_resource(resources.resource_slots_40[index]);
    }
    for (std::size_t index = 9; index < 13; ++index) {
        release_dynamic_resource(resources.resource_slots_40[index]);
    }

    release_dynamic_resource(resources.resource_slots_280[2]);
    release_dynamic_resource(resources.resource_slots_184[0]);
    release_dynamic_resource(resources.resource_slots_184[1]);
    release_dynamic_resource(resources.resource_slots_40[8]);
    release_dynamic_resource(resources.resource_slots_184[2]);
    release_dynamic_resource(resources.resource_slots_184[3]);

    clear_resource_array(
        resources.primary_array_begin,
        resources.primary_array_end);
    clear_resource_array(
        resources.secondary_array_begin,
        resources.secondary_array_end);

    release_dynamic_resource(resources.resource_slots_280[0]);
    release_dynamic_resource(resources.resource_slots_280[1]);
    release_owned_state(resources.owned_state_144);
    release_owned_state(resources.owned_state_176);
    release_owned_state(resources.owned_state_152);
    resources.reserved_state_160 = nullptr;
    resources.reserved_state_168 = nullptr;
}

// Reconstructed from eboot.elf at 0x47EFA0.
void render_lighting_resources_destruct(RenderLightingResources& resources) {
    release_pointer_array(
        resources.secondary_array_begin,
        resources.secondary_array_end,
        resources.secondary_array_capacity);
    release_pointer_array(
        resources.primary_array_begin,
        resources.primary_array_end,
        resources.primary_array_capacity);
    release_owned_state(resources.owned_state_176);
    release_owned_state(resources.owned_state_152);
    release_owned_state(resources.owned_state_144);
}

}  // namespace rb4
