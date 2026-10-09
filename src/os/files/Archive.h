#pragma once

#include <functional>

// The game's archive index (os/Archive.o). Only the members its users call
// are declared; the class is not reconstructed.
class Archive {
public:
    // Calls `func` with each archived file or directory matching the
    // pattern under `dir` until it returns true.
    bool Enumerate(const char* dir, std::function<bool(const char*)> func, bool recursive,
                   const char* pattern, bool dirs);  // 0x389E90
};

// The mounted archive, or null when files come from the disk. At
// 0x1A01CF0.
extern Archive* TheArchive;

// Reads the archive options and mounts the main archive.
void ArchiveInit();  // 0x387990
