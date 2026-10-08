#include "render/resources/system/render_resource_manager.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include "core/memory/engine_memory.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"

namespace rb4 {

namespace {

constexpr std::size_t kResourceManagerOffset = 2544;
constexpr std::size_t kPrimaryShaderLinkOffset = 272;
constexpr std::ptrdiff_t kSecondaryShaderDirtyOffset = -395;

struct RenderManagedObjectDispatch {
    void* reserved_0;
    void (*release_dynamic)(void* object);
};

struct RenderManagedObject {
    RenderManagedObjectDispatch* dispatch;
};

struct RenderManagedObjectArray {
    RenderManagedObject** begin;
    RenderManagedObject** end;
    RenderManagedObject** capacity;
    void* allocator;
};

struct RenderPrimaryShaderResource {
    std::uint8_t reserved_0[12];
    bool compiled;
    std::uint8_t reserved_13[3];
    RenderManagedObjectArray compiled_objects[6];
    std::uint8_t reserved_208[64];
    RenderResourceListNode manager_link;
};

static_assert(sizeof(RenderManagedObjectArray) == 32);
static_assert(offsetof(RenderPrimaryShaderResource, compiled) == 12);
static_assert(offsetof(RenderPrimaryShaderResource, compiled_objects) == 16);
static_assert(
    offsetof(RenderPrimaryShaderResource, manager_link) ==
    kPrimaryShaderLinkOffset);

void initialize_registry(std::uint8_t* registry) {
    std::fill_n(registry, 20, std::uint8_t{0});
}

RenderResourceListNode* create_list_sentinel() {
    auto* node = static_cast<RenderResourceListNode*>(
        render_allocate(sizeof(RenderResourceListNode)));
    node->next = node;
    node->previous = node;
    return node;
}

void release_list_sentinel(RenderResourceListNode*& node) {
    if (node == nullptr) {
        return;
    }

    node->next->previous = node->previous;
    node->previous->next = node->next;
    render_release(node);
    node = nullptr;
}

RenderPrimaryShaderResource& primary_shader_from_link(
    RenderResourceListNode& link) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&link);
    return *reinterpret_cast<RenderPrimaryShaderResource*>(
        bytes - kPrimaryShaderLinkOffset);
}

void clear_compiled_objects(RenderManagedObjectArray& objects) {
    for (auto** object = objects.begin; object != objects.end; ++object) {
        if (*object != nullptr) {
            (*object)->dispatch->release_dynamic(*object);
        }
    }
    objects.end = objects.begin;
}

}  // namespace

RenderResourceManager& render_system_resource_manager(RenderSystem& system) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&system);
    return *reinterpret_cast<RenderResourceManager*>(
        bytes + kResourceManagerOffset);
}

// Reconstructed from eboot.elf at 0x63F180.
void render_resource_manager_construct(RenderResourceManager& manager) {
    for (std::size_t index = 0; index < 4; ++index) {
        initialize_registry(manager.registries + index * 20);
    }

    std::fill_n(
        manager.handle_state,
        34,
        std::int64_t{-1});
    for (const auto index : {0U, 1U, 11U, 13U, 20U, 22U, 24U, 27U, 29U}) {
        manager.handle_state[index] = 0;
    }
    std::fill_n(manager.runtime_state, 336, std::uint8_t{0});

    manager.pointer_array = static_cast<RenderResourcePointerArray*>(
        render_allocate(sizeof(RenderResourcePointerArray)));
    *manager.pointer_array = {};
    manager.primary_list = create_list_sentinel();
    manager.secondary_list = create_list_sentinel();
}

// Reconstructed from eboot.elf at 0x63F350.
void render_resource_manager_destruct(RenderResourceManager& manager) {
    release_list_sentinel(manager.primary_list);
    release_list_sentinel(manager.secondary_list);

    if (manager.pointer_array != nullptr) {
        auto& array = *manager.pointer_array;
        if (array.begin != nullptr) {
            const auto byte_count = static_cast<std::size_t>(
                reinterpret_cast<std::uint8_t*>(array.capacity) -
                reinterpret_cast<std::uint8_t*>(array.begin));
            engine_deallocate_sized(array.begin, byte_count);
        }
        render_release(manager.pointer_array);
        manager.pointer_array = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x641F30, with the primary-resource clear
// helper at 0x6388C0 and compiled-array clear at 0x63B210.
void render_resource_manager_reload_shaders(RenderResourceManager& manager) {
    for (auto* node = manager.primary_list->next;
         node != manager.primary_list;
         node = node->next) {
        auto& shader = primary_shader_from_link(*node);
        for (auto& objects : shader.compiled_objects) {
            clear_compiled_objects(objects);
        }
        shader.compiled = false;
    }

    const auto& settings = *render_system_settings(*render_system_instance());
    if (settings.max_partial_framerate_scenes != 0) {
        return;
    }

    for (auto* node = manager.secondary_list->next;
         node != manager.secondary_list;
         node = node->next) {
        auto* bytes = reinterpret_cast<std::uint8_t*>(node);
        bytes[kSecondaryShaderDirtyOffset] = true;
    }
}

}  // namespace rb4
