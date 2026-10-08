#include "render/core/targets/render_target_resource_factory.h"

#include <cstring>

#include "core/memory/engine_memory.h"
#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_resources.h"
#include "render/core/textures/render_texture.h"
#include "render/core/textures/render_texture_2d.h"
#include "render/core/textures/render_texture_adapters.h"
#include "render/core/textures/render_texture_array_2d.h"

namespace rb4 {

namespace {

constexpr std::size_t kDescriptorTypeOffset = 0;
constexpr std::size_t kCreationStateOffset = 4;
constexpr std::size_t kResolvedStateOffset = 48;
constexpr std::size_t kDataFormatOffset = 92;
constexpr std::size_t kTargetFlagsOffset = 120;
constexpr std::size_t kAttachmentIndexOffset = 128;
constexpr std::size_t kNameOffset = 136;

template <typename T>
void write_value(std::uint8_t* destination, std::size_t offset, T value) {
    std::memcpy(destination + offset, &value, sizeof(value));
}

void initialize_texture_descriptor(
    std::uint8_t (&state)[144],
    std::int32_t descriptor_type,
    const RenderTextureCreationState& creation_state,
    const char* name,
    std::int32_t attachment_index,
    std::uint32_t target_flags) {
    std::memset(state, 0, sizeof(state));
    write_value(state, kDescriptorTypeOffset, descriptor_type);
    std::memcpy(
        state + kCreationStateOffset,
        creation_state.values,
        sizeof(creation_state.values));
    write_value<std::int32_t>(state, kDataFormatOffset, -1);
    write_value(state, kTargetFlagsOffset, target_flags);
    write_value(state, kAttachmentIndexOffset, attachment_index);
    write_value(state, kNameOffset, name);
}

void initialize_mip_descriptor(
    RenderTextureMipChainDescriptor& mip,
    RenderExtent extent,
    std::int32_t data_format) {
    mip = {};
    mip.fields.width = extent.width;
    mip.fields.height = extent.height;
    mip.fields.depth = 1;
    mip.fields.data_format = data_format;
}

void resolve_descriptor(
    std::uint8_t (&state)[144],
    std::int32_t descriptor_type) {
    render_texture_resolve_descriptor_fields(
        state + kResolvedStateOffset,
        descriptor_type,
        state + kCreationStateOffset,
        -1);
}

RenderFactory& render_factory() {
    return *render_system_factory(*render_system_instance());
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B4120.
RenderTexture* render_target_resources_create_texture_2d(
    RenderTargetResources&,
    const char* name,
    const RenderTextureCreationState& creation_state,
    std::int32_t data_format,
    RenderExtent extent,
    std::int32_t attachment_index,
    std::uint32_t target_flags,
    RenderTexture* reusable_texture) {
    RenderTexture2DDescriptor descriptor{};
    initialize_texture_descriptor(
        descriptor.texture_state,
        1,
        creation_state,
        name,
        attachment_index,
        target_flags);
    initialize_mip_descriptor(
        descriptor.mip_chain, extent, data_format);
    resolve_descriptor(descriptor.texture_state, 1);

    auto* texture = render_factory_create_texture_2d(
        render_factory(), descriptor);
    render_texture_initialize_backend(*texture, reusable_texture);
    return texture;
}

// Reconstructed from eboot.elf at 0x6B41F0.
RenderTexture* render_target_resources_create_texture_array_2d(
    RenderTargetResources&,
    const char* name,
    const RenderTextureCreationState& creation_state,
    std::int32_t data_format,
    RenderExtent extent,
    std::size_t layer_count,
    std::int32_t attachment_index,
    std::uint32_t target_flags,
    RenderTexture* reusable_texture) {
    RenderTextureArray2DDescriptor descriptor{};
    initialize_texture_descriptor(
        descriptor.texture_state,
        5,
        creation_state,
        name,
        attachment_index,
        target_flags);

    auto* mip_chains = static_cast<RenderTextureMipChainDescriptor*>(
        engine_allocate_sized(
            layer_count * sizeof(RenderTextureMipChainDescriptor)));
    for (std::size_t index = 0; index < layer_count; ++index) {
        initialize_mip_descriptor(
            mip_chains[index], extent, data_format);
    }
    descriptor.mip_chains = {
        mip_chains,
        mip_chains + layer_count,
        mip_chains + layer_count,
    };
    resolve_descriptor(descriptor.texture_state, 5);

    auto* texture = render_factory_create_texture_array_2d(
        render_factory(), descriptor);
    render_texture_initialize_backend(*texture, reusable_texture);
    if (mip_chains != nullptr) {
        engine_deallocate_sized(
            mip_chains,
            layer_count * sizeof(RenderTextureMipChainDescriptor));
    }
    return texture;
}

}  // namespace rb4
