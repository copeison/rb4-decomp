#pragma once

#include "game/startup/system_init_options.h"

namespace rb4 {

struct RenderGpuStatBlock;
struct RenderSystem;

void render_gpu_stat_block_initialize(RenderGpuStatBlock& block);
}  // namespace rb4
