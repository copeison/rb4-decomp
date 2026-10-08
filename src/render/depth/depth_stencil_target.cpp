#include "render/depth/depth_stencil_target.h"

#include <cstdint>

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture.h"
#include "render/core/textures/render_texture_adapters.h"
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
        : reusable_block->depth_stencil;

    auto attachment_index = partial_frame
        ? kUnassignedAttachment
        : static_cast<std::int32_t>(resources.attachment_cursor);
    if (!partial_frame && reusable_target != nullptr) {
        if (reusable_target->attachment_index == kUnassignedAttachment) {
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
    block.depth_stencil = target;
    if (target->attachment_index != kUnassignedAttachment) {
        resources.attachment_cursor = static_cast<std::uint32_t>(
            target->attachment_index + target->attachment_count);
    }
}

// Reconstructed from the depth/stencil portion of eboot.elf at 0x6AFFE0.
void render_depth_stencil_target_release(RenderTargetResourceBlock& block) {
    auto*& target = block.depth_stencil;
    if (target != nullptr) {
        render_texture_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace rb4
