#include "render/resources/names/render_resource_name.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "core/memory/engine_memory.h"

namespace rb4 {

namespace {

struct RenderResourceNameDispatch {
    void* reserved_0[3];
    void (*reserve)(RenderResourceName& name, std::size_t capacity);
};

struct EmptyStringStorage {
    std::uint32_t capacity;
    char text;
};

void reserve_name(RenderResourceName& name, std::size_t capacity);

RenderResourceNameDispatch g_resource_name_dispatch{
    {},
    reserve_name,
};

EmptyStringStorage g_empty_name{};

std::uint32_t name_capacity(const RenderResourceName& name) {
    return *(reinterpret_cast<const std::uint32_t*>(name.text) - 1);
}

void release_text(RenderResourceName& name) {
    const auto capacity = name_capacity(name);
    if (capacity != 0) {
        auto* allocation =
            reinterpret_cast<std::uint32_t*>(const_cast<char*>(name.text)) - 1;
        engine_deallocate_sized(allocation, capacity + sizeof(std::uint32_t) + 1);
    }
}

// Reconstructed from eboot.elf at 0x2553B0.
void reserve_name(RenderResourceName& name, std::size_t capacity) {
    const auto old_capacity = name_capacity(name);
    if (capacity <= old_capacity) {
        return;
    }

    auto* allocation = static_cast<std::uint8_t*>(
        engine_allocate_sized(capacity + sizeof(std::uint32_t) + 1));
    *reinterpret_cast<std::uint32_t*>(allocation) =
        static_cast<std::uint32_t>(capacity);
    auto* replacement = reinterpret_cast<char*>(
        allocation + sizeof(std::uint32_t));
    std::memcpy(replacement, name.text, old_capacity + 1);
    replacement[capacity] = '\0';
    release_text(name);
    name.text = replacement;
}

}  // namespace

// Reconstructed from eboot.elf at 0x2550C0.
void render_resource_name_construct(
    RenderResourceName& name,
    const char* text) {
    name.dispatch = &g_resource_name_dispatch;
    name.text = &g_empty_name.text;
    if (text == nullptr || text[0] == '\0') {
        return;
    }

    const auto length = std::strlen(text);
    reserve_name(name, length);
    std::memmove(const_cast<char*>(name.text), text, length);
    const_cast<char*>(name.text)[length] = '\0';
}

// Reconstructed from eboot.elf at 0x255550.
void render_resource_name_destruct(RenderResourceName& name) {
    name.dispatch = &g_resource_name_dispatch;
    release_text(name);
    name.text = &g_empty_name.text;
}

}  // namespace rb4
