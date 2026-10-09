#include "utl/text/Str.h"

#include <cstring>

#include "utl/containers/Std.h"

namespace {

struct EmptyStringStorage {
    unsigned int capacity;
    char text;
};

EmptyStringStorage gEmptyString{};  // Name not in the reference map.

}  // namespace

// Reconstructed from eboot.elf at 0x2542E0.
FixedString& FixedString::operator+=(const char* str) {
    if (str != nullptr && str[0] != '\0') {
        const auto length = std::strlen(mStr);
        const auto added = std::strlen(str);
        reserve(added + length);
        auto count = capacity() - length;
        if (count >= added) {
            count = added;
        }
        std::strncpy(mStr + length, str, count);
        mStr[count + length] = '\0';
    }
    return *this;
}

// Reconstructed from eboot.elf at 0x2550C0.
String::String(const char* str) {
    mStr = &gEmptyString.text;
    if (str == nullptr || str[0] == '\0') {
        return;
    }

    const auto length = std::strlen(str);
    reserve(length);
    std::memmove(mStr, str, length);
    mStr[length] = '\0';
}

String::String(const String& other) : String(other.c_str()) {}

// Reconstructed from eboot.elf at 0x255550.
String::~String() {
    FreeText();
}

// Reconstructed from eboot.elf at 0x5F3C0.
String& String::operator+=(const char* str) {
    FixedString::operator+=(str);
    return *this;
}

void String::FreeText() {
    const auto old_capacity = capacity();
    if (old_capacity != 0) {
        HmxAllocator::gStlAllocator.deallocate(
            reinterpret_cast<unsigned int*>(mStr) - 1,
            old_capacity + sizeof(unsigned int) + 1);
    }
}

// Reconstructed from eboot.elf at 0x2553B0.
void String::reserve(unsigned long new_capacity) {
    const auto old_capacity = capacity();
    if (new_capacity <= old_capacity) {
        return;
    }

    auto* allocation = static_cast<unsigned char*>(
        HmxAllocator::gStlAllocator.allocate(
            new_capacity + sizeof(unsigned int) + 1));
    *reinterpret_cast<unsigned int*>(allocation) =
        static_cast<unsigned int>(new_capacity);
    auto* replacement =
        reinterpret_cast<char*>(allocation + sizeof(unsigned int));
    std::memcpy(replacement, mStr, old_capacity + 1);
    replacement[new_capacity] = '\0';
    FreeText();
    mStr = replacement;
}
