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

}  // namespace

// Reconstructed from eboot.elf at 0x48A400.
void render_tiled_light_buffers_initialize(RenderLightingSystem& system) {
    const auto& settings =
        *render_system_settings(*render_system_instance());

    if (settings.use_tiled_lighting) {
        render_lighting_set_point_light_buffer(
            system,
            create_tiled_light_buffer(
                208,
                static_cast<std::size_t>(settings.max_point_lights),
                "Point Lights"));
        render_lighting_set_spot_light_buffer(
            system,
            create_tiled_light_buffer(
                352,
                static_cast<std::size_t>(settings.max_spot_lights),
                "Spotlights"));
        render_lighting_set_directional_light_buffer(
            system,
            create_tiled_light_buffer(
                112,
                static_cast<std::size_t>(settings.max_directional_lights),
                "Directional Lights"));
        render_lighting_set_light_probe_buffer(
            system,
            create_tiled_light_buffer(
                96,
                static_cast<std::size_t>(settings.max_light_probes),
                "Light Probes"));

        const auto slice_zero_capacity =
            settings.max_point_lights +
            settings.max_spot_lights +
            settings.max_light_probes;
        render_lighting_set_slice_zero_light_ids_buffer(
            system,
            create_tiled_light_buffer(
                sizeof(std::uint32_t),
                static_cast<std::size_t>(slice_zero_capacity),
                "Slice Zero Ligth Ids"));
    }

    render_lighting_initialize_remaining(system);
}

}  // namespace rb4
