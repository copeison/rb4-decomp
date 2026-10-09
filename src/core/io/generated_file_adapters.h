#pragma once

#include <cstdint>

namespace rb4 {

class Symbol;
struct RenderResourceName;

// 16-byte file timestamp filled by generated-file lookups.
struct EngineFileTimestamp {
    std::int64_t seconds;
    std::int64_t fraction;
};

// Byte at 0x19E4558. When set, generated files are trusted as shipped and
// never treated as stale; several file-system paths consult it.
extern std::uint8_t g_engine_file_archive_mode;

// Path-to-symbol resolution at 0x1AF950. Any "::" suffix is ignored while the
// path is normalized, then restored.
void engine_file_resolve_path(Symbol& symbol, const char* path);

// Generated-file lookup at 0x1AD8B0. Resolves the generated file for a
// source path and extension, reports whether it must be rebuilt because the
// source is newer, and returns false when neither file is usable.
bool engine_file_find_generated(
    const Symbol& source,
    const char* extension,
    RenderResourceName& generated_path,
    bool& rebuild_needed,
    EngineFileTimestamp& timestamp);

}  // namespace rb4
