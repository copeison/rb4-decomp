#include "render/core/debug/render_gpu_stat_block.h"

#include <cstddef>
#include <cstdint>

#include "core/memory/engine_memory.h"
#include "render/core/context/render_context.h"
#include "render/core/debug/render_gpu_stat_block_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kRenderGpuStatBlockOffset = 3584;

struct RenderGpuStatisticDispatch {
    void* reserved_0;
    void (*release_dynamic)(void* statistic);
};

void release_gpu_statistic(void* statistic) {
    if (statistic == nullptr) {
        return;
    }

    auto* dispatch = *static_cast<RenderGpuStatisticDispatch**>(statistic);
    dispatch->release_dynamic(statistic);
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

void release_pointer_array(
    void**& begin,
    void**& end,
    void**& capacity) {
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

}  // namespace

RenderGpuStatBlock& render_system_gpu_stat_block(RenderSystem& system) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&system);
    return *reinterpret_cast<RenderGpuStatBlock*>(
        bytes + kRenderGpuStatBlockOffset);
}

// Reconstructed from eboot.elf at 0x62AAE0.
void render_gpu_stat_block_construct(RenderGpuStatBlock& block) {
    block.statistics_begin = nullptr;
    block.statistics_end = nullptr;
    block.statistics_capacity = nullptr;
    block.statistics_allocator = nullptr;
    block.total_statistic = nullptr;
    block.root_statistics_begin = nullptr;
    block.root_statistics_end = nullptr;
    block.root_statistics_capacity = nullptr;
    block.root_statistics_allocator = nullptr;
    block.next_query_id = 0;
    block.frame_slot = 0;
    for (auto& byte : block.reserved_88) {
        byte = 0;
    }
    block.backend = nullptr;
    block.lock_depth = 0;
    block.reserved_116 = 0;
    initialize_recursive_mutex(block.mutex);
}

// Reconstructed from eboot.elf at 0x62ABA0.
void render_gpu_stat_block_destruct(RenderGpuStatBlock& block) {
    for (auto** item = block.statistics_begin;
         item != block.statistics_end;
         ++item) {
        release_gpu_statistic(*item);
    }
    block.statistics_end = block.statistics_begin;

    for (auto** item = block.root_statistics_begin;
         item != block.root_statistics_end;
         ++item) {
        if (*item != nullptr) {
            render_gpu_root_statistic_destruct(*item);
            render_release(*item);
        }
    }
    block.root_statistics_end = block.root_statistics_begin;

    destroy_recursive_mutex(block.mutex, block.lock_depth);
    release_pointer_array(
        block.root_statistics_begin,
        block.root_statistics_end,
        block.root_statistics_capacity);
    release_pointer_array(
        block.statistics_begin,
        block.statistics_end,
        block.statistics_capacity);
}

// Reconstructed from eboot.elf at 0x62B5B0.
void render_gpu_stat_block_end(
    RenderGpuStatBlock& block,
    RenderContext& context,
    std::int64_t query_id) {
    if (query_id < 0 || block.backend == nullptr) {
        return;
    }

    render_context_pop_gpu_stat_scope(context);
    render_context_end_gpu_stat(
        context, static_cast<std::uint64_t>(query_id));
}

// Reconstructed from eboot.elf at 0x62B960.
void render_gpu_stat_block_finish_frame(RenderGpuStatBlock& block) {
    render_gpu_stat_block_resolve_frame(block);

    scePthreadMutexLock(&block.mutex);
    const auto previous_lock_depth = block.lock_depth;
    block.lock_depth = previous_lock_depth + 1;
    block.frame_slot =
        (static_cast<std::uint8_t>(block.frame_slot) + 1U) & 3U;

    const auto history_offset = 56 + 32 * block.frame_slot;
    for (auto** item = block.statistics_begin;
         item != block.statistics_end;
         ++item) {
        auto* statistic = static_cast<std::uint8_t*>(*item);
        auto*& begin = *reinterpret_cast<void***>(
            statistic + history_offset);
        auto*& end = *reinterpret_cast<void***>(
            statistic + history_offset + sizeof(void*));
        end = begin;
    }

    block.lock_depth = previous_lock_depth;
    scePthreadMutexUnlock(&block.mutex);
}

}  // namespace rb4
