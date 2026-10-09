#include "utl/files/FileUtl.h"

#include <cctype>
#include <cstring>

#include "utl/text/Str.h"

namespace {

bool IsSeparator(char c) {
    return c == '/' || c == '\\';
}

// The character after the path's last separator, or the path itself.
// Inlined into FileGetName and FileGetBase.
const char* NameStart(const char* path) {
    const char* p = path + std::strlen(path) - 1;
    while (p >= path && !IsSeparator(*p)) {
        --p;
    }
    return p + 1;
}

}  // namespace

// Reconstructed from eboot.elf at 0x244860. Characters 0xFF are kept as
// they are.
char* FileNormalizePath(char* path) {
    for (char* p = path; *p != '\0'; ++p) {
        const unsigned char c = static_cast<unsigned char>(*p);
        if (c == 0xFF) {
            continue;
        }
        *p = c == '\\' ? '/' : static_cast<char>(std::tolower(c));
    }
    return path;
}

// Reconstructed from eboot.elf at 0x2448D0.
char* FileNormalizeSlash(char* path) {
    for (char* p = path; *p != '\0'; ++p) {
        if (*p == '\\') {
            *p = '/';
        }
    }
    return path;
}

// Reconstructed from eboot.elf at 0x244F00. A "::" is not a drive.
char* FileGetDrive(const char* path, char* buffer) {
    const char* colon = std::strchr(path, ':');
    char* end = buffer;
    if (colon != nullptr && colon[1] != ':') {
        const long length = colon - path;
        std::strncpy(buffer, path, length);
        end = buffer + length;
    }
    *end = '\0';
    return buffer;
}

// Reconstructed from eboot.elf at 0x245300. A separator at the start, or
// after a drive colon, is kept.
char* FileGetPath(const char* path, char* buffer) {
    if (path == nullptr || *path == '\0') {
        buffer[0] = '.';
        buffer[1] = '\0';
        return buffer;
    }
    std::strcpy(buffer, path);
    char* p = buffer + std::strlen(buffer) - 1;
    while (p >= buffer && !IsSeparator(*p)) {
        --p;
    }
    if (p < buffer) {
        buffer[0] = '.';
        buffer[1] = '\0';
    } else if (p == buffer || p[-1] == ':') {
        p[1] = '\0';
    } else {
        p[0] = '\0';
    }
    return buffer;
}

// Reconstructed from eboot.elf at 0x245380.
const char* FileGetExt(const char* path, bool withDot) {
    const char* const end = path + std::strlen(path);
    for (const char* p = end - 1; p >= path && !IsSeparator(*p); --p) {
        if (*p == '.') {
            return withDot ? p : p + 1;
        }
    }
    return end;
}

// Reconstructed from eboot.elf at 0x2453D0.
const char* FileGetName(const char* path) {
    return NameStart(path);
}

// Reconstructed from eboot.elf at 0x245420.
char* FileGetBase(const char* path, char* buffer) {
    const char* const name = NameStart(path);
    unsigned long length = 0;
    while (name[length] != '\0' && name[length] != '.') {
        buffer[length] = name[length];
        ++length;
    }
    buffer[length] = '\0';
    return buffer;
}

// Reconstructed from eboot.elf at 0x245820.
bool FileIsAbsolute(const char* path) {
    if (path == nullptr || *path == '\0') {
        return false;
    }
    if (IsSeparator(path[0])) {
        return true;
    }
    return path[1] == ':' && IsSeparator(path[2]);
}

// Reconstructed from eboot.elf at 0x246200. The characters before the
// colon move right by one and a slash takes the first place.
bool DriveColonToSlash(FixedString& path, bool) {
    char* start = const_cast<char*>(path.c_str());
    char* p = start;
    for (;; ++p) {
        if (*p == '\0') {
            return false;
        }
        if (*p == ':') {
            break;
        }
    }
    for (; p > start; --p) {
        *p = p[-1];
    }
    *p = '/';
    return true;
}

// Reconstructed from eboot.elf at 0x246270. The characters after the
// leading slash move left by one and the first slash after them becomes the
// colon.
bool DriveSlashToColon(FixedString& path, bool) {
    char* p = const_cast<char*>(path.c_str());
    for (;;) {
        const char c = p[1];
        *p = c;
        if (c == '\0') {
            return false;
        }
        ++p;
        if (c == '/') {
            p[-1] = ':';
            return true;
        }
    }
}
