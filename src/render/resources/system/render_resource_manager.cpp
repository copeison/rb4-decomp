#include "render/resources/system/render_resource_manager.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include "core/memory/engine_memory.h"

namespace rb4 {

namespace {

constexpr std::size_t kResourceManagerOffset = 2544;

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

}  // namespace rb4
