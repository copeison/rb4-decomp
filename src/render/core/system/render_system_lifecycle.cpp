#include "render/core/system/render_system_lifecycle.h"

#include <cstddef>
#include <_pthread.h>

#include "core/memory/engine_memory.h"
#include "render/core/debug/render_gpu_stat_block.h"
#include "render/core/settings/render_settings.h"
#include "render/core/synchronization/render_deferred_release.h"
#include "render/core/system/render_system_lifecycle_adapters.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/system/render_system_state.h"
#include "render/resources/lighting/render_lighting_resources.h"
#include "render/resources/system/render_resource_manager.h"

namespace rb4 {

namespace {

constexpr std::size_t kPlatformConfigCount = 13;
constexpr std::size_t kCurrentPlatformConfig = 7;
constexpr std::size_t kBackendResourceOffset = 3560;
constexpr std::size_t kPrimitiveMeshSetOffset = 3568;
constexpr std::size_t kAudioAnalysisTextureSetOffset = 3576;
constexpr std::size_t kBuiltinBufferStorageOffset = 3712;

void* state_at(RenderSystem& system, std::size_t offset) {
    return reinterpret_cast<std::uint8_t*>(&system) + offset;
}

void*& pointer_at(RenderSystem& system, std::size_t offset) {
    return *reinterpret_cast<void**>(state_at(system, offset));
}

template <typename T>
void release_array(T*& begin, T*& end, T*& capacity) {
    if (begin != nullptr) {
        const auto byte_count = static_cast<std::size_t>(
            reinterpret_cast<std::uint8_t*>(capacity) -
            reinterpret_cast<std::uint8_t*>(begin));
        engine_deallocate_sized(begin, byte_count);
    }
    begin = nullptr;
    end = nullptr;
    capacity = nullptr;
}

void initialize_recursive_mutex(ScePthreadMutex& mutex) {
    ScePthreadMutexattr attributes{};
    scePthreadMutexattrInit(&attributes);
    scePthreadMutexattrSettype(&attributes, 2);
    scePthreadMutexInit(&mutex, &attributes, "hx crit sec");
    scePthreadMutexattrDestroy(&attributes);
}

void destroy_recursive_mutex(
    ScePthreadMutex& mutex,
    std::int32_t& lock_depth) {
    scePthreadMutexLock(&mutex);
    scePthreadMutexUnlock(&mutex);
    while (lock_depth > 0) {
        --lock_depth;
        scePthreadMutexUnlock(&mutex);
    }
    scePthreadMutexDestroy(&mutex);
}

void construct_core_state(RenderSystem& system) {
    render_system_install_base_vtable(system);
    auto& state = render_system_core_state(system);
    state.lock_depth = 0;
    initialize_recursive_mutex(state.frame_mutex);
    state.lock_owner = {};
    state.initialized = false;
    state.init_options = {true, true, true, {}, 0};
    state.render_context = nullptr;
    state.frame_activation_pending = false;
    state.frame_activation_flags = 0;
    state.render_contexts = {};
    state.frame_owner = nullptr;
    state.active_frame_owner = nullptr;
    state.active_target_states = {};
    state.frame_epoch = 0;
    state.auxiliary_frame_epoch = 0;
    state.frame_in_progress = false;
    state.shutting_down = false;

    auto* inline_owners = reinterpret_cast<RenderFrameOwner**>(
        reinterpret_cast<std::uint8_t*>(&system) + 208);
    state.submitted_frame_owners = {inline_owners, 0};
    state.previous_frame_counter = 0;
    state.initial_frame_tick_span = 0;
    state.frame_timing_initialized = 0;
    state.gpu_frame_stat_id = -1;
    state.instantaneous_frame_rate = 0.0F;
    state.smoothed_frame_rate = 0.0F;
    state.settings = nullptr;
    state.factory = nullptr;
}

void destroy_core_state(RenderSystem& system) {
    auto& state = render_system_core_state(system);
    release_array(
        state.active_target_states.begin,
        state.active_target_states.end,
        state.active_target_states.capacity);
    release_array(
        state.render_contexts.begin,
        state.render_contexts.end,
        state.render_contexts.capacity);
    destroy_recursive_mutex(state.frame_mutex, state.lock_depth);
}

void construct_backend_state(RenderSystem& system) {
    render_resource_manager_construct(
        render_system_resource_manager(system));
    render_lighting_resources_construct(
        render_system_lighting_resources(system));
    pointer_at(system, kBackendResourceOffset) = nullptr;
    pointer_at(system, kPrimitiveMeshSetOffset) = nullptr;
    pointer_at(system, kAudioAnalysisTextureSetOffset) = nullptr;
    render_gpu_stat_block_construct(render_system_gpu_stat_block(system));

    auto** builtin_buffers = reinterpret_cast<void**>(
        state_at(system, kBuiltinBufferStorageOffset));
    for (std::size_t index = 0; index < 4; ++index) {
        builtin_buffers[index] = nullptr;
    }
}

void destroy_backend_state(RenderSystem& system) {
    render_gpu_stat_block_destruct(render_system_gpu_stat_block(system));
    render_lighting_resources_destruct(
        render_system_lighting_resources(system));
    render_resource_manager_destruct(
        render_system_resource_manager(system));
}

}  // namespace

// Reconstructed from eboot.elf at 0x3DD410.
void render_system_construct(RenderSystem& system) {
    construct_core_state(system);

    for (std::size_t index = 0; index < kPlatformConfigCount; ++index) {
        render_platform_config_construct(
            render_system_platform_config_at(system, index));
    }

    render_system_construct_default_resources(system);
    construct_backend_state(system);
    render_system_construct_deferred_release_state(system);
    render_system_publish_instance(system);

    for (const auto platform_id : render_supported_platform_ids()) {
        if (platform_id < kPlatformConfigCount) {
            render_platform_config_initialize(
                render_system_platform_config_at(system, platform_id),
                platform_id);
        }
    }

    // The original invokes this predicate for platform slot seven and ignores
    // its result. Keep the call until its source-level purpose is known.
    render_platform_config_boot_probe(
        render_system_platform_config_at(system, kCurrentPlatformConfig));

    auto* settings = render_settings_allocate();
    render_settings_initialize(*settings);
    render_system_set_settings(system, settings);
}

// Reconstructed from eboot.elf at 0x3DD790.
void render_system_destruct(RenderSystem& system) {
    render_settings_release(render_system_settings(system));
    render_system_set_settings(system, nullptr);

    render_system_destroy_deferred_release_state(system);
    destroy_backend_state(system);
    render_system_destroy_default_resources(system);

    for (std::size_t index = kPlatformConfigCount; index != 0; --index) {
        render_platform_config_destroy(
            render_system_platform_config_at(system, index - 1));
    }

    destroy_core_state(system);
}

}  // namespace rb4
