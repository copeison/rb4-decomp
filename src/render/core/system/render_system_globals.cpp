#include "render/core/system/render_system_globals.h"

#include <cstddef>
#include <cstdint>

#include "render/core/context/render_context_adapters.h"
#include "render/core/system/render_system_frame_adapters.h"

namespace rb4 {

RenderSystem* g_render_system = nullptr;

namespace {

struct RenderFrameOwnerArray {
    RenderFrameOwner** begin;
    RenderFrameOwner** end;
    RenderFrameOwner** capacity;
    void* allocator;
    void* allocator_state;
};

struct RenderSystemFramePrefix {
    std::uint8_t reserved_0[56];
    union {
        RenderContext* render_context;
        RenderFrameOwner* primary_frame_owner;
    };
    bool frame_activation_pending;
    std::uint8_t reserved_65[3];
    std::uint32_t frame_activation_flags;
    RenderFrameOwnerArray frame_owners;
    RenderFrameOwner* frame_owner;
    std::uint8_t reserved_120[176];
    RenderSettings* settings;
    RenderFactory* factory;
};

static_assert(
    offsetof(RenderSystemFramePrefix, primary_frame_owner) == 56);
static_assert(
    offsetof(RenderSystemFramePrefix, frame_activation_pending) == 64);
static_assert(
    offsetof(RenderSystemFramePrefix, frame_activation_flags) == 68);
static_assert(sizeof(RenderFrameOwnerArray) == 40);
static_assert(offsetof(RenderSystemFramePrefix, frame_owners) == 72);
static_assert(offsetof(RenderSystemFramePrefix, frame_owner) == 112);
static_assert(offsetof(RenderSystemFramePrefix, settings) == 296);
static_assert(offsetof(RenderSystemFramePrefix, factory) == 304);

}  // namespace

RenderSystem* render_system_instance() {
    return g_render_system;
}

RenderFrameOwner* render_system_frame_owner(RenderSystem& system) {
    auto* runtime = reinterpret_cast<RenderSystemFramePrefix*>(&system);
    return runtime->frame_owner;
}

RenderFrameOwner& render_system_primary_frame_owner(RenderSystem& system) {
    auto* runtime = reinterpret_cast<RenderSystemFramePrefix*>(&system);
    return *runtime->primary_frame_owner;
}

std::size_t render_system_frame_owner_count(const RenderSystem& system) {
    const auto* runtime =
        reinterpret_cast<const RenderSystemFramePrefix*>(&system);
    return static_cast<std::size_t>(
        runtime->frame_owners.end - runtime->frame_owners.begin);
}

RenderFrameOwner& render_system_frame_owner_at(
    RenderSystem& system,
    std::size_t index) {
    auto* runtime = reinterpret_cast<RenderSystemFramePrefix*>(&system);
    return *runtime->frame_owners.begin[index];
}

bool render_system_has_pending_frame(const RenderSystem& system) {
    const auto* runtime =
        reinterpret_cast<const RenderSystemFramePrefix*>(&system);
    return runtime->frame_activation_pending;
}

// Reconstructed from eboot.elf at 0x3DEF20.
void render_system_activate_pending_frame(RenderSystem& system) {
    auto* runtime = reinterpret_cast<RenderSystemFramePrefix*>(&system);
    render_context_begin_frame(
        *runtime->render_context, runtime->frame_activation_flags);
    runtime->frame_activation_pending = false;
    runtime->frame_activation_flags = 0;
}

RenderSettings* render_system_settings(RenderSystem& system) {
    auto* runtime = reinterpret_cast<RenderSystemFramePrefix*>(&system);
    return runtime->settings;
}

void render_system_set_settings(
    RenderSystem& system,
    RenderSettings* settings) {
    auto* runtime = reinterpret_cast<RenderSystemFramePrefix*>(&system);
    runtime->settings = settings;
}

RenderFactory* render_system_factory(RenderSystem& system) {
    auto* runtime = reinterpret_cast<RenderSystemFramePrefix*>(&system);
    return runtime->factory;
}

// Reconstructed from eboot.elf at 0x3DEDB0.
void render_system_set_factory(
    RenderSystem& system,
    RenderFactory* factory) {
    auto* runtime = reinterpret_cast<RenderSystemFramePrefix*>(&system);
    runtime->factory = factory;
}

// Reconstructed from eboot.elf at 0x3DED80.
void render_system_release_back_buffer(RenderSystem& system) {
    auto* runtime = reinterpret_cast<RenderSystemFramePrefix*>(&system);
    if (runtime->frame_owner != nullptr) {
        render_frame_owner_delete(*runtime->frame_owner);
        runtime->frame_owner = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x3DEEA0.
void render_system_release_render_contexts(RenderSystem& system) {
    auto* runtime = reinterpret_cast<RenderSystemFramePrefix*>(&system);
    if (runtime->primary_frame_owner != nullptr) {
        render_frame_owner_delete(*runtime->primary_frame_owner);
        runtime->primary_frame_owner = nullptr;
    }

    while (runtime->frame_owners.end != runtime->frame_owners.begin) {
        --runtime->frame_owners.end;
        auto* context = *runtime->frame_owners.end;
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
