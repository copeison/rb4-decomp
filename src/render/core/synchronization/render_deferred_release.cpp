#include "render/core/synchronization/render_deferred_release.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <_pthread.h>

#include "core/memory/engine_memory.h"
#include "render/core/synchronization/render_deferred_release_adapters.h"
#include "render/core/system/render_system.h"
#include "render/core/system/render_system_state.h"

namespace rb4 {

namespace {

constexpr std::size_t kDeferredReleaseQueueOffset = 3744;

struct RenderDeferredReleaseQueue {
    std::int32_t lock_depth;
    std::uint32_t reserved_4;
    ScePthreadMutex mutex;
    void** begin;
    void** end;
    void** capacity;
    void* allocator;
};

static_assert(sizeof(RenderDeferredReleaseQueue) == 48);
static_assert(offsetof(RenderDeferredReleaseQueue, mutex) == 8);
static_assert(offsetof(RenderDeferredReleaseQueue, begin) == 16);
static_assert(offsetof(RenderDeferredReleaseQueue, allocator) == 40);

RenderDeferredReleaseQueue& deferred_release_queue(
    RenderSystem& system) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&system);
    return *reinterpret_cast<RenderDeferredReleaseQueue*>(
        bytes + kDeferredReleaseQueueOffset);
}

void append_deferred_object(
    RenderDeferredReleaseQueue& queue,
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
        engine_allocate_sized(new_capacity * sizeof(void*)));

    if (current_size != 0) {
        std::memmove(
            new_begin,
            queue.begin,
            current_size * sizeof(void*));
    }
    new_begin[current_size] = object;

    if (queue.begin != nullptr) {
        engine_deallocate_sized(
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

void render_system_flush_deferred_releases(RenderSystem& system) {
    auto& queue = deferred_release_queue(system);
    scePthreadMutexLock(&queue.mutex);
    ++queue.lock_depth;

    for (auto** item = queue.begin; item != queue.end; ++item) {
        render_deferred_object_release(*item);
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
        render_deferred_object_release(object);
        return;
    }

    auto& queue = deferred_release_queue(system);
    scePthreadMutexLock(&queue.mutex);
    ++queue.lock_depth;
    append_deferred_object(queue, object);
    --queue.lock_depth;
    scePthreadMutexUnlock(&queue.mutex);
}

}  // namespace rb4
