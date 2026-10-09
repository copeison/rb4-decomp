#include "utl/text/UTF8.h"

#include <cstring>

#include "utl/containers/Set.h"

namespace {

// Characters that may not begin a line, at 0x136DD00: closing punctuation,
// small kana and prolonged sound marks. Name not in the reference map.
const unsigned short sNoLineBeginChars[] = {
    0x0021, 0x0025, 0x0029, 0x002C, 0x002E, 0x003A, 0x003B, 0x003F,
    0x005D, 0x007D, 0x0022, 0x00A2, 0x00B0, 0x00B7, 0x00BB, 0x2010,
    0x2013, 0x2014, 0x2019, 0x2020, 0x2021, 0x2022, 0x203A, 0x203C,
    0x2047, 0x2048, 0x2049, 0x2103, 0x2236, 0x3001, 0x3002, 0x3003,
    0x3005, 0x3006, 0x3008, 0x3009, 0x300A, 0x300B, 0x300C, 0x300D,
    0x300E, 0x300F, 0x3011, 0x3015, 0x3017, 0x3019, 0x301C, 0x301E,
    0x301F, 0x303B, 0x3041, 0x3043, 0x3045, 0x3047, 0x3049, 0x3063,
    0x3083, 0x3085, 0x3087, 0x308E, 0x3095, 0x3096, 0x30A0, 0x30A1,
    0x30A3, 0x30A5, 0x30A7, 0x30A9, 0x30C3, 0x30E3, 0x30E5, 0x30E7,
    0x30EE, 0x30F5, 0x30F6, 0x30FB, 0x30FC, 0x30FD, 0x30FE, 0x31F0,
    0x31F1, 0x31F2, 0x31F3, 0x31F4, 0x31F5, 0x31F6, 0x31F7, 0x31F8,
    0x31F9, 0x31FA, 0x31FB, 0x31FC, 0x31FD, 0x31FE, 0x31FF, 0xFE30,
    0xFE31, 0xFE32, 0xFE33, 0xFE36, 0xFE38, 0xFE3A, 0xFE3C, 0xFE3E,
    0xFE40, 0xFE42, 0xFE50, 0xFE51, 0xFE52, 0xFE53, 0xFE54, 0xFE55,
    0xFE56, 0xFE57, 0xFE58, 0xFE5A, 0xFE5C, 0xFF01, 0xFF02, 0xFF05,
    0xFF07, 0xFF09, 0xFF0C, 0xFF0E, 0xFF1A, 0xFF1B, 0xFF1F, 0xFF3D,
    0xFF5C, 0xFF5D, 0xFF5E, 0xFF60, 0xFF64,
};

// Characters that may not end a line, at 0x136DE10: opening punctuation and
// currency signs. Name not in the reference map.
const unsigned short sNoLineEndChars[] = {
    0x0023, 0x0022, 0x0024, 0x0028, 0x005B, 0x005C, 0x007B, 0x00A3,
    0x00A3, 0x00A3, 0x00A5, 0x00A5, 0x00A5, 0x00AB, 0x00B7, 0x2018,
    0x2018, 0x2018, 0x2018, 0x2035, 0x3005, 0x3007, 0x3008, 0x3008,
    0x3008, 0x3009, 0x300A, 0x300A, 0x300A, 0x300B, 0x300C, 0x300C,
    0x300C, 0x300D, 0x300E, 0x300E, 0x300E, 0x3010, 0x3010, 0x3014,
    0x3014, 0x3014, 0x3014, 0x3016, 0x3016, 0x3018, 0x301D, 0x301D,
    0x301D, 0xFE34, 0xFE35, 0xFE37, 0xFE39, 0xFE3B, 0xFE3D, 0xFE3F,
    0xFE41, 0xFE43, 0xFE4F, 0xFE59, 0xFE59, 0xFE5B, 0xFE5B, 0xFF04,
    0xFF04, 0xFF08, 0xFF08, 0xFF08, 0xFF0E, 0xFF3B, 0xFF3B, 0xFF5B,
    0xFF5B, 0xFF5B, 0xFF5B, 0xFF5F, 0xFF60, 0xFFE1, 0xFFE5, 0xFFE5,
    0xFFE6,
};

// The sets at 0x1B5D2A8 and 0x1B5D2E0, built by the static initializer at
// 0x1186E60. Their ranges end sizeof(array) elements, not bytes, past the
// start, so each set also takes in the data that follows its array in the
// binary. Names not in the reference map.
const eastl::set<unsigned short> sNoLineBeginSet(
    sNoLineBeginChars, sNoLineBeginChars + sizeof(sNoLineBeginChars));
const eastl::set<unsigned short> sNoLineEndSet(
    sNoLineEndChars, sNoLineEndChars + sizeof(sNoLineEndChars));

// The byte length of the UTF-8 sequence starting with `lead`, as the
// decoder counts it. Name not in the reference map.
unsigned long UTF8CharLength(char lead) {
    if ((lead & 0x80) == 0) {
        return 1;
    }
    if ((lead & 0xE0) == 0xC0) {
        return 2;
    }
    return (lead & 0xF0) == 0xE0 ? 3 : 1;
}

}  // namespace

// Reconstructed from eboot.elf at 0x11848E0.
bool IsCJKChar(unsigned short ch) {
    return (ch >= 0x3040 && ch < 0x30A0) || (ch >= 0x3000 && ch < 0x3040) ||
        (ch >= 0xF900 && ch < 0xFB00) || (ch >= 0x4E00 && ch < 0xA000) ||
        (ch >= 0x3400 && ch < 0x4DC0) || (ch >= 0x30A0 && ch < 0x3100);
}

// Reconstructed from eboot.elf at 0x1184950.
bool CanBeginLine(unsigned short ch) {
    return sNoLineBeginSet.find(ch) == sNoLineBeginSet.end();
}

// Reconstructed from eboot.elf at 0x11849D0.
bool CanEndLine(unsigned short ch) {
    return sNoLineEndSet.find(ch) == sNoLineEndSet.end();
}

// Reconstructed from eboot.elf at 0x1184A50.
int DecodeUTF8(unsigned short& ch, const char* str) {
    const auto* bytes = reinterpret_cast<const unsigned char*>(str);
    const unsigned short lead = bytes[0];
    if ((lead & 0x80) == 0) {
        ch = lead;
        return 1;
    }
    if ((lead & 0xE0) == 0xC0) {
        ch = static_cast<unsigned short>(((lead - 0xC0) << 6) + (bytes[1] - 0x80));
        return 2;
    }
    if ((lead & 0xF0) == 0xE0) {
        ch = static_cast<unsigned short>(
            ((lead - 0xE0) << 12) + ((bytes[1] - 0x80) << 6) + (bytes[2] - 0x80));
        return 3;
    }
    ch = '*';
    return 1;
}

// Reconstructed from eboot.elf at 0x1184AC0.
int EncodeUTF8(char* out, unsigned int ch) {
    if (ch <= 0x7F) {
        out[0] = static_cast<char>(ch);
        out[1] = '\0';
        return 1;
    }
    if (ch <= 0x7FF) {
        out[0] = static_cast<char>((ch >> 6) + 0xC0);
        out[1] = static_cast<char>((ch & 0x3F) | 0x80);
        out[2] = '\0';
        return 2;
    }
    if (ch <= 0xFFFF) {
        out[0] = static_cast<char>((ch >> 12) + 0xE0);
        out[1] = static_cast<char>(((ch >> 6) & 0x3F) | 0x80);
        out[2] = static_cast<char>((ch & 0x3F) | 0x80);
        out[3] = '\0';
        return 3;
    }
    out[0] = '*';
    out[1] = '\0';
    return 1;
}

// Reconstructed from eboot.elf at 0x1184B40.
int EncodeUTF8(String& out, unsigned int ch) {
    char encoded[4];
    const int length = EncodeUTF8(encoded, ch);
    out = encoded;
    return length;
}

// Reconstructed from eboot.elf at 0x1184C50.
void UTF8toASCIIs(char* out, unsigned long size, const char* in, char replacement) {
    unsigned long count = 0;
    if (size - 1 != 0) {
        while (*in != '\0') {
            unsigned short ch;
            in += DecodeUTF8(ch, in);
            out[count++] = ch < 0x100 ? static_cast<char>(ch) : replacement;
            if (count >= size - 1) {
                break;
            }
        }
    }
    out[count] = '\0';
}

// Reconstructed from eboot.elf at 0x1184D40.
void ASCIItoUTF8(char* out, unsigned long size, const char* in) {
    std::memset(out, 0, size);
    char* current = out;
    for (; *in != '\0'; ++in) {
        char encoded[4];
        const unsigned long length = EncodeUTF8(encoded, static_cast<unsigned char>(*in));
        if (current + length - out >= static_cast<long>(size)) {
            break;
        }
        for (unsigned long i = 0; i < length; ++i) {
            current[i] = encoded[i];
        }
        current += length;
    }
}

// Reconstructed from eboot.elf at 0x1184E00.
unsigned short* CharToWideChar(const char* str, unsigned short* out, unsigned long) {
    if (str == nullptr) {
        return nullptr;
    }
    const auto length = std::strlen(str);
    for (unsigned long i = 0; i < length; ++i) {
        out[i] = static_cast<unsigned char>(str[i]);
    }
    out[length] = 0;
    return out;
}

// Reconstructed from eboot.elf at 0x1184F00.
char* WideCharToChar(const unsigned short* str, char* out, unsigned long size) {
    if (str == nullptr) {
        return nullptr;
    }
    char* current = out;
    unsigned long room = size - 16;
    for (; *str != 0; ++str) {
        if (room == 0) {
            std::memcpy(current, "... [TRUNCATED]", 15);
            current += 15;
            break;
        }
        *current++ = *str <= 0xFF ? static_cast<char>(*str) : '*';
        --room;
    }
    *current = '\0';
    return out;
}

// Reconstructed from eboot.elf at 0x1184F90.
char* ConvertStr(const unsigned short* in, char* out, unsigned long size) {
    char* current = out;
    if (in != nullptr) {
        for (unsigned long room = size - 1; *in != 0 && room != 0; ++in, --room) {
            *current++ = *in <= 0xFF ? static_cast<char>(*in) : '*';
        }
    }
    *current = '\0';
    return out;
}

// Reconstructed from eboot.elf at 0x1185000.
char* ConvertStr(const wchar_t* in, char* out, unsigned long size) {
    char* current = out;
    if (in != nullptr) {
        for (unsigned long room = size - 1; *in != 0 && room != 0; ++in, --room) {
            *current++ = *in <= 0xFF ? static_cast<char>(*in) : '*';
        }
    }
    *current = '\0';
    return out;
}

// Reconstructed from eboot.elf at 0x1185070.
char* ConvertStr(const char* in, char* out, unsigned long size) {
    if (in != nullptr) {
        std::strncpy(out, in, size);
    } else {
        out[0] = '\0';
    }
    return out;
}

// Reconstructed from eboot.elf at 0x11850A0.
unsigned short* ConvertStr(const char* in, unsigned short* out, unsigned long size) {
    unsigned short* current = out;
    if (in != nullptr) {
        for (unsigned long room = size - 1; *in != '\0' && room != 0; ++in, --room) {
            *current++ = static_cast<unsigned short>(*in);
        }
    }
    *current = 0;
    return out;
}

// Reconstructed from eboot.elf at 0x1185100.
wchar_t* ConvertStr(const char* in, wchar_t* out, unsigned long size) {
    wchar_t* current = out;
    if (in != nullptr) {
        for (unsigned long room = size - 1; *in != '\0' && room != 0; ++in, --room) {
            *current++ = static_cast<wchar_t>(*in);
        }
    }
    *current = 0;
    return out;
}

// Reconstructed from eboot.elf at 0x1185160.
int UTF8StrLen(const char* str) {
    int length = 0;
    for (; *str != '\0'; str += UTF8CharLength(*str)) {
        ++length;
    }
    return length;
}

// Reconstructed from eboot.elf at 0x11851C0.
int WideStrLen(const unsigned short* str) {
    int length = 0;
    while (str[length] != 0) {
        ++length;
    }
    return length;
}

// Reconstructed from eboot.elf at 0x11851E0.
const char* UTF8strchr(const char* str, unsigned short ch) {
    while (*str != '\0') {
        unsigned short current;
        const int length = DecodeUTF8(current, str);
        if (current == ch) {
            return str;
        }
        str += length;
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x1185270.
void UTF8SearchReplace(
    char* out,
    unsigned long size,
    const char* in,
    const char* find,
    const char* replacement) {
    if (size == 0) {
        return;
    }
    char* const end = out + size;
    const auto findLength = std::strlen(find);
    const auto replacementLength = std::strlen(replacement);
    if (static_cast<long>(size) > 0) {
        while (*in != '\0') {
            const char* source;
            char* next;
            unsigned long consumed;
            if (std::strncmp(in, find, findLength) == 0) {
                source = replacement;
                next = out + replacementLength;
                consumed = findLength;
            } else {
                source = in;
                consumed = UTF8CharLength(*in);
                next = out + consumed;
            }
            if (next >= end) {
                next = end;
            }
            std::strncpy(out, source, next - out);
            out = next;
            if (out >= end) {
                break;
            }
            in += consumed;
        }
    }
    *(out < end - 1 ? out : end - 1) = '\0';
}

// Reconstructed from eboot.elf at 0x11853C0.
unsigned short WToLower(unsigned short ch) {
    if (ch >= 'A' && ch <= 'Z') {
        return ch + 32;
    }
    if (ch >= 0xC0 && ch <= 0xDE) {
        // The multiplication sign has no case.
        return ch == 0xD7 ? 0xD7 : ch + 32;
    }
    if ((ch >= 0x100 && ch <= 0x137 && (ch & 1) == 0) ||
        (ch >= 0x139 && ch <= 0x148 && (ch & 1) != 0) ||
        (ch >= 0x14A && ch <= 0x177 && (ch & 1) == 0) ||
        (ch >= 0x179 && ch <= 0x17E && (ch & 1) != 0)) {
        return ch + 1;
    }
    if (ch == 0x178) {
        return 0xFF;
    }
    return ch;
}

// Reconstructed from eboot.elf at 0x1185460.
void UTF8ToLower(unsigned short ch, char* out) {
    String encoded;
    EncodeUTF8(encoded, WToLower(ch));
    std::memcpy(out, encoded.c_str(), std::strlen(encoded.c_str()));
}

// Reconstructed from eboot.elf at 0x1185640.
unsigned short WToUpper(unsigned short ch) {
    if (ch >= 'a' && ch <= 'z') {
        return ch - 32;
    }
    if (ch >= 0xE0 && ch <= 0xFE) {
        // The division sign has no case.
        return ch == 0xF7 ? 0xF7 : ch - 32;
    }
    if ((ch >= 0x100 && ch <= 0x137 && (ch & 1) != 0) ||
        (ch >= 0x139 && ch <= 0x148 && (ch & 1) == 0) ||
        (ch >= 0x14A && ch <= 0x177 && (ch & 1) != 0) ||
        (ch >= 0x179 && ch <= 0x17E && (ch & 1) == 0)) {
        return ch - 1;
    }
    if (ch == 0xFF) {
        return 0x178;
    }
    return ch;
}

// Reconstructed from eboot.elf at 0x11856E0.
void UTF8ToUpper(unsigned short ch, char* out) {
    String encoded;
    EncodeUTF8(encoded, WToUpper(ch));
    std::memcpy(out, encoded.c_str(), std::strlen(encoded.c_str()));
}

// Reconstructed from eboot.elf at 0x11858C0.
int UTF8StrniCmp(const char* a, const unsigned short* b, int count) {
    for (; count != 0; --count, ++b) {
        unsigned short ch;
        a += DecodeUTF8(ch, a);
        const unsigned short lower = WToLower(ch);
        const unsigned short other = WToLower(*b);
        if (lower != other) {
            return lower - other;
        }
        if (lower == 0) {
            break;
        }
    }
    return 0;
}

// Reconstructed from eboot.elf at 0x1185B20.
int WStrnCmp(const unsigned short* a, const unsigned short* b, int count) {
    for (; count != 0; --count, ++a, ++b) {
        if (*a != *b) {
            return *a - *b;
        }
        if (*a == 0) {
            break;
        }
    }
    return 0;
}

// Reconstructed from eboot.elf at 0x1185B50.
int WStrniCmp(const unsigned short* a, const unsigned short* b, int count) {
    for (; count != 0; --count, ++a, ++b) {
        const unsigned short lower = WToLower(*a);
        const unsigned short other = WToLower(*b);
        if (lower != other) {
            return lower - other;
        }
        if (lower == 0) {
            break;
        }
    }
    return 0;
}

// Reconstructed from eboot.elf at 0x1185D50.
void UTF8FilterString(
    char* out,
    unsigned long size,
    const char* in,
    const char* allowed,
    char replacement) {
    char* const end = out + size;
    while (*in != '\0' && out + 3 < end) {
        unsigned short ch;
        const int length = DecodeUTF8(ch, in);
        if (UTF8strchr(allowed, ch) == nullptr) {
            *out++ = replacement;
        } else {
            for (int i = 0; i < length; ++i) {
                out[i] = in[i];
            }
            out += length;
        }
        in += length;
    }
    *out = '\0';
}

// Reconstructed from eboot.elf at 0x1185F00.
void UTF8RemoveSpaces(char* out, unsigned long size, const char* in) {
    char* const last = out + size - 3;
    char* current = out;
    if (last > out && *in != '\0') {
        bool previousSpace = true;
        do {
            unsigned short ch;
            const int length = DecodeUTF8(ch, in);
            const bool space = ch == ' ';
            if (!previousSpace || !space) {
                for (int i = 0; i < length; ++i) {
                    *current++ = in[i];
                }
            }
            previousSpace = space;
            if (current >= last) {
                break;
            }
            in += length;
        } while (*in != '\0');
        if (current > out && current[-1] == ' ') {
            --current;
        }
    }
    *current = '\0';
}

// Reconstructed from eboot.elf at 0x11860C0.
unsigned long UTF8Resize(char* out, unsigned long count, unsigned long size, const char* in) {
    std::memset(out, 0, size);
    unsigned long copied = 0;
    bool full = false;
    for (unsigned long i = 0; i < count && !full && *in != '\0'; ++i) {
        const auto length = UTF8CharLength(*in);
        if (copied + length >= size) {
            full = true;
        } else {
            for (unsigned int j = 0; j < length; ++j) {
                out[j] = in[j];
            }
            out += length;
            in += length;
            copied += length;
        }
    }
    return copied;
}

// Reconstructed from eboot.elf at 0x11861C0.
unsigned short* UTF8toWide(
    const char* in,
    unsigned short* out,
    unsigned long size,
    unsigned long* length) {
    unsigned short* current = out;
    while (*in != '\0' && current - out < static_cast<long>(size - 1)) {
        unsigned short ch;
        in += DecodeUTF8(ch, in);
        *current++ = ch;
    }
    *current = 0;
    if (length != nullptr) {
        *length = current - out;
    }
    return out;
}

// Reconstructed from eboot.elf at 0x11862C0.
wchar_t* UTF8toWChar_t(wchar_t* out, const char* in) {
    wchar_t* current = out;
    while (*in != '\0') {
        unsigned short ch;
        in += DecodeUTF8(ch, in);
        *current++ = ch;
    }
    *current = 0;
    return out;
}

// Reconstructed from eboot.elf at 0x1186370.
wchar_t* UTF8toWChar_t(wchar_t* out, unsigned long size, const char* in) {
    wchar_t* const last = out + size - 1;
    wchar_t* current = out;
    if (last > out) {
        while (*in != '\0') {
            unsigned short ch;
            const int length = DecodeUTF8(ch, in);
            *current++ = ch;
            if (current >= last) {
                break;
            }
            in += length;
        }
    }
    *current = 0;
    return out;
}

// Reconstructed from eboot.elf at 0x1186450.
void WChar_tToUTF8(String& out, const wchar_t* in) {
    String encoded;
    for (; *in != 0; ++in) {
        EncodeUTF8(encoded, *in);
        out.Print(encoded.c_str());
    }
}

// Reconstructed from eboot.elf at 0x11865B0.
void UTF8toWideVector(eastl::vector<unsigned short>& out, const char* in) {
    out.clear();
    while (*in != '\0') {
        unsigned short ch;
        in += DecodeUTF8(ch, in);
        out.push_back(ch);
    }
}

// Reconstructed from eboot.elf at 0x1186730. Each character goes through
// its UTF-8 encoding and back.
void ASCIItoWideVector(eastl::vector<unsigned short>& out, const char* in) {
    out.clear();
    for (; *in != '\0'; ++in) {
        String encoded;
        EncodeUTF8(encoded, static_cast<unsigned char>(*in));
        unsigned short ch;
        DecodeUTF8(ch, encoded.c_str());
        out.push_back(ch);
    }
}

// Reconstructed from eboot.elf at 0x1186960.
String WideVectorToASCII(eastl::vector<unsigned short>& in) {
    String text;
    for (unsigned long i = 0; i < in.size(); ++i) {
        text += in[i] < 0x100 ? static_cast<char>(in[i]) : '*';
    }
    return text;
}

// Reconstructed from eboot.elf at 0x11869D0.
int WideVectorToUTF8(eastl::vector<unsigned short>& in, String& out) {
    String encoded;
    int length = 0;
    for (unsigned long i = 0; i < in.size(); ++i) {
        length += in[i] < 0x80 ? 1 : (in[i] > 0x7FF ? 3 : 2);
    }
    out.resize(length + 1);
    out.erase();
    for (unsigned long i = 0; i < in.size(); ++i) {
        EncodeUTF8(encoded, in[i]);
        out.Print(encoded.c_str());
    }
    return length;
}
