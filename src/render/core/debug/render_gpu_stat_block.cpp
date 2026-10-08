#include "render/core/debug/render_gpu_stat_block.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstring>

#include "core/memory/engine_memory.h"
#include "core/types/symbol.h"
#include "render/core/context/render_context.h"
#include "render/core/debug/render_gpu_stat_block_adapters.h"
#include "render/resources/system/render_resource_manager_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kRenderGpuStatBlockOffset = 3584;

struct RenderGpuStatisticDispatch {
    void* reserved_0;
    void (*release_dynamic)(void* statistic);
};

struct RenderPointerArray {
    void** begin;
    void** end;
    void** capacity;
    void* allocator;
};

struct RenderGpuRootStatistic {
    RenderPointerArray children;
    std::uint8_t statistic_base[56];
    RenderPointerArray history[4];
    std::uint8_t reserved_216[168];
};

static_assert(offsetof(RenderGpuRootStatistic, statistic_base) == 32);
static_assert(offsetof(RenderGpuRootStatistic, history) == 88);
static_assert(sizeof(RenderGpuRootStatistic) == 384);

void* root_statistic_base(RenderGpuRootStatistic& statistic) {
    return statistic.statistic_base;
}

void initialize_root_statistic(
    RenderGpuRootStatistic& statistic,
    const char* name,
    RenderGpuRootStatistic* parent) {
    std::memset(&statistic.children, 0, sizeof(statistic.children));
    render_gpu_statistic_construct(
        root_statistic_base(statistic),
        name,
        parent == nullptr ? nullptr : root_statistic_base(*parent));
}

bool& root_has_children(RenderGpuRootStatistic& statistic) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&statistic);
    return *reinterpret_cast<bool*>(bytes + 73);
}

float& root_counter_scale(RenderGpuRootStatistic& statistic) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&statistic);
    return *reinterpret_cast<float*>(bytes + 76);
}

std::uint32_t& root_counter_index(RenderGpuRootStatistic& statistic) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&statistic);
    return *reinterpret_cast<std::uint32_t*>(bytes + 80);
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

void destruct_root_statistic(RenderGpuRootStatistic& statistic) {
    for (std::size_t index = 4; index != 0; --index) {
        auto& history = statistic.history[index - 1];
        release_pointer_array(
            history.begin, history.end, history.capacity);
    }

    render_resource_name_destruct(statistic.statistic_base + 16);
    release_pointer_array(
        statistic.children.begin,
        statistic.children.end,
        statistic.children.capacity);
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

void insert_pointer(
    void**& begin,
    void**& end,
    void**& capacity,
    void** position,
    void* value) {
    if (end != capacity) {
        std::memmove(
            position + 1,
            position,
            static_cast<std::size_t>(end - position) * sizeof(void*));
        *position = value;
        ++end;
        return;
    }

    const auto size = begin == nullptr
        ? std::size_t{0}
        : static_cast<std::size_t>(end - begin);
    const auto insertion_index = begin == nullptr
        ? std::size_t{0}
        : static_cast<std::size_t>(position - begin);
    const auto new_capacity = size == 0 ? std::size_t{1} : size * 2;
    auto** new_begin = static_cast<void**>(
        engine_allocate_sized(new_capacity * sizeof(void*)));

    if (insertion_index != 0) {
        std::memmove(
            new_begin,
            begin,
            insertion_index * sizeof(void*));
    }
    new_begin[insertion_index] = value;
    if (insertion_index != size) {
        std::memmove(
            new_begin + insertion_index + 1,
            position,
            (size - insertion_index) * sizeof(void*));
    }

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

std::uintptr_t pointer_key(const void* value) {
    return reinterpret_cast<std::uintptr_t>(value);
}

const void* statistic_full_name(const void* statistic) {
    const auto* bytes = static_cast<const std::uint8_t*>(statistic);
    return *reinterpret_cast<const void* const*>(bytes + 344);
}

const void* root_statistic_name(const void* statistic) {
    const auto* bytes = static_cast<const std::uint8_t*>(statistic);
    return *reinterpret_cast<const void* const*>(bytes + 40);
}

void append_root_child(
    RenderGpuRootStatistic& root,
    void* statistic) {
    insert_pointer(
        root.children.begin,
        root.children.end,
        root.children.capacity,
        root.children.end,
        statistic);
}

float& statistic_counter_scale(void* statistic) {
    auto* bytes = static_cast<std::uint8_t*>(statistic);
    return *reinterpret_cast<float*>(bytes + 44);
}

std::int32_t& statistic_counter_index(void* statistic) {
    auto* bytes = static_cast<std::uint8_t*>(statistic);
    return *reinterpret_cast<std::int32_t*>(bytes + 48);
}

// Reconstructed from eboot.elf at 0x62B2C0.
void* find_or_create_gpu_statistic(
    RenderGpuStatBlock& block,
    const char* name,
    const char* full_name,
    void* parent) {
    const Symbol name_symbol{name};
    const Symbol full_name_symbol{full_name};
    const auto name_key = name_symbol.value();
    const auto full_name_key = full_name_symbol.value();

    scePthreadMutexLock(&block.mutex);
    ++block.lock_depth;

    auto** statistic_position = block.statistics_begin;
    if (block.statistics_begin != block.statistics_end) {
        statistic_position = std::lower_bound(
            block.statistics_begin,
            block.statistics_end,
            full_name_key,
            [](const void* statistic, const void* key) {
                return pointer_key(statistic_full_name(statistic)) <
                    pointer_key(key);
            });
    }

    void* statistic = nullptr;
    if (statistic_position != block.statistics_end &&
        statistic_full_name(*statistic_position) == full_name_key) {
        statistic = *statistic_position;
        --block.lock_depth;
        scePthreadMutexUnlock(&block.mutex);
        return statistic;
    } else {
        statistic = render_allocate(352);
        render_gpu_statistic_construct(
            statistic,
            static_cast<const char*>(name_symbol.value()),
            parent);
        insert_pointer(
            block.statistics_begin,
            block.statistics_end,
            block.statistics_capacity,
            statistic_position,
            statistic);
    }

    auto** root_position = block.root_statistics_begin;
    if (block.root_statistics_begin != block.root_statistics_end) {
        root_position = std::lower_bound(
            block.root_statistics_begin,
            block.root_statistics_end,
            name_key,
            [](const void* root, const void* key) {
                return pointer_key(root_statistic_name(root)) <
                    pointer_key(key);
            });
    }

    RenderGpuRootStatistic* root = nullptr;
    if (root_position != block.root_statistics_end &&
        root_statistic_name(*root_position) == name_key) {
        root = static_cast<RenderGpuRootStatistic*>(*root_position);
        if (root_counter_index(*root) != UINT32_MAX &&
            statistic_counter_scale(statistic) == 0.0F) {
            statistic_counter_scale(statistic) = root_counter_scale(*root);
            statistic_counter_index(statistic) = static_cast<std::int32_t>(
                root_counter_index(*root));
        }
    } else {
        root = static_cast<RenderGpuRootStatistic*>(
            render_allocate(sizeof(RenderGpuRootStatistic)));
        initialize_root_statistic(
            *root,
            static_cast<const char*>(name_symbol.value()),
            static_cast<RenderGpuRootStatistic*>(block.total_statistic));
        root_has_children(
            *static_cast<RenderGpuRootStatistic*>(block.total_statistic)) = true;
        insert_pointer(
            block.root_statistics_begin,
            block.root_statistics_end,
            block.root_statistics_capacity,
            root_position,
            root);
    }

    append_root_child(*root, statistic);

    --block.lock_depth;
    scePthreadMutexUnlock(&block.mutex);
    return statistic;
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
        root_has_children(*total) = true;
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
            destruct_root_statistic(
                *static_cast<RenderGpuRootStatistic*>(*item));
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
    auto* statistic = find_or_create_gpu_statistic(
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
