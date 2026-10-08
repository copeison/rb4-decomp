#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct OrbisRenderSystem;
struct OrbisRenderContext;

constexpr std::size_t kOrbisFrameSlotCount = 2;
constexpr std::size_t kOrbisComputeContextCount = 18;
constexpr std::size_t kOrbisTransientFormatCount = 8;

OrbisRenderContext* orbis_render_context_create(
    OrbisRenderSystem& system);
void orbis_render_context_construct(OrbisRenderContext& context);
void orbis_render_context_destruct(OrbisRenderContext& context);
void orbis_render_context_delete(OrbisRenderContext& context);
void orbis_render_context_create_gfx_contexts(
    OrbisRenderContext& context);
void orbis_render_context_create_gpu_timestamp_pool(
    OrbisRenderContext& context);

}  // namespace rb4
