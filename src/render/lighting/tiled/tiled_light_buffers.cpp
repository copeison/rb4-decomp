#include "render/lighting/tiled/tiled_light_buffers.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "render/buffers/RndComputeBuffer.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_data_format.h"
#include "render/core/textures/render_texture_array_2d.h"
#include "render/core/textures/render_texture_mip_chain.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kTiledLightBufferFlags = 0x12;
constexpr std::size_t kSpotShadowConfigurationsOffset = 152;
constexpr std::size_t kSpotShadowConfigurationIndexOffset = 204;
constexpr std::size_t kSpotShadowDepthActiveOffset = 1048;
constexpr std::size_t kSpotShadowDepthArrayOffset = 1056;

struct SpotShadowConfiguration {
    std::uint64_t layer_count;
    std::uint32_t resolution_index;
    std::uint32_t reserved_12;
};

static_assert(sizeof(SpotShadowConfiguration) == 16);

constexpr std::array<std::size_t, 5> kTiledLightBufferOffsets{
    1688,
    1712,
    1736,
    1760,
    1768,
};

constexpr std::array<std::uint32_t, 7> kSpotShadowResolutionSizes{
    256,
    512,
    1024,
    1600,
    2048,
    3200,
    4096,
};

std::uint8_t* lighting_system_bytes(RenderLightingSystem& system) {
    return reinterpret_cast<std::uint8_t*>(&system);
}

RndComputeBuffer*& tiled_light_buffer(
    RenderLightingSystem& system,
    TiledLightBufferKind kind) {
    const auto index = static_cast<std::size_t>(kind);
    return *reinterpret_cast<RndComputeBuffer**>(
        lighting_system_bytes(system) + kTiledLightBufferOffsets[index]);
}

SpotShadowConfiguration& active_spot_shadow_configuration(
    RenderLightingSystem& system) {
    auto* bytes = lighting_system_bytes(system);
    auto* configurations = *reinterpret_cast<SpotShadowConfiguration**>(
        bytes + kSpotShadowConfigurationsOffset);
    const auto index = *reinterpret_cast<std::int32_t*>(
        bytes + kSpotShadowConfigurationIndexOffset);
    return configurations[index];
}

bool& spot_shadow_depth_active(RenderLightingSystem& system) {
    return *reinterpret_cast<bool*>(
        lighting_system_bytes(system) + kSpotShadowDepthActiveOffset);
}

RenderTextureArray2D*& spot_shadow_depth_array(
    RenderLightingSystem& system) {
    return *reinterpret_cast<RenderTextureArray2D**>(
        lighting_system_bytes(system) + kSpotShadowDepthArrayOffset);
}

RndComputeBuffer* create_tiled_light_buffer(
    std::size_t element_stride,
    std::size_t element_count,
    const char* name) {
    RndComputeBuffer::Description descriptor{};
    descriptor.mElementSize = element_stride;
    descriptor.mNumElements = element_count;
    descriptor.mFlags = kTiledLightBufferFlags;
    descriptor.mName = name;
    return RndComputeBuffer::New(descriptor);
}

void set_tiled_light_buffer(
    RenderLightingSystem& system,
    TiledLightBufferKind kind,
    RndComputeBuffer* buffer) {
    tiled_light_buffer(system, kind) = buffer;
}

void release_tiled_light_buffer(
    RenderLightingSystem& system,
    TiledLightBufferKind kind) {
    auto*& buffer = tiled_light_buffer(system, kind);
    if (buffer != nullptr) {
        delete buffer;
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

    render_lighting_rebuild_spot_shadow_depth_array(system);
}

// Reconstructed from eboot.elf at 0x48AB30.
void render_lighting_rebuild_spot_shadow_depth_array(
    RenderLightingSystem& system) {
    spot_shadow_depth_active(system) = false;
    auto*& texture = spot_shadow_depth_array(system);
    if (texture != nullptr) {
        render_texture_release_dynamic(*texture);
        texture = nullptr;
    }

    const auto& configuration = active_spot_shadow_configuration(system);
    if (configuration.layer_count == 0) {
        return;
    }

    const RenderDataFormatDescriptor depth_format{
        16,
        12,
        0,
        1,
        -1,
    };
    const auto data_format = render_data_format_resolve(depth_format, 7);
    const auto resolution = configuration.resolution_index <
            kSpotShadowResolutionSizes.size()
        ? kSpotShadowResolutionSizes[configuration.resolution_index]
        : std::numeric_limits<std::uint32_t>::max();

    RenderTextureArray2DDescriptor descriptor;
    render_texture_array_2d_descriptor_construct(descriptor);
    descriptor.texture_state.creation_state.values[0] = 2;
    descriptor.texture_state.creation_state.values[6] = 1;
    descriptor.texture_state.creation_state.values[8] = 1;
    descriptor.texture_state.creation_state.values[9] = 1;
    descriptor.texture_state.creation_state.values[10] = 2;
    descriptor.texture_state.name = "Spot Shadow Depth TexArray";

    const auto layer_count = static_cast<std::size_t>(
        configuration.layer_count);
    auto* mip_chains = new RenderTextureMipChainDescriptor[layer_count];
    const RenderTextureExtent3D extent{resolution, resolution, 1};
    for (std::size_t index = 0; index < layer_count; ++index) {
        render_texture_mip_chain_descriptor_construct(mip_chains[index]);
        render_texture_mip_chain_descriptor_initialize(
            mip_chains[index], extent, data_format);
    }
    descriptor.mip_chains = {
        mip_chains,
        mip_chains + layer_count,
        mip_chains + layer_count,
    };

    texture = render_create_texture_array_2d(descriptor);
    for (std::size_t index = 0; index < layer_count; ++index) {
        render_texture_mip_chain_descriptor_destruct(mip_chains[index]);
    }
    delete[] mip_chains;
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
