#pragma once

#include <cstdint>

namespace rb4 {

// Interned decimal text for small integers at 0x256410. Negative values map
// to "-1"; non-negative values index the table at 0x18EF6F0.
const char* engine_integer_text(std::int32_t value);

}  // namespace rb4
