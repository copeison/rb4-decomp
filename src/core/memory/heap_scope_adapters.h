#pragma once

#include <cstdint>

namespace rb4 {

// Thread-local allocation-mode scope at 0x37AA30 and 0x37AAF0. Begin saves the
// calling thread's mode word (+0x84 in its heap state) and, when requested,
// replaces it; end restores the saved word.
void engine_heap_scope_begin(std::uint32_t& saved, bool enable, bool apply);
void engine_heap_scope_end(const std::uint32_t& saved);

}  // namespace rb4
