#pragma once

#include <cstdarg>
#include <cstdio>

// snprintf that always terminates the buffer. Inline in the map's build,
// where its first copy is emitted in audio/ByteGrinder.o; the binary keeps
// one copy at 0x9290. Returns the formatted length, or -1 when the text was
// truncated.
inline int HmxSnprintf(char* buffer, unsigned long size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int length = std::vsnprintf(buffer, size, format, args);
    if (length >= static_cast<int>(size)) {
        buffer[size - 1] = '\0';
        length = -1;
    }
    va_end(args);
    return length;
}
