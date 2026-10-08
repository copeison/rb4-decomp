#include "render/lighting/accumulation/light_accumulation_targets.h"

#include "render/core/targets/render_target_adapters.h"
#include "render/lighting/accumulation/light_accumulation_target_adapters.h"

namespace rb4 {

namespace {

RenderTarget* reusable_target(
    const RenderTargetResources* resources,
    LightAccumulationTargetKind kind) {
    return resources == nullptr
        ? nullptr
        : render_target_resources_light_accumulation_target(*resources, kind);
}

void create_target(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources,
    LightAccumulationTargetKind kind) {
    render_target_resources_light_accumulation_target(resources, kind) =
        render_target_resources_create_light_accumulation_target(
            resources,
            kind,
            reusable_target(reusable_resources, kind));
}

void release_target(
    RenderTargetResources& resources,
    LightAccumulationTargetKind kind) {
    auto*& target =
        render_target_resources_light_accumulation_target(resources, kind);
    if (target != nullptr) {
        render_target_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B0B20.
void render_light_accumulation_targets_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources) {
    create_target(
        resources, reusable_resources, LightAccumulationTargetKind::kPrimary0);
    create_target(
        resources, reusable_resources, LightAccumulationTargetKind::kPrimary1);
    create_target(
        resources,
        reusable_resources,
        LightAccumulationTargetKind::kBlurredHalf);
    create_target(
        resources,
        reusable_resources,
        LightAccumulationTargetKind::kBlurredQuarter);
    create_target(
        resources,
        reusable_resources,
        LightAccumulationTargetKind::kBlurredEighth);
}

// Reconstructed from the light-accumulation portion of eboot.elf at 0x6AFFE0.
void render_light_accumulation_targets_release(
    RenderTargetResources& resources) {
    release_target(resources, LightAccumulationTargetKind::kPrimary0);
    release_target(resources, LightAccumulationTargetKind::kPrimary1);
    release_target(resources, LightAccumulationTargetKind::kBlurredHalf);
    release_target(resources, LightAccumulationTargetKind::kBlurredQuarter);
    release_target(resources, LightAccumulationTargetKind::kBlurredEighth);
}

}  // namespace rb4
