#include "render/lighting/ambient_occlusion/ambient_occlusion_target.h"

#include "render/core/targets/render_target_adapters.h"
#include "render/core/targets/render_target_resource_adapters.h"
#include "render/lighting/ambient_occlusion/ambient_occlusion_target_adapters.h"

namespace rb4 {

// Reconstructed from the AO portion of eboot.elf at 0x6B2660.
void render_ambient_occlusion_target_create(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    bool partial_frame,
    const RenderTargetResourceBlock* reusable_block) {
    auto* reusable_target = reusable_block == nullptr
        ? nullptr
        : render_target_resource_block_ambient_occlusion_target(
              *reusable_block);
    render_target_resource_block_ambient_occlusion_target(block) =
        render_target_resources_create_ambient_occlusion_target(
            resources,
            resources.extent,
            reusable_target,
            !partial_frame);
}

// Reconstructed from the AO-target portion of eboot.elf at 0x6AFFE0.
void render_ambient_occlusion_target_release(
    RenderTargetResourceBlock& block) {
    auto*& target =
        render_target_resource_block_ambient_occlusion_target(block);
    if (target != nullptr) {
        render_target_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace rb4
