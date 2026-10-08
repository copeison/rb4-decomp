#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderGpuStatBlock;

void render_gpu_root_statistic_destruct(void* statistic);
void render_gpu_root_statistic_construct(
    void* statistic,
    const char* name,
    void* parent);
std::size_t render_gpu_counter_count();
const char* render_gpu_counter_name(std::uint32_t index);
float render_gpu_counter_scale(std::uint32_t index);
void render_gpu_stat_block_resolve_frame(RenderGpuStatBlock& block);

}  // namespace rb4
