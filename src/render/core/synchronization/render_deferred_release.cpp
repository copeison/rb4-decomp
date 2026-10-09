#include "render/core/synchronization/render_deferred_release.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <_pthread.h>

#include "os/memory/MemMgr.h"
#include "utl/containers/Std.h"
#include "render/core/system/render_system.h"
#include "render/core/system/render_system_state.h"

namespace rb4 {

namespace {

constexpr std::size_t kDeferredReleaseQueueOffset = 3744;

struct RenderDeferredReleaseState {
    std::int32_t lock_depth;
    std::uint32_t reserved_4;
    ScePthreadMutex mutex;
    void** begin;
    void** end;
    void** capacity;
    void* allocator;
    void* reserved_callback;
    std::uint32_t frame_phase;
    std::uint32_t reserved_60;
};

struct RenderDeferredObjectDispatch {
    void* reserved_destruct;
    void (*release_dynamic)(void* object);
};

static_assert(sizeof(RenderDeferredReleaseState) == 64);
static_assert(offsetof(RenderDeferredReleaseState, mutex) == 8);
static_assert(offsetof(RenderDeferredReleaseState, begin) == 16);
static_assert(offsetof(RenderDeferredReleaseState, allocator) == 40);
static_assert(offsetof(RenderDeferredReleaseState, reserved_callback) == 48);
static_assert(offsetof(RenderDeferredReleaseState, frame_phase) == 56);

RenderDeferredReleaseState& deferred_release_state(
    RenderSystem& system) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&system);
    return *reinterpret_cast<RenderDeferredReleaseState*>(
        bytes + kDeferredReleaseQueueOffset);
}

void release_deferred_object(void* object) {
    if (object == nullptr) {
        return;
    }
    auto* dispatch = *static_cast<RenderDeferredObjectDispatch**>(object);
    dispatch->release_dynamic(object);
}

void append_deferred_object(
    RenderDeferredReleaseState& queue,
    void* object) {
    if (queue.end != queue.capacity) {
        *queue.end++ = object;
        return;
    }

    const auto current_size = queue.begin == nullptr
        ? std::size_t{0}
        : static_cast<std::size_t>(queue.end - queue.begin);
    const auto new_capacity = current_size == 0
        ? std::size_t{1}
        : current_size * 2;
    auto** new_begin = static_cast<void**>(
        HmxAllocator::gStlAllocator.allocate(new_capacity * sizeof(void*)));

    if (current_size != 0) {
        std::memmove(
            new_begin,
            queue.begin,
            current_size * sizeof(void*));
    }
    new_begin[current_size] = object;

    if (queue.begin != nullptr) {
        HmxAllocator::gStlAllocator.deallocate(
            queue.begin,
            static_cast<std::size_t>(
                reinterpret_cast<std::uint8_t*>(queue.capacity) -
                reinterpret_cast<std::uint8_t*>(queue.begin)));
    }

    queue.begin = new_begin;
    queue.end = new_begin + current_size + 1;
    queue.capacity = new_begin + new_capacity;
}

}  // namespace

// Reconstructed from eboot.elf at 0x3DD68A-0x3DD6D9.
void render_system_construct_deferred_release_state(RenderSystem& system) {
    auto& state = deferred_release_state(system);
    state.lock_depth = 0;
    state.reserved_4 = 0;

    ScePthreadMutexattr attributes{};
    scePthreadMutexattrInit(&attributes);
    scePthreadMutexattrSettype(&attributes, 2);
    scePthreadMutexInit(&state.mutex, &attributes, "hx crit sec");
    scePthreadMutexattrDestroy(&attributes);

    state.begin = nullptr;
    state.end = nullptr;
    state.capacity = nullptr;
    state.allocator = nullptr;
    state.reserved_callback = nullptr;
    state.frame_phase = 0;
    state.reserved_60 = 0;
}

// Reconstructed from eboot.elf at 0x3DD7B9-0x3DD815.
void render_system_destroy_deferred_release_state(RenderSystem& system) {
    auto& state = deferred_release_state(system);
    if (state.begin != nullptr) {
        const auto byte_count = static_cast<std::size_t>(
            reinterpret_cast<std::uint8_t*>(state.capacity) -
            reinterpret_cast<std::uint8_t*>(state.begin));
        HmxAllocator::gStlAllocator.deallocate(state.begin, byte_count);
        state.begin = nullptr;
        state.end = nullptr;
        state.capacity = nullptr;
    }

    scePthreadMutexLock(&state.mutex);
    scePthreadMutexUnlock(&state.mutex);
    while (state.lock_depth > 0) {
        --state.lock_depth;
        scePthreadMutexUnlock(&state.mutex);
    }
    scePthreadMutexDestroy(&state.mutex);
}

void render_system_flush_deferred_releases(RenderSystem& system) {
    auto& queue = deferred_release_state(system);
    scePthreadMutexLock(&queue.mutex);
    ++queue.lock_depth;

    for (auto** item = queue.begin; item != queue.end; ++item) {
        release_deferred_object(*item);
    }
    queue.end = queue.begin;

    --queue.lock_depth;
    scePthreadMutexUnlock(&queue.mutex);
}

// Reconstructed from eboot.elf at 0x3DEC20.
void render_system_enqueue_deferred_release(
    RenderSystem& system,
    void* object) {
    if (render_system_core_state(system).shutting_down) {
        release_deferred_object(object);
        return;
    }

    auto& queue = deferred_release_state(system);
    scePthreadMutexLock(&queue.mutex);
    ++queue.lock_depth;
    append_deferred_object(queue, object);
    --queue.lock_depth;
    scePthreadMutexUnlock(&queue.mutex);
}

}  // namespace rb4
