#include "render/lighting/accumulation/partial_light_accumulation_target.h"

#include "render/core/textures/render_texture.h"
#include "render/lighting/accumulation/light_accumulation_target_factory.h"

namespace rb4 {

// Reconstructed from the partial-light portion of eboot.elf at 0x6B2660.
void render_partial_light_accumulation_target_create(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    const RenderTargetResourceBlock* reusable_block) {
    auto* reusable_target = reusable_block == nullptr
        ? nullptr
        : reusable_block->partial_light_accumulation;
    block.partial_light_accumulation =
        render_light_accumulation_target_create(
            resources,
            "Partial Light Accum Buffer",
            0,
            false,
            reusable_target);
}

// Reconstructed from the partial-light portion of eboot.elf at 0x6AFFE0.
void render_partial_light_accumulation_target_release(
    RenderTargetResourceBlock& block) {
    auto*& target = block.partial_light_accumulation;
    if (target != nullptr) {
        render_texture_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace rb4
