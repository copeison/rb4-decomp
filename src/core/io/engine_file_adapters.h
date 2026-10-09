#pragma once

#include <cstdint>

namespace rb4 {

struct EngineFile;

// Engine file-system open routine at 0x376D40.
EngineFile* engine_file_system_open(const char* path, std::uint32_t mode);

}  // namespace rb4
