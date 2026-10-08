#include "render/lighting/accumulation/light_accumulation_targets.h"

#include "render/core/textures/render_texture.h"
#include "render/lighting/accumulation/light_accumulation_target_factory.h"

namespace rb4 {

namespace {

RenderTexture*& target_slot(
    RenderTargetResources& resources,
    LightAccumulationTargetKind kind) {
    const auto index = static_cast<std::uint32_t>(kind);
    return index < 2
        ? resources.light_accumulation[index]
        : resources.blurred_light_accumulation[index - 2];
}

RenderTexture* target_slot(
    const RenderTargetResources& resources,
    LightAccumulationTargetKind kind) {
    const auto index = static_cast<std::uint32_t>(kind);
    return index < 2
        ? resources.light_accumulation[index]
        : resources.blurred_light_accumulation[index - 2];
}

RenderTexture* reusable_target(
    const RenderTargetResources* resources,
    LightAccumulationTargetKind kind) {
    return resources == nullptr
        ? nullptr
        : target_slot(*resources, kind);
}

void create_target(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources,
    LightAccumulationTargetKind kind) {
    const char* name = "Light Accum Buffer";
    std::uint32_t scale_shift = 0;
    bool allocate_attachment = true;
    switch (kind) {
    case LightAccumulationTargetKind::kPrimary0:
    case LightAccumulationTargetKind::kPrimary1:
        break;
    case LightAccumulationTargetKind::kBlurredHalf:
        name = "Blurred Light Accum Buffer";
        scale_shift = 1;
        allocate_attachment = false;
        break;
    case LightAccumulationTargetKind::kBlurredQuarter:
        name = "Blurred Light Accum Buffer";
        scale_shift = 2;
        allocate_attachment = false;
        break;
    case LightAccumulationTargetKind::kBlurredEighth:
        name = "Blurred Light Accum Buffer";
        scale_shift = 3;
        allocate_attachment = false;
        break;
    }
    target_slot(resources, kind) =
        render_light_accumulation_target_create(
            resources,
            name,
            scale_shift,
            allocate_attachment,
            reusable_target(reusable_resources, kind));
}

void release_target(
    RenderTargetResources& resources,
    LightAccumulationTargetKind kind) {
    auto*& target = target_slot(resources, kind);
    if (target != nullptr) {
        render_texture_release_dynamic(*target);
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
