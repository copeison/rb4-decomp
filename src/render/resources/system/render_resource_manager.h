#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderSystem;

struct RenderResourcePointerArray {
    void** begin;
    void** end;
    void** capacity;
    void* allocator;
};

struct RenderResourceListNode {
    RenderResourceListNode* next;
    RenderResourceListNode* previous;
};

struct RenderResourceManagerRuntime {
    void* sized_array_owners[3];
    void* name_array_owner;
    void* specialized_state;
    void* function_table_texture;
    void* resources[35];
    std::uint64_t reserved_680;
};

struct RenderResourceManager {
    std::uint8_t registries[80];
    std::int64_t handle_state[34];
    RenderResourceManagerRuntime runtime;
    RenderResourcePointerArray* pointer_array;
    RenderResourceListNode* primary_list;
    RenderResourceListNode* secondary_list;
};

static_assert(sizeof(RenderResourcePointerArray) == 32);
static_assert(sizeof(RenderResourceListNode) == 16);
static_assert(offsetof(RenderResourceManager, handle_state) == 80);
static_assert(sizeof(RenderResourceManagerRuntime) == 336);
static_assert(offsetof(RenderResourceManager, runtime) == 352);
static_assert(
    offsetof(RenderResourceManager, runtime) +
        offsetof(RenderResourceManagerRuntime, function_table_texture) ==
    392);
static_assert(
    offsetof(RenderResourceManager, runtime) +
        offsetof(RenderResourceManagerRuntime, resources) ==
    400);
static_assert(offsetof(RenderResourceManager, pointer_array) == 688);
static_assert(offsetof(RenderResourceManager, primary_list) == 696);
static_assert(offsetof(RenderResourceManager, secondary_list) == 704);
static_assert(sizeof(RenderResourceManager) == 712);

RenderResourceManager& render_system_resource_manager(RenderSystem& system);
void render_resource_manager_construct(RenderResourceManager& manager);
void render_resource_manager_destruct(RenderResourceManager& manager);
void render_resource_manager_shutdown(RenderResourceManager& manager);
void render_resource_manager_reload_shaders(RenderResourceManager& manager);

}  // namespace rb4
