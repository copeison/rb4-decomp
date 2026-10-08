#include "render/core/targets/render_target.h"

#include <cstddef>

#include "core/memory/engine_memory.h"
#include "render/core/targets/render_target_adapters.h"
#include "render/core/targets/render_target_resources.h"
#include "render/core/targets/render_target_resources_lifecycle.h"

namespace rb4 {

namespace {

constexpr std::size_t kRenderTargetStateSize = 1552;

static_assert(kRenderTargetStateSize == sizeof(RenderTargetResources));
static_assert(
    offsetof(RenderTargetState, state_flags) ==
    offsetof(RenderTargetResources, flags));
static_assert(
    offsetof(RenderTargetState, reserved_12) ==
    offsetof(RenderTargetResources, resource_mode));
static_assert(
    offsetof(RenderTargetState, draw_mode) ==
    offsetof(RenderTargetResources, reserved_10));
static_assert(
    offsetof(RenderTargetState, width) ==
    offsetof(RenderTargetResources, extent));

RenderTargetResources& target_resources(RenderTargetState& state) {
    return reinterpret_cast<RenderTargetResources&>(state);
}

// Reconstructed from eboot.elf at 0x6B40A0.
void construct_target_state(
    RenderTargetState& state,
    std::uint32_t state_flags,
    std::int32_t resource_mode) {
    auto& resources = target_resources(state);
    render_target_resources_construct(
        resources, state_flags, resource_mode);
    render_target_resources_set_concrete_dispatch(resources);
}

// Reconstructed from eboot.elf at 0x6B40E0.
void delete_target_state(RenderTargetState& state) {
    render_target_resources_destruct(target_resources(state));
    render_release(&state);
}

}  // namespace

// Reconstructed from eboot.elf at 0x11B2CD0.
void render_target_construct(
    RenderTarget& target,
    std::uint32_t state_flags,
    bool create_state) {
    render_target_set_base_dispatch(target);
    target.attachment_index = -1;
    target.owns_state = create_state;
    target.owned_state = nullptr;
    target.active_state = nullptr;

    if (create_state) {
        auto* state = static_cast<RenderTargetState*>(
            render_allocate(kRenderTargetStateSize));
        construct_target_state(*state, state_flags, 0);
        target.owned_state = state;
        target.active_state = state;
    }
}

// Reconstructed from eboot.elf at 0x11B2D40.
void render_target_destruct(RenderTarget& target) {
    render_target_set_base_dispatch(target);
    if (target.owns_state) {
        if (target.owned_state != nullptr) {
            delete_target_state(*target.owned_state);
        }
        target.owned_state = nullptr;
        target.active_state = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x11B2D90.
void render_target_delete(RenderTarget& target) {
    render_target_destruct(target);
    render_delete_target_storage(target);
}

// Reconstructed from eboot.elf at 0x11B2DE0.
std::size_t render_target_active_buffer_index(const RenderTarget&) {
    return 0;
}

// Reconstructed from eboot.elf at 0x11B2DF0.
RenderTargetStateHandle render_target_active_state_handle(
    RenderTarget& target) {
    return {&target.active_state, 1};
}

RenderTexture* render_target_state_texture(RenderTargetState& state) {
    return target_resources(state).source_texture;
}

// Reconstructed from eboot.elf at 0x11B2E00.
void render_target_set_state(
    RenderTarget& target,
    RenderTargetState* state) {
    target.owned_state = state;
    target.active_state = state;
}

}  // namespace rb4
