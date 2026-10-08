#include "render/lighting/tiled/tiled_light_target_buffers.h"

#include <cstddef>

#include "render/core/buffers/render_compute_buffer.h"
#include "render/core/buffers/render_compute_buffer_adapters.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture_adapters.h"
#include "render/lighting/tiled/tiled_light_target_buffer_adapters.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kLightIndexStride = sizeof(std::uint32_t);
constexpr std::uint32_t kLightRangeStride = 32;
constexpr std::uint32_t kTargetBufferFlags = 1;

std::size_t divide_round_up(std::size_t value, std::size_t divisor) {
    return value / divisor + (value % divisor != 0);
}

RenderComputeBuffer* create_target_buffer(
    std::size_t element_stride,
    std::size_t element_count,
    const char* name) {
    RenderComputeBufferDescriptor descriptor{};
    descriptor.element_stride = element_stride;
    descriptor.element_count = element_count;
    descriptor.flags = kTargetBufferFlags;
    descriptor.name = name;
    return render_create_compute_buffer(descriptor);
}

void create_light_index_and_range_buffers(
    RenderComputeBuffer* (&light_ids)[2],
    RenderComputeBuffer*& light_id_ranges,
    std::size_t tile_count,
    std::size_t max_lights_per_tile,
    const char* id_buffer_name,
    const char* range_buffer_name) {
    // Each uint32 element holds two 16-bit light IDs.
    const auto light_id_element_count =
        tile_count * max_lights_per_tile / 2;
    light_ids[0] = create_target_buffer(
        kLightIndexStride, light_id_element_count, id_buffer_name);
    light_ids[1] = create_target_buffer(
        kLightIndexStride, light_id_element_count, id_buffer_name);
    light_id_ranges = create_target_buffer(
        kLightRangeStride, tile_count, range_buffer_name);
}

void release_compute_buffer(RenderComputeBuffer*& buffer) {
    if (buffer != nullptr) {
        render_compute_buffer_release_dynamic(*buffer);
        buffer = nullptr;
    }
}

void release_render_target(RenderTexture*& target) {
    if (target != nullptr) {
        render_texture_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B3380.
void render_tiled_light_target_buffers_create(
    TiledLightTargetResources& resources,
    RenderExtent extent,
    bool create_interpolation_target,
    bool stereo,
    RenderTexture* existing_interpolation_target) {
    const auto& settings =
        *render_system_settings(*render_system_instance());
    if (!settings.use_tiled_lighting) {
        return;
    }

    const auto tile_size = static_cast<std::size_t>(settings.light_tile_size);
    const auto tile_count =
        divide_round_up(extent.width, tile_size) *
        divide_round_up(extent.height, tile_size) *
        static_cast<std::size_t>(settings.light_tile_depth_slices);
    const auto max_lights_per_tile =
        static_cast<std::size_t>(settings.max_lights_per_tile);

    create_light_index_and_range_buffers(
        resources.light_ids,
        resources.light_id_ranges,
        tile_count,
        max_lights_per_tile,
        "Tiled Light Ids",
        "Tiled Light Id Ranges");

    if (create_interpolation_target) {
        const RenderExtent interpolation_extent{
            static_cast<std::uint32_t>(
                2 * divide_round_up(extent.width, std::size_t{2})),
            static_cast<std::uint32_t>(
                divide_round_up(extent.height, std::size_t{2})),
        };
        resources.interpolation_target =
            render_create_tiled_light_interpolation_target(
                interpolation_extent, existing_interpolation_target);
    }

    if (stereo) {
        create_light_index_and_range_buffers(
            resources.stereo_light_ids,
            resources.stereo_light_id_ranges,
            tile_count,
            max_lights_per_tile,
            "Tiled Light Ids (Both Eyes)",
            "Tiled Light Id Ranges (Both Eyes)");
    }
}

// Reconstructed from the tiled-light portion of eboot.elf at 0x6AFFE0.
void render_tiled_light_target_buffers_release(
    TiledLightTargetResources& resources) {
    release_compute_buffer(resources.light_ids[0]);
    release_compute_buffer(resources.stereo_light_ids[0]);
    release_compute_buffer(resources.light_ids[1]);
    release_compute_buffer(resources.stereo_light_ids[1]);
    release_compute_buffer(resources.light_id_ranges);
    release_render_target(resources.interpolation_target);
    release_compute_buffer(resources.stereo_light_id_ranges);
}

}  // namespace rb4
