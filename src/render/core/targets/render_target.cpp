#include "render/core/targets/render_target.h"

#include <cstddef>

#include "render/core/targets/render_target_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kRenderTargetStateSize = 1552;

struct RenderTargetStateTexturePrefix {
    std::uint8_t reserved_0[368];
    RenderTexture* texture;
};

static_assert(
    offsetof(RenderTargetStateTexturePrefix, texture) == 368);

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
        render_target_state_construct(state, state_flags, nullptr);
        target.owned_state = state;
        target.active_state = state;
    }
}

// Reconstructed from eboot.elf at 0x11B2D40.
void render_target_destruct(RenderTarget& target) {
    render_target_set_base_dispatch(target);
    if (target.owns_state) {
        if (target.owned_state != nullptr) {
            render_target_state_delete(target.owned_state);
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
    auto* runtime =
        reinterpret_cast<RenderTargetStateTexturePrefix*>(&state);
    return runtime->texture;
}

// Reconstructed from eboot.elf at 0x11B2E00.
void render_target_set_state(
    RenderTarget& target,
    RenderTargetState* state) {
    target.owned_state = state;
    target.active_state = state;
}

}  // namespace rb4
