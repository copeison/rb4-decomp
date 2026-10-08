#include "render/platform/orbis/system/orbis_render_system_globals.h"

#include <cstddef>

#include "core/threading/engine_thread.h"
#include "render/core/system/render_epoch.h"
#include "render/platform/orbis/meshes/orbis_vertex_descriptors.h"
#include "render/platform/orbis/video/orbis_back_buffer.h"

namespace rb4 {

OrbisRenderSystem* g_orbis_render_system = nullptr;

namespace {

struct RetiredAllocationNode {
    RetiredAllocationNode* next;
    RetiredAllocationNode* previous;
    void* allocation;
    std::uint64_t frame;
};

struct OrbisRenderSystemRuntimePrefix {
    std::uint8_t reserved_0[56];
    OrbisRenderContext* render_context;
    std::uint8_t reserved_64[48];
    OrbisBackBuffer* back_buffer;
    std::uint8_t reserved_120[3684];
    std::int32_t video_output_handle;
    SceKernelEqueue event_queue;
    std::uint8_t reserved_3816[8];
    ScePthreadMutex* submit_condition_mutex;
    ScePthreadCond submit_condition;
    std::uint64_t submit_token;
    bool submit_thread_running;
    std::uint8_t reserved_3849[3];
    OrbisBufferDescriptor default_vertex_descriptors[8];
    std::uint8_t reserved_3980[4];
    void* default_vertex_buffer;
    OrbisBufferDescriptor identity_instance_descriptors[9];
    void* identity_instance_buffer;
    EngineThread submit_thread;
    std::int32_t submission_lock_depth;
    std::uint8_t reserved_4284[4];
    ScePthreadMutex submission_mutex;
    std::int32_t retired_allocation_lock_depth;
    std::uint8_t reserved_4300[4];
    ScePthreadMutex retired_allocation_mutex;
    RetiredAllocationNode* retired_allocations_head;
    RetiredAllocationNode* retired_allocations_tail;
    std::size_t retired_allocation_count;
    std::uint8_t retired_allocation_allocator[8];
    std::int32_t cached_flip_rate;
};

static_assert(offsetof(OrbisRenderSystemRuntimePrefix, render_context) == 56);
static_assert(offsetof(OrbisRenderSystemRuntimePrefix, back_buffer) == 112);
static_assert(
    offsetof(OrbisRenderSystemRuntimePrefix, video_output_handle) == 3804);
static_assert(offsetof(OrbisRenderSystemRuntimePrefix, event_queue) == 3808);
static_assert(
    offsetof(OrbisRenderSystemRuntimePrefix, submit_condition_mutex) == 3824);
static_assert(
    offsetof(OrbisRenderSystemRuntimePrefix, submit_condition) == 3832);
static_assert(offsetof(OrbisRenderSystemRuntimePrefix, submit_token) == 3840);
static_assert(
    offsetof(OrbisRenderSystemRuntimePrefix, submit_thread_running) == 3848);
static_assert(
    offsetof(
        OrbisRenderSystemRuntimePrefix,
        default_vertex_descriptors) == 3852);
static_assert(
    offsetof(OrbisRenderSystemRuntimePrefix, default_vertex_buffer) == 3984);
static_assert(
    offsetof(
        OrbisRenderSystemRuntimePrefix,
        identity_instance_descriptors) == 3992);
static_assert(
    offsetof(OrbisRenderSystemRuntimePrefix, identity_instance_buffer) == 4136);
static_assert(
    offsetof(OrbisRenderSystemRuntimePrefix, submit_thread) == 4144);
static_assert(
    offsetof(OrbisRenderSystemRuntimePrefix, submit_thread) +
        offsetof(EngineThread, runtime) == 4152);
static_assert(
    offsetof(OrbisRenderSystemRuntimePrefix, submission_lock_depth) == 4280);
static_assert(
    offsetof(OrbisRenderSystemRuntimePrefix, submission_mutex) == 4288);
static_assert(
    offsetof(
        OrbisRenderSystemRuntimePrefix,
        retired_allocation_lock_depth) == 4296);
static_assert(
    offsetof(OrbisRenderSystemRuntimePrefix, retired_allocation_mutex) == 4304);
static_assert(
    offsetof(OrbisRenderSystemRuntimePrefix, retired_allocations_head) == 4312);
static_assert(
    offsetof(OrbisRenderSystemRuntimePrefix, retired_allocations_tail) == 4320);
static_assert(
    offsetof(OrbisRenderSystemRuntimePrefix, retired_allocation_count) == 4328);
static_assert(
    offsetof(OrbisRenderSystemRuntimePrefix, cached_flip_rate) == 4344);

}  // namespace

OrbisRenderSystem* orbis_render_system_instance() {
    return g_orbis_render_system;
}

RenderSystem& orbis_render_system_base(OrbisRenderSystem& system) {
    return reinterpret_cast<RenderSystem&>(system);
}

OrbisRenderContext& orbis_render_system_context(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    return *runtime->render_context;
}

std::int32_t orbis_video_output_handle(const OrbisRenderSystem& system) {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderSystemRuntimePrefix*>(&system);
    return runtime->video_output_handle;
}

SceKernelEqueue orbis_event_queue(const OrbisRenderSystem& system) {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderSystemRuntimePrefix*>(&system);
    return runtime->event_queue;
}

void orbis_set_video_output_handle(
    OrbisRenderSystem& system,
    std::int32_t handle) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->video_output_handle = handle;
}

void orbis_set_event_queue(
    OrbisRenderSystem& system,
    SceKernelEqueue queue) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->event_queue = queue;
}

void orbis_initialize_submit_condition(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->submit_condition_mutex = &runtime->submission_mutex;

    ScePthreadCondattr attributes;
    scePthreadCondattrInit(&attributes);
    scePthreadCondInit(
        &runtime->submit_condition, &attributes, "Condition");
}

void orbis_destroy_submit_condition(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    if (runtime->submit_condition_mutex == nullptr) {
        return;
    }
    scePthreadCondDestroy(&runtime->submit_condition);
    runtime->submit_condition_mutex = nullptr;
}

void orbis_wait_for_submit_token(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    scePthreadCondWait(
        &runtime->submit_condition, runtime->submit_condition_mutex);
}

void orbis_lock_submission(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    scePthreadMutexLock(&runtime->submission_mutex);
}

void orbis_unlock_submission(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    scePthreadMutexUnlock(&runtime->submission_mutex);
}

void orbis_submit_scope_begin(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    ++runtime->submission_lock_depth;
}

void orbis_submit_scope_end(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    --runtime->submission_lock_depth;
}

void orbis_lock_retired_allocations(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    scePthreadMutexLock(&runtime->retired_allocation_mutex);
    ++runtime->retired_allocation_lock_depth;
}

void orbis_unlock_retired_allocations(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    --runtime->retired_allocation_lock_depth;
    scePthreadMutexUnlock(&runtime->retired_allocation_mutex);
}

std::size_t orbis_retired_allocation_count(
    const OrbisRenderSystem& system) {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderSystemRuntimePrefix*>(&system);
    return runtime->retired_allocation_count;
}

std::uint64_t orbis_retired_allocation_frame(
    const OrbisRenderSystem& system,
    std::size_t index) {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderSystemRuntimePrefix*>(&system);
    auto* node = runtime->retired_allocations_head;
    while (index-- != 0) {
        node = node->next;
    }
    return node->frame;
}

void render_system_set_render_context(
    OrbisRenderSystem& system,
    OrbisRenderContext& context) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->render_context = &context;
}

void render_system_set_back_buffer(
    OrbisRenderSystem& system,
    OrbisBackBuffer& back_buffer) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->back_buffer = &back_buffer;
}

std::uint64_t orbis_render_system_epoch(const OrbisRenderSystem& system) {
    const auto& base = reinterpret_cast<const RenderSystem&>(system);
    return render_epoch(base);
}

bool orbis_submit_token_available(const OrbisRenderSystem& system) {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderSystemRuntimePrefix*>(&system);
    return runtime->submit_token != 0;
}

void orbis_publish_submit_token(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->submit_token = 1;
}

void orbis_consume_submit_token(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->submit_token = 0;
}

void orbis_signal_submit_condition(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    scePthreadCondSignal(&runtime->submit_condition);
}

bool orbis_submit_thread_running(const OrbisRenderSystem& system) {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderSystemRuntimePrefix*>(&system);
    return runtime->submit_thread_running;
}

void orbis_set_submit_thread_running(
    OrbisRenderSystem& system,
    bool running) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->submit_thread_running = running;
}

std::int32_t orbis_cached_flip_rate(const OrbisRenderSystem& system) {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderSystemRuntimePrefix*>(&system);
    return runtime->cached_flip_rate;
}

void orbis_set_cached_flip_rate(
    OrbisRenderSystem& system,
    std::int32_t rate) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->cached_flip_rate = rate;
}

EngineThreadRuntime& orbis_submit_thread(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    return runtime->submit_thread.runtime;
}

EngineThread& orbis_submit_thread_wrapper(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    return runtime->submit_thread;
}

OrbisBufferDescriptor* orbis_default_vertex_descriptors(
    OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    return runtime->default_vertex_descriptors;
}

const OrbisBufferDescriptor* orbis_default_vertex_descriptors() {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderSystemRuntimePrefix*>(
            g_orbis_render_system);
    return runtime->default_vertex_descriptors;
}

void orbis_set_default_vertex_buffer(
    OrbisRenderSystem& system,
    void* buffer) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->default_vertex_buffer = buffer;
}

OrbisBufferDescriptor* orbis_identity_instance_descriptors(
    OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    return runtime->identity_instance_descriptors;
}

const OrbisBufferDescriptor* orbis_identity_instance_descriptors() {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderSystemRuntimePrefix*>(
            g_orbis_render_system);
    return runtime->identity_instance_descriptors;
}

void orbis_set_identity_instance_buffer(
    OrbisRenderSystem& system,
    void* buffer) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->identity_instance_buffer = buffer;
}

std::size_t orbis_active_render_frame_index() {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderSystemRuntimePrefix*>(
            g_orbis_render_system);
    return runtime->back_buffer->active_buffer;
}

void orbis_render_system_publish_instance(OrbisRenderSystem& system) {
    g_orbis_render_system = &system;
}

void orbis_render_system_clear_instance() {
    g_orbis_render_system = nullptr;
}

}  // namespace rb4
