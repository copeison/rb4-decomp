#include "render/core/debug/render_gpu_stat_block.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstring>

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

struct RenderGpuRootStatistic {
    std::uint8_t storage[384];
};

static_assert(sizeof(RenderGpuRootStatistic) == 384);

void* root_statistic_base(RenderGpuRootStatistic& statistic) {
    return statistic.storage + 32;
}

void initialize_root_statistic(
    RenderGpuRootStatistic& statistic,
    const char* name,
    RenderGpuRootStatistic* parent) {
    std::memset(statistic.storage, 0, 32);
    render_gpu_statistic_construct(
        root_statistic_base(statistic),
        name,
        parent == nullptr ? nullptr : root_statistic_base(*parent));
}

bool& root_has_hardware_counters(RenderGpuRootStatistic& statistic) {
    return *reinterpret_cast<bool*>(&statistic.storage[73]);
}

float& root_counter_scale(RenderGpuRootStatistic& statistic) {
    return *reinterpret_cast<float*>(&statistic.storage[76]);
}

std::uint32_t& root_counter_index(RenderGpuRootStatistic& statistic) {
    return *reinterpret_cast<std::uint32_t*>(&statistic.storage[80]);
}

std::uint64_t root_sort_key(const void* statistic) {
    const auto* bytes = static_cast<const std::uint8_t*>(statistic);
    return *reinterpret_cast<const std::uint64_t*>(bytes + 40);
}

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

void append_root_statistic(
    RenderGpuStatBlock& block,
    RenderGpuRootStatistic* statistic) {
    if (block.root_statistics_end != block.root_statistics_capacity) {
        *block.root_statistics_end++ = statistic;
        return;
    }

    const auto size = block.root_statistics_begin == nullptr
        ? std::size_t{0}
        : static_cast<std::size_t>(
              block.root_statistics_end - block.root_statistics_begin);
    const auto new_capacity = size == 0 ? std::size_t{1} : size * 2;
    auto** new_begin = static_cast<void**>(
        engine_allocate_sized(new_capacity * sizeof(void*)));
    if (size != 0) {
        std::memmove(
            new_begin,
            block.root_statistics_begin,
            size * sizeof(void*));
    }
    new_begin[size] = statistic;

    if (block.root_statistics_begin != nullptr) {
        const auto byte_count = static_cast<std::size_t>(
            reinterpret_cast<std::uint8_t*>(
                block.root_statistics_capacity) -
            reinterpret_cast<std::uint8_t*>(
                block.root_statistics_begin));
        engine_deallocate_sized(block.root_statistics_begin, byte_count);
    }

    block.root_statistics_begin = new_begin;
    block.root_statistics_end = new_begin + size + 1;
    block.root_statistics_capacity = new_begin + new_capacity;
}

void append_query_id(
    void* statistic,
    std::uint64_t frame_slot,
    std::uint64_t query_id) {
    constexpr std::size_t kHistoryOffset = 56;
    constexpr std::size_t kHistoryStride = 32;

    auto* bytes = static_cast<std::uint8_t*>(statistic);
    auto** history = reinterpret_cast<std::uint64_t**>(
        bytes + kHistoryOffset + frame_slot * kHistoryStride);
    auto*& begin = history[0];
    auto*& end = history[1];
    auto*& capacity = history[2];

    if (end != capacity) {
        *end++ = query_id;
        return;
    }

    const auto size = begin == nullptr
        ? std::size_t{0}
        : static_cast<std::size_t>(end - begin);
    const auto new_capacity = size == 0 ? std::size_t{1} : size * 2;
    auto* new_begin = static_cast<std::uint64_t*>(
        engine_allocate_sized(new_capacity * sizeof(std::uint64_t)));
    if (size != 0) {
        std::memmove(
            new_begin,
            begin,
            size * sizeof(std::uint64_t));
    }
    new_begin[size] = query_id;

    if (begin != nullptr) {
        const auto byte_count = static_cast<std::size_t>(
            reinterpret_cast<std::uint8_t*>(capacity) -
            reinterpret_cast<std::uint8_t*>(begin));
        engine_deallocate_sized(begin, byte_count);
    }

    begin = new_begin;
    end = new_begin + size + 1;
    capacity = new_begin + new_capacity;
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

// Reconstructed from eboot.elf at 0x62ACB0.
void render_gpu_stat_block_initialize(RenderGpuStatBlock& block) {
    auto* total = static_cast<RenderGpuRootStatistic*>(
        render_allocate(sizeof(RenderGpuRootStatistic)));
    initialize_root_statistic(*total, "GPU Total", nullptr);
    block.total_statistic = total;
    append_root_statistic(block, total);

    const auto counter_count = render_gpu_counter_count();
    for (std::size_t index = 0; index < counter_count; ++index) {
        auto* counter = static_cast<RenderGpuRootStatistic*>(
            render_allocate(sizeof(RenderGpuRootStatistic)));
        initialize_root_statistic(
            *counter,
            render_gpu_counter_name(static_cast<std::uint32_t>(index)),
            total);
        root_has_hardware_counters(*total) = true;
        root_counter_scale(*counter) =
            render_gpu_counter_scale(static_cast<std::uint32_t>(index));
        root_counter_index(*counter) = static_cast<std::uint32_t>(index);
        append_root_statistic(block, counter);
    }

    std::sort(
        block.root_statistics_begin,
        block.root_statistics_end,
        [](const void* left, const void* right) {
            return root_sort_key(left) < root_sort_key(right);
        });
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

// Reconstructed from eboot.elf at 0x62AF80.
std::int64_t render_gpu_stat_block_begin(
    RenderGpuStatBlock& block,
    RenderContext& context,
    const char* name) {
    if (block.backend == nullptr) {
        return -1;
    }

    auto* parent_scope = render_context_last_gpu_stat_scope(context);
    auto* parent = parent_scope == nullptr
        ? nullptr
        : parent_scope->statistic;
    const char* full_name = name;
    char nested_name[4096]{};
    if (parent != nullptr) {
        auto* parent_bytes = static_cast<std::uint8_t*>(parent);
        parent_bytes[41] = 1;
        const auto* parent_name =
            *reinterpret_cast<const char* const*>(parent_bytes + 24);
        std::snprintf(
            nested_name,
            sizeof(nested_name),
            "%s %s",
            parent_name,
            name);
        full_name = nested_name;
    }

    scePthreadMutexLock(&block.mutex);
    ++block.lock_depth;
    auto* statistic = render_gpu_statistic_find_or_create(
        block, name, full_name, parent);
    const auto query_id = block.next_query_id++;
    append_query_id(statistic, block.frame_slot, query_id);
    --block.lock_depth;
    scePthreadMutexUnlock(&block.mutex);

    render_context_begin_gpu_stat(context, query_id);
    render_context_push_gpu_stat_scope(
        context, {statistic, query_id});
    return static_cast<std::int64_t>(query_id);
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
