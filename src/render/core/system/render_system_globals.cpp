#include "render/core/system/render_system_globals.h"

#include "render/core/context/render_context_adapters.h"
#include "render/core/system/render_system_frame_adapters.h"
#include "render/core/system/render_system_state.h"

namespace rb4 {

RenderSystem* g_render_system = nullptr;

RenderSystem* render_system_instance() {
    return g_render_system;
}

RenderFrameOwner* render_system_frame_owner(RenderSystem& system) {
    return render_system_core_state(system).frame_owner;
}

RenderFrameOwner& render_system_primary_frame_owner(RenderSystem& system) {
    return *render_system_core_state(system).primary_frame_owner;
}

std::size_t render_system_frame_owner_count(const RenderSystem& system) {
    const auto& runtime = render_system_core_state(system);
    return static_cast<std::size_t>(
        runtime.frame_owners.end - runtime.frame_owners.begin);
}

RenderFrameOwner& render_system_frame_owner_at(
    RenderSystem& system,
    std::size_t index) {
    return *render_system_core_state(system).frame_owners.begin[index];
}

bool render_system_has_pending_frame(const RenderSystem& system) {
    return render_system_core_state(system).frame_activation_pending;
}

// Reconstructed from eboot.elf at 0x3DEF20.
void render_system_activate_pending_frame(RenderSystem& system) {
    auto& runtime = render_system_core_state(system);
    render_context_begin_frame(
        *runtime.render_context, runtime.frame_activation_flags);
    runtime.frame_activation_pending = false;
    runtime.frame_activation_flags = 0;
}

RenderSettings* render_system_settings(RenderSystem& system) {
    return render_system_core_state(system).settings;
}

void render_system_set_settings(
    RenderSystem& system,
    RenderSettings* settings) {
    render_system_core_state(system).settings = settings;
}

RenderFactory* render_system_factory(RenderSystem& system) {
    return render_system_core_state(system).factory;
}

// Reconstructed from eboot.elf at 0x3DEDB0.
void render_system_set_factory(
    RenderSystem& system,
    RenderFactory* factory) {
    render_system_core_state(system).factory = factory;
}

// Reconstructed from eboot.elf at 0x3DED80.
void render_system_release_back_buffer(RenderSystem& system) {
    auto& runtime = render_system_core_state(system);
    if (runtime.frame_owner != nullptr) {
        render_frame_owner_delete(*runtime.frame_owner);
        runtime.frame_owner = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x3DEEA0.
void render_system_release_render_contexts(RenderSystem& system) {
    auto& runtime = render_system_core_state(system);
    if (runtime.primary_frame_owner != nullptr) {
        render_frame_owner_delete(*runtime.primary_frame_owner);
        runtime.primary_frame_owner = nullptr;
    }

    while (runtime.frame_owners.end != runtime.frame_owners.begin) {
        --runtime.frame_owners.end;
        auto* context = *runtime.frame_owners.end;
        if (context != nullptr) {
            render_frame_owner_delete(*context);
        }
    }
}

void render_system_publish_instance(RenderSystem& system) {
    g_render_system = &system;
}

void render_system_clear_instance() {
    g_render_system = nullptr;
}

}  // namespace rb4
