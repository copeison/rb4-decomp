#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderContext;
struct RenderFactory;
struct RenderFrameOwner;
struct RenderSettings;
struct RenderSystem;

struct RenderFrameOwnerArray {
    RenderFrameOwner** begin;
    RenderFrameOwner** end;
    RenderFrameOwner** capacity;
    void* allocator;
    void* allocator_state;
};

struct RenderSystemCoreState {
    std::uint8_t reserved_0[56];
    union {
        RenderContext* render_context;
        RenderFrameOwner* primary_frame_owner;
        void* render_context_storage;
    };
    bool frame_activation_pending;
    std::uint8_t reserved_65[3];
    std::uint32_t frame_activation_flags;
    RenderFrameOwnerArray frame_owners;
    union {
        RenderFrameOwner* frame_owner;
        void* frame_owner_storage;
    };
    std::uint8_t reserved_120[40];
    std::uint64_t frame_epoch;
    std::uint8_t reserved_168[128];
    RenderSettings* settings;
    RenderFactory* factory;
};

static_assert(sizeof(RenderFrameOwnerArray) == 40);
static_assert(offsetof(RenderSystemCoreState, primary_frame_owner) == 56);
static_assert(
    offsetof(RenderSystemCoreState, frame_activation_pending) == 64);
static_assert(offsetof(RenderSystemCoreState, frame_activation_flags) == 68);
static_assert(offsetof(RenderSystemCoreState, frame_owners) == 72);
static_assert(offsetof(RenderSystemCoreState, frame_owner) == 112);
static_assert(offsetof(RenderSystemCoreState, frame_epoch) == 160);
static_assert(offsetof(RenderSystemCoreState, settings) == 296);
static_assert(offsetof(RenderSystemCoreState, factory) == 304);
static_assert(sizeof(RenderSystemCoreState) == 312);

inline RenderSystemCoreState& render_system_core_state(
    RenderSystem& system) {
    return reinterpret_cast<RenderSystemCoreState&>(system);
}

inline const RenderSystemCoreState& render_system_core_state(
    const RenderSystem& system) {
    return reinterpret_cast<const RenderSystemCoreState&>(system);
}

}  // namespace rb4
