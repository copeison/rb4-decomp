#pragma once

namespace rb4 {

struct RenderGpuStatBlock;

void render_gpu_root_statistic_destruct(void* statistic);
void render_gpu_stat_block_resolve_frame(RenderGpuStatBlock& block);

}  // namespace rb4
