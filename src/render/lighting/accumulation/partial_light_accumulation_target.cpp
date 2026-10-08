#include "render/lighting/accumulation/partial_light_accumulation_target.h"

#include "render/core/targets/render_target_adapters.h"
#include "render/lighting/accumulation/partial_light_accumulation_target_adapters.h"

namespace rb4 {

// Reconstructed from the partial-light portion of eboot.elf at 0x6B2660.
void render_partial_light_accumulation_target_create(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    const RenderTargetResourceBlock* reusable_block) {
    auto* reusable_target = reusable_block == nullptr
        ? nullptr
        : render_target_resource_block_partial_light_accumulation_target(
              *reusable_block);
    render_target_resource_block_partial_light_accumulation_target(block) =
        render_target_resources_create_partial_light_accumulation_target(
            resources, reusable_target);
}

// Reconstructed from the partial-light portion of eboot.elf at 0x6AFFE0.
void render_partial_light_accumulation_target_release(
    RenderTargetResourceBlock& block) {
    auto*& target =
        render_target_resource_block_partial_light_accumulation_target(block);
    if (target != nullptr) {
        render_target_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace rb4
