#include "render/lighting/ambient_occlusion/ambient_occlusion_target.h"

#include "render/core/targets/render_target_resource_factory.h"
#include "render/core/textures/render_data_format.h"
#include "render/core/textures/render_texture.h"
#include "render/core/textures/render_texture_adapters.h"

namespace rb4 {

// Reconstructed from the AO portion of eboot.elf at 0x6B2660.
void render_ambient_occlusion_target_create(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    bool partial_frame,
    const RenderTargetResourceBlock* reusable_block) {
    auto* reusable_target = reusable_block == nullptr
        ? nullptr
        : reusable_block->ambient_occlusion;
    RenderTextureCreationState creation_state{};
    creation_state.values[6] = 1;
    creation_state.values[8] = static_cast<std::uint32_t>(
        render_texture_default_address_mode(28));
    creation_state.values[9] = static_cast<std::uint32_t>(
        render_texture_default_filter_mode(28));
    creation_state.values[10] = 10;
    const RenderDataFormatDescriptor format_descriptor{
        32, 10, 2, 1, -1,
    };
    auto* target = render_target_resources_create_texture_2d(
        resources,
        "AO Buffer",
        creation_state,
        render_data_format_resolve(format_descriptor, 7),
        resources.extent,
        -1,
        0,
        reusable_target);
    block.ambient_occlusion = target;
    if (!partial_frame) {
        resources.registered_resources_begin[
            resources.registered_resource_count++] = target;
    }
}

// Reconstructed from the AO-target portion of eboot.elf at 0x6AFFE0.
void render_ambient_occlusion_target_release(
    RenderTargetResourceBlock& block) {
    auto*& target = block.ambient_occlusion;
    if (target != nullptr) {
        render_texture_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace rb4
