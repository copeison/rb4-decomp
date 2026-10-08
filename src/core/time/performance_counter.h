#pragma once

#include <cstdint>

namespace rb4 {

std::uint64_t performance_counter_read();
double performance_counter_ticks_to_milliseconds(std::uint64_t ticks);

}  // namespace rb4
