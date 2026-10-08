#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderSystem;

struct RenderBackendResource {
    void* dispatch;
    std::uint8_t storage[336];
};

static_assert(sizeof(RenderBackendResource) == 344);

RenderBackendResource*& render_system_backend_resource(RenderSystem& system);
void render_backend_resource_create(RenderBackendResource*& resource);
void render_backend_resource_release(RenderBackendResource*& resource);

}  // namespace rb4
