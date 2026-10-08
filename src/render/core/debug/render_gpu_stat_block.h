#pragma once

#include <cstddef>
#include <cstdint>
#include <_pthread.h>

namespace rb4 {

struct RenderContext;
struct RenderSystem;

struct RenderGpuStatBlock {
    void** statistics_begin;
    void** statistics_end;
    void** statistics_capacity;
    void* statistics_allocator;
    void* total_statistic;
    void** root_statistics_begin;
    void** root_statistics_end;
    void** root_statistics_capacity;
    void* root_statistics_allocator;
    std::uint64_t next_query_id;
    std::uint64_t frame_slot;
    std::uint8_t reserved_88[8];
    std::uint64_t active_statistics_slot;
    void* backend;
    std::int32_t lock_depth;
    std::uint32_t reserved_116;
    ScePthreadMutex mutex;
};

static_assert(offsetof(RenderGpuStatBlock, total_statistic) == 32);
static_assert(offsetof(RenderGpuStatBlock, next_query_id) == 72);
static_assert(offsetof(RenderGpuStatBlock, frame_slot) == 80);
static_assert(offsetof(RenderGpuStatBlock, active_statistics_slot) == 96);
static_assert(offsetof(RenderGpuStatBlock, backend) == 104);
static_assert(offsetof(RenderGpuStatBlock, lock_depth) == 112);
static_assert(offsetof(RenderGpuStatBlock, mutex) == 120);
static_assert(sizeof(RenderGpuStatBlock) == 128);

RenderGpuStatBlock& render_system_gpu_stat_block(RenderSystem& system);

void render_gpu_stat_block_construct(RenderGpuStatBlock& block);
void render_gpu_stat_block_initialize(RenderGpuStatBlock& block);
void render_gpu_stat_block_destruct(RenderGpuStatBlock& block);
std::int64_t render_gpu_stat_block_begin(
    RenderGpuStatBlock& block,
    RenderContext& context,
    const char* name);
void render_gpu_stat_block_end(
    RenderGpuStatBlock& block,
    RenderContext& context,
    std::int64_t query_id);
void render_gpu_stat_block_finish_frame(RenderGpuStatBlock& block);

}  // namespace rb4
