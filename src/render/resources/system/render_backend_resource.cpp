#include "render/resources/system/render_backend_resource.h"

#include <cstddef>
#include <cstdint>

#include "core/memory/engine_memory.h"
#include "render/resources/system/render_backend_resource_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kBackendResourceOffset = 3560;

struct RenderBackendResourceDispatch {
    void* reserved_0;
    void (*release_dynamic)(RenderBackendResource* resource);
};

}  // namespace

RenderBackendResource*& render_system_backend_resource(RenderSystem& system) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&system);
    return *reinterpret_cast<RenderBackendResource**>(
        bytes + kBackendResourceOffset);
}

// Reconstructed from eboot.elf at 0x451C90.
void render_backend_resource_create(RenderBackendResource*& resource) {
    auto* created = static_cast<RenderBackendResource*>(
        render_allocate(sizeof(RenderBackendResource)));
    render_backend_resource_construct_and_register(*created);
    resource = created;
}

// Reconstructed from eboot.elf at 0x451CC0.
void render_backend_resource_release(RenderBackendResource*& resource) {
    if (resource != nullptr) {
        auto* dispatch = static_cast<RenderBackendResourceDispatch*>(
            resource->dispatch);
        dispatch->release_dynamic(resource);
        resource = nullptr;
    }
}

}  // namespace rb4
