#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderSystem;

struct FogDeferredShaderResource {
    void* dispatch;
    std::uint8_t storage[336];
};

static_assert(sizeof(FogDeferredShaderResource) == 344);

FogDeferredShaderResource*& render_system_fog_deferred_shader(
    RenderSystem& system);
void fog_deferred_shader_create(FogDeferredShaderResource*& shader);
void fog_deferred_shader_release(FogDeferredShaderResource*& shader);

}  // namespace rb4
