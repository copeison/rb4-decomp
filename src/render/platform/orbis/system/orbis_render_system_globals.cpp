#include "render/platform/orbis/system/orbis_render_system_globals.h"

#include <algorithm>
#include <cstddef>

#include "os/memory/MemMgr.h"
#include "utl/containers/Std.h"
#include "utl/threading/Thread.h"
#include "render/core/system/render_epoch.h"
#include "render/core/system/render_system_state.h"
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
    RenderSystemCoreState core;
    std::uint8_t reserved_312[3492];
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
    NamedThread submit_thread;
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

static_assert(offsetof(OrbisRenderSystemRuntimePrefix, core) == 0);
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
        offsetof(NamedThread, mThread) == 4152);
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

void initialize_recursive_mutex(ScePthreadMutex& mutex) {
    ScePthreadMutexattr attributes;
    scePthreadMutexattrInit(&attributes);
    scePthreadMutexattrSettype(&attributes, 2);
    scePthreadMutexInit(&mutex, &attributes, "hx crit sec");
    scePthreadMutexattrDestroy(&attributes);
}

void destroy_recursive_mutex(
    ScePthreadMutex& mutex,
    std::int32_t& lock_depth) {
    scePthreadMutexLock(&mutex);
    const auto outstanding_locks = lock_depth;
    scePthreadMutexUnlock(&mutex);

    for (auto index = 0; index < outstanding_locks; ++index) {
        --lock_depth;
        scePthreadMutexUnlock(&mutex);
    }
    scePthreadMutexDestroy(&mutex);
}

RetiredAllocationNode* retired_allocation_sentinel(
    OrbisRenderSystemRuntimePrefix& runtime) {
    return reinterpret_cast<RetiredAllocationNode*>(
        &runtime.retired_allocations_head);
}

void erase_retired_allocation(
    OrbisRenderSystemRuntimePrefix& runtime,
    RetiredAllocationNode& node) {
    node.next->previous = node.previous;
    node.previous->next = node.next;
    HmxAllocator::gStlAllocator.deallocate(&node, sizeof(node));
    --runtime.retired_allocation_count;
}

}  // namespace

OrbisRenderSystem* orbis_render_system_instance() {
    return g_orbis_render_system;
}

RenderSystem& orbis_render_system_base(OrbisRenderSystem& system) {
    return reinterpret_cast<RenderSystem&>(system);
}

OrbisRenderContext& orbis_render_system_context(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    return *reinterpret_cast<OrbisRenderContext*>(
        runtime->core.render_context);
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

void orbis_render_system_initialize_video_state(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    std::fill_n(
        runtime->reserved_3816,
        sizeof(runtime->reserved_3816),
        std::uint8_t{0});
    runtime->submit_condition_mutex = nullptr;
    runtime->submit_token = 1;
    runtime->default_vertex_buffer = nullptr;
}

void orbis_render_system_initialize_submission_state(
    OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->submission_lock_depth = 0;
    initialize_recursive_mutex(runtime->submission_mutex);
    runtime->retired_allocation_lock_depth = 0;
    initialize_recursive_mutex(runtime->retired_allocation_mutex);
}

void orbis_render_system_initialize_command_list(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    auto* sentinel = retired_allocation_sentinel(*runtime);
    runtime->retired_allocations_head = sentinel;
    runtime->retired_allocations_tail = sentinel;
    runtime->retired_allocation_count = 0;
}

void orbis_render_system_destroy_command_list(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    auto* sentinel = retired_allocation_sentinel(*runtime);
    auto* node = runtime->retired_allocations_head;
    while (node != sentinel) {
        auto* next = node->next;
        HmxAllocator::gStlAllocator.deallocate(node, sizeof(*node));
        node = next;
    }
}

void orbis_render_system_destroy_submission_state(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    destroy_recursive_mutex(
        runtime->retired_allocation_mutex,
        runtime->retired_allocation_lock_depth);
    destroy_recursive_mutex(
        runtime->submission_mutex,
        runtime->submission_lock_depth);
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

void orbis_enqueue_retired_allocation(
    OrbisRenderSystem& system,
    void* allocation,
    std::uint64_t frame) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    auto* sentinel = retired_allocation_sentinel(*runtime);
    auto* node = static_cast<RetiredAllocationNode*>(
        HmxAllocator::gStlAllocator.allocate(sizeof(RetiredAllocationNode)));
    node->allocation = allocation;
    node->frame = frame;
    node->next = sentinel;
    node->previous = runtime->retired_allocations_tail;
    runtime->retired_allocations_tail->next = node;
    runtime->retired_allocations_tail = node;
    ++runtime->retired_allocation_count;
}

void orbis_release_retired_allocations_through(
    OrbisRenderSystem& system,
    std::uint64_t completed_frame) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    auto* sentinel = retired_allocation_sentinel(*runtime);
    auto* node = runtime->retired_allocations_head;
    while (node != sentinel) {
        auto* next = node->next;
        if (node->frame <= completed_frame) {
            MemFree(node->allocation);
            erase_retired_allocation(*runtime, *node);
        }
        node = next;
    }
}

void orbis_release_all_retired_allocations_locked(
    OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    auto* sentinel = retired_allocation_sentinel(*runtime);
    auto* node = runtime->retired_allocations_head;
    while (node != sentinel) {
        auto* next = node->next;
        MemFree(node->allocation);
        erase_retired_allocation(*runtime, *node);
        node = next;
    }
}

void render_system_set_render_context(
    OrbisRenderSystem& system,
    OrbisRenderContext& context) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->core.render_context =
        reinterpret_cast<RenderContext*>(&context);
}

void render_system_set_back_buffer(
    OrbisRenderSystem& system,
    OrbisBackBuffer& back_buffer) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->core.frame_owner_storage = &back_buffer;
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

Thread& orbis_submit_thread(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    return runtime->submit_thread.mThread;
}

NamedThread& orbis_submit_thread_wrapper(OrbisRenderSystem& system) {
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
    const auto* back_buffer = static_cast<const OrbisBackBuffer*>(
        runtime->core.frame_owner_storage);
    return back_buffer->active_buffer;
}

void orbis_render_system_publish_instance(OrbisRenderSystem& system) {
    g_orbis_render_system = &system;
}

void orbis_render_system_clear_instance() {
    g_orbis_render_system = nullptr;
}

}  // namespace rb4
