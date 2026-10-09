#pragma once

#include <cstdarg>
#include <cstdio>

// snprintf that always terminates the buffer. Inline in the map's build,
// where its first copy is emitted in audio/ByteGrinder.o; the binary keeps
// one copy at 0x9290. The return type is not recovered.
inline void HmxSnprintf(char* buffer, unsigned long size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    if (std::vsnprintf(buffer, size, format, args) >= static_cast<int>(size)) {
        buffer[size - 1] = '\0';
    }
    va_end(args);
}
