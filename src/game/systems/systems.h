#pragma once

#include "render/system/RndDevice.h"

namespace rb4 {

void game_systems_initialize(const RndInitParams& options);
void game_systems_shutdown(void* context);

}  // namespace rb4
