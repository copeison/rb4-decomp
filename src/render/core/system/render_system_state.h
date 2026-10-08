#pragma once

#include <cstddef>
#include <cstdint>
#include <_pthread.h>

#include "game/startup/system_init_options.h"
#include "render/core/frame/render_frame_owner_list.h"

namespace rb4 {

struct RenderContext;
struct RenderFactory;
struct RenderFrameOwner;
struct RenderSettings;
struct RenderSystem;
struct RenderTargetState;

struct RenderContextArray {
    RenderContext** begin;
    RenderContext** end;
    RenderContext** capacity;
    void* allocator;
    void* allocator_state;
};

struct RenderTargetStateArray {
    RenderTargetState** begin;
    RenderTargetState** end;
    RenderTargetState** capacity;
    void* allocator;
};

struct RenderSystemCoreState {
    void* virtual_table;
    std::int32_t lock_depth;
    std::uint32_t reserved_12;
    ScePthreadMutex frame_mutex;
    ScePthread lock_owner;
    bool initialized;
    std::uint8_t reserved_33[7];
    GameSystemInitOptions init_options;
    RenderContext* render_context;
    bool frame_activation_pending;
    std::uint8_t reserved_65[3];
    std::uint32_t frame_activation_flags;
    RenderContextArray render_contexts;
    union {
        RenderFrameOwner* frame_owner;
        RenderFrameOwner* back_buffer_owner;
        void* frame_owner_storage;
    };
    RenderFrameOwner* active_frame_owner;
    RenderTargetStateArray active_target_states;
    std::uint64_t frame_epoch;
    std::uint64_t auxiliary_frame_epoch;
    bool frame_in_progress;
    bool shutting_down;
    std::uint8_t reserved_178[6];
    RenderFrameOwnerList submitted_frame_owners;
    std::uint8_t reserved_200[56];
    std::uint64_t previous_frame_counter;
    std::uint64_t initial_frame_tick_span;
    std::uint32_t frame_timing_initialized;
    std::uint32_t reserved_276;
    void* gpu_frame_stat;
    float instantaneous_frame_rate;
    float smoothed_frame_rate;
    RenderSettings* settings;
    RenderFactory* factory;
};

static_assert(sizeof(RenderContextArray) == 40);
static_assert(sizeof(RenderTargetStateArray) == 32);
static_assert(offsetof(RenderSystemCoreState, lock_depth) == 8);
static_assert(offsetof(RenderSystemCoreState, frame_mutex) == 16);
static_assert(offsetof(RenderSystemCoreState, lock_owner) == 24);
static_assert(offsetof(RenderSystemCoreState, initialized) == 32);
static_assert(offsetof(RenderSystemCoreState, init_options) == 40);
static_assert(offsetof(RenderSystemCoreState, render_context) == 56);
static_assert(
    offsetof(RenderSystemCoreState, frame_activation_pending) == 64);
static_assert(offsetof(RenderSystemCoreState, frame_activation_flags) == 68);
static_assert(offsetof(RenderSystemCoreState, render_contexts) == 72);
static_assert(offsetof(RenderSystemCoreState, frame_owner) == 112);
static_assert(offsetof(RenderSystemCoreState, active_frame_owner) == 120);
static_assert(offsetof(RenderSystemCoreState, active_target_states) == 128);
static_assert(offsetof(RenderSystemCoreState, frame_epoch) == 160);
static_assert(offsetof(RenderSystemCoreState, auxiliary_frame_epoch) == 168);
static_assert(offsetof(RenderSystemCoreState, frame_in_progress) == 176);
static_assert(offsetof(RenderSystemCoreState, shutting_down) == 177);
static_assert(offsetof(RenderSystemCoreState, submitted_frame_owners) == 184);
static_assert(
    offsetof(RenderSystemCoreState, previous_frame_counter) == 256);
static_assert(
    offsetof(RenderSystemCoreState, frame_timing_initialized) == 272);
static_assert(offsetof(RenderSystemCoreState, gpu_frame_stat) == 280);
static_assert(
    offsetof(RenderSystemCoreState, instantaneous_frame_rate) == 288);
static_assert(offsetof(RenderSystemCoreState, smoothed_frame_rate) == 292);
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
