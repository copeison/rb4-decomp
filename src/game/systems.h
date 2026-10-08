#pragma once

#include "system_init_options.h"

namespace rb4 {

void game_systems_initialize(const GameSystemInitOptions& options);
void game_systems_shutdown(void* context);

}  // namespace rb4
