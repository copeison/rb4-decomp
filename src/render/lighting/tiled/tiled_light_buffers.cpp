#include "render/lighting/tiled/tiled_light_buffers.h"

#include <cstddef>
#include <cstdint>

#include "render/core/buffers/render_compute_buffer.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/lighting/tiled/tiled_light_buffer_adapters.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kTiledLightBufferFlags = 0x12;

RenderComputeBuffer* create_tiled_light_buffer(
    std::size_t element_stride,
    std::size_t element_count,
    const char* name) {
    RenderComputeBufferDescriptor descriptor{};
    descriptor.element_stride = element_stride;
    descriptor.element_count = element_count;
    descriptor.flags = kTiledLightBufferFlags;
    descriptor.name = name;
    return render_create_compute_buffer(descriptor);
}

void set_tiled_light_buffer(
    RenderLightingSystem& system,
    TiledLightBufferKind kind,
    RenderComputeBuffer* buffer) {
    render_lighting_tiled_light_buffer(system, kind) = buffer;
}

void release_tiled_light_buffer(
    RenderLightingSystem& system,
    TiledLightBufferKind kind) {
    auto*& buffer = render_lighting_tiled_light_buffer(system, kind);
    if (buffer != nullptr) {
        render_compute_buffer_release_dynamic(*buffer);
        buffer = nullptr;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x48A400.
void render_tiled_light_buffers_initialize(RenderLightingSystem& system) {
    const auto& settings =
        *render_system_settings(*render_system_instance());

    if (settings.use_tiled_lighting) {
        set_tiled_light_buffer(
            system,
            TiledLightBufferKind::kPointLights,
            create_tiled_light_buffer(
                208,
                static_cast<std::size_t>(settings.max_point_lights),
                "Point Lights"));
        set_tiled_light_buffer(
            system,
            TiledLightBufferKind::kSpotLights,
            create_tiled_light_buffer(
                352,
                static_cast<std::size_t>(settings.max_spot_lights),
                "Spotlights"));
        set_tiled_light_buffer(
            system,
            TiledLightBufferKind::kDirectionalLights,
            create_tiled_light_buffer(
                112,
                static_cast<std::size_t>(settings.max_directional_lights),
                "Directional Lights"));
        set_tiled_light_buffer(
            system,
            TiledLightBufferKind::kLightProbes,
            create_tiled_light_buffer(
                96,
                static_cast<std::size_t>(settings.max_light_probes),
                "Light Probes"));

        const auto slice_zero_capacity =
            settings.max_point_lights +
            settings.max_spot_lights +
            settings.max_light_probes;
        set_tiled_light_buffer(
            system,
            TiledLightBufferKind::kSliceZeroLightIds,
            create_tiled_light_buffer(
                sizeof(std::uint32_t),
                static_cast<std::size_t>(slice_zero_capacity),
                "Slice Zero Ligth Ids"));
    }

    render_lighting_initialize_remaining(system);
}

// Reconstructed from the tiled-light portion of eboot.elf at 0x480AD0.
void render_tiled_light_buffers_release(RenderLightingSystem& system) {
    release_tiled_light_buffer(system, TiledLightBufferKind::kPointLights);
    release_tiled_light_buffer(system, TiledLightBufferKind::kSpotLights);
    release_tiled_light_buffer(system, TiledLightBufferKind::kDirectionalLights);
    release_tiled_light_buffer(system, TiledLightBufferKind::kLightProbes);
    release_tiled_light_buffer(system, TiledLightBufferKind::kSliceZeroLightIds);
}

}  // namespace rb4
