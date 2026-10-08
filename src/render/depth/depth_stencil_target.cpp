#include "render/depth/depth_stencil_target.h"

#include <cstdint>

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_adapters.h"
#include "render/depth/depth_stencil_target_adapters.h"

namespace rb4 {

namespace {

constexpr std::int32_t kUnassignedAttachment = -1;

}  // namespace

// Reconstructed from eboot.elf at 0x6B2A80.
void render_depth_stencil_target_create(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    bool partial_frame,
    const RenderTargetResourceBlock* reusable_block) {
    auto* reusable_target = reusable_block == nullptr
        ? nullptr
        : render_target_resource_block_depth_stencil_target(*reusable_block);

    auto attachment_index = partial_frame
        ? kUnassignedAttachment
        : render_target_resources_depth_attachment_end(resources);
    if (!partial_frame && reusable_target != nullptr) {
        if (render_depth_stencil_target_has_unassigned_attachment(
                *reusable_target)) {
            attachment_index = kUnassignedAttachment;
        } else {
            reusable_target = nullptr;
        }
    }

    const auto& settings =
        *render_system_settings(*render_system_instance());
    auto* target = render_target_resources_create_depth_stencil_target(
        resources,
        settings.use_40_bit_depth_stencil,
        attachment_index,
        reusable_target,
        !partial_frame);
    render_target_resource_block_depth_stencil_target(block) = target;
    render_target_resources_update_depth_attachment_end(resources, *target);
}

// Reconstructed from the depth/stencil portion of eboot.elf at 0x6AFFE0.
void render_depth_stencil_target_release(RenderTargetResourceBlock& block) {
    auto*& target =
        render_target_resource_block_depth_stencil_target(block);
    if (target != nullptr) {
        render_target_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace rb4
