#include "utl/text/Str.h"

#include <cctype>
#include <cstring>

#include "os/memory/MemMgr.h"

namespace {

struct EmptyStringStorage {
    unsigned int capacity;
    char text;
};

// The shared empty text at 0x19E8250, with its zero capacity word at
// 0x19E824C. Each String constructor clears both before using it.
EmptyStringStorage gEmptyString{};  // Name not in the reference map.

char* EmptyText() {  // Name not in the reference map.
    gEmptyString.capacity = 0;
    gEmptyString.text = '\0';
    return &gEmptyString.text;
}

// Unit suffixes for PrintBytes at 0x18EF6A0. Name not in the reference map.
const char* const sByteUnits[] = {
    " B", " KB", " MB", " GB", " TB", " PB", " EB", " ZB", " YB",
};

// IntToStaticString's table at 0x18EF6F0. Name not in the reference map.
const char* const sStaticInts[] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12",
    "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25",
    "26", "27", "28", "29", "30", "31", "32", "33", "34", "35", "36", "37", "38",
    "39", "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "50", "51",
    "52", "53", "54", "55", "56", "57", "58", "59", "60", "61", "62", "63", "64",
    "65", "66", "67", "68", "69", "70", "71", "72", "73", "74", "75", "76", "77",
    "78", "79", "80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "90",
    "91", "92", "93", "94", "95", "96", "97", "98", "99", "100", "101", "102", "103",
    "104", "105", "106", "107", "108", "109", "110", "111", "112", "113", "114", "115", "116",
    "117", "118", "119", "120", "121", "122", "123", "124", "125", "126", "127", "128", "129",
};

// Ordinal suffixes by last digit at 0x18EFB00. Name not in the reference
// map.
const char* const sOrdinalSuffixes[] = {"th", "st", "nd", "rd", "th"};

// Writes the value in decimal with comma thousands separators and
// terminates it. Inlined into PrintBytes and PrintIntWithCommas. Name not
// in the reference map.
void WriteWithCommas(unsigned long value, char* out) {
    unsigned long digits = 1;
    unsigned long divisor = 1;
    for (unsigned long rest = value / 10; rest != 0; rest /= 10) {
        ++digits;
        divisor *= 10;
    }
    do {
        *out++ = static_cast<char>('0' + value / divisor);
        if (digits >= 4 && digits % 3 == 1) {
            *out++ = ',';
        }
        value %= divisor;
        divisor /= 10;
    } while (--digits != 0);
    *out = '\0';
}

}  // namespace

// Reconstructed from eboot.elf at 0x254280.
FixedString& FixedString::operator+=(char c) {
    const auto length = std::strlen(mStr);
    reserve(length + 1);
    if (length < capacity()) {
        mStr[length] = c;
        mStr[length + 1] = '\0';
    }
    return *this;
}

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

// Reconstructed from eboot.elf at 0x254360.
char FixedString::operator[](unsigned long index) const {
    return mStr[index];
}

// Reconstructed from eboot.elf at 0x254370.
bool FixedString::operator==(const char* str) const {
    return str != nullptr && std::strcmp(mStr, str) == 0;
}

// Reconstructed from eboot.elf at 0x254390.
bool FixedString::operator!=(const char* str) const {
    return str == nullptr || std::strcmp(mStr, str) != 0;
}

// Reconstructed from eboot.elf at 0x2543B0.
bool FixedString::operator==(const FixedString& other) const {
    return std::strcmp(mStr, other.mStr) == 0;
}

// Reconstructed from eboot.elf at 0x2543D0.
bool FixedString::operator!=(const FixedString& other) const {
    return std::strcmp(mStr, other.mStr) != 0;
}

// Reconstructed from eboot.elf at 0x2543F0.
bool FixedString::operator==(Symbol symbol) const {
    return std::strcmp(mStr, symbol.Str()) == 0;
}

// Reconstructed from eboot.elf at 0x254410.
bool FixedString::operator!=(Symbol symbol) const {
    return std::strcmp(mStr, symbol.Str()) != 0;
}

// Reconstructed from eboot.elf at 0x254430. This build counts from the
// capacity, not the length.
char FixedString::rindex(long offset) const {
    return mStr[capacity() + offset];
}

// Reconstructed from eboot.elf at 0x254440.
bool FixedString::operator<(const FixedString& other) const {
    return std::strcmp(mStr, other.mStr) < 0;
}

// Reconstructed from eboot.elf at 0x254460.
bool FixedString::operator>(const FixedString& other) const {
    return std::strcmp(mStr, other.mStr) > 0;
}

// Reconstructed from eboot.elf at 0x254480.
unsigned long FixedString::find(char c, unsigned long start) const {
    const char* current = mStr + start;
    while (*current != c && *current != '\0') {
        ++current;
    }
    return *current == '\0' ? npos : current - mStr;
}

// Reconstructed from eboot.elf at 0x2544C0.
unsigned long FixedString::find(char c) const {
    return find(c, 0);
}

// Reconstructed from eboot.elf at 0x254500.
unsigned long FixedString::find(const char* str) const {
    const char* match = std::strstr(mStr, str);
    return match == nullptr ? npos : match - mStr;
}

// Reconstructed from eboot.elf at 0x254530.
unsigned long FixedString::find(const char* str, unsigned long start) const {
    const char* match = std::strstr(mStr + start, str);
    return match == nullptr ? npos : match - mStr;
}

// Reconstructed from eboot.elf at 0x254560.
unsigned long FixedString::find_first_of(const char* chars, unsigned long start) const {
    if (chars == nullptr) {
        return npos;
    }
    for (const char* current = mStr + start; *current != '\0'; ++current) {
        for (const char* c = chars; *c != '\0'; ++c) {
            if (*current == *c) {
                return current - mStr;
            }
        }
    }
    return npos;
}

// Reconstructed from eboot.elf at 0x2545D0.
unsigned long FixedString::find_first_of(const char* chars) const {
    return find_first_of(chars, 0);
}

// Reconstructed from eboot.elf at 0x254640.
unsigned long FixedString::find_last_of(char c, unsigned long end) const {
    for (const char* current = mStr + end - 1; current >= mStr; --current) {
        if (*current == c) {
            return current - mStr;
        }
    }
    return npos;
}

// Reconstructed from eboot.elf at 0x254670.
unsigned long FixedString::find_last_of(char c) const {
    return find_last_of(c, std::strlen(mStr));
}

// Reconstructed from eboot.elf at 0x2546C0.
unsigned long FixedString::find_last_of(const char* chars, unsigned long end) const {
    if (chars == nullptr) {
        return npos;
    }
    for (const char* current = mStr + end - 1; current >= mStr; --current) {
        for (const char* c = chars; *c != '\0'; ++c) {
            if (*current == *c) {
                return current - mStr;
            }
        }
    }
    return npos;
}

// Reconstructed from eboot.elf at 0x254730.
unsigned long FixedString::find_last_of(const char* chars) const {
    return find_last_of(chars, std::strlen(mStr));
}

// Reconstructed from eboot.elf at 0x2547C0.
unsigned long FixedString::rfind(const char* str) const {
    if (str == nullptr || mStr == nullptr) {
        return npos;
    }
    const auto length = std::strlen(str);
    const long last = std::strlen(mStr) - length;
    if (last < 0) {
        return npos;
    }
    for (const char* current = mStr + last; current >= mStr; --current) {
        if (std::strncmp(current, str, length) == 0) {
            return current - mStr;
        }
    }
    return npos;
}

// Reconstructed from eboot.elf at 0x254850.
bool FixedString::contains(const char* str) const {
    return find(str) != npos;
}

// Reconstructed from eboot.elf at 0x254880.
bool FixedString::startswith(const char* str) const {
    if (str == nullptr) {
        return false;
    }
    for (const char* current = mStr; *str != '\0'; ++current, ++str) {
        if (*current != *str) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x2548C0. The walk back stops at a zero
// byte, so it reads the byte before `str` when every character matches.
bool FixedString::endswith(const char* str) const {
    if (str == nullptr) {
        return false;
    }
    const char* current = mStr + std::strlen(mStr) - 1;
    for (const char* c = str + std::strlen(str) - 1; *c != '\0'; --c, --current) {
        if (*current != *c) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x254930.
int FixedString::compare(unsigned long start, unsigned long count, const char* str) const {
    if (str == nullptr) {
        return -1;
    }
    return std::strncmp(mStr + start, str, count);
}

// Reconstructed from eboot.elf at 0x254950.
FixedString& FixedString::ToLower() {
    for (char* current = mStr; *current != '\0'; ++current) {
        *current = static_cast<char>(std::tolower(*current));
    }
    return *this;
}

// Reconstructed from eboot.elf at 0x2549B0.
FixedString& FixedString::ToUpper() {
    for (char* current = mStr; *current != '\0'; ++current) {
        *current = static_cast<char>(std::toupper(*current));
    }
    return *this;
}

// Reconstructed from eboot.elf at 0x254A10.
void FixedString::append(const char* str, unsigned long count) {
    if (str == nullptr || str[0] == '\0') {
        return;
    }
    const auto length = std::strlen(mStr);
    unsigned long added = 0;
    if (count != 0) {
        added = 1;
        while (added < count && str[added] != '\0') {
            ++added;
        }
    }
    reserve(added + length);
    auto copied = capacity() - length;
    if (copied >= added) {
        copied = added;
    }
    std::strncpy(mStr + length, str, copied);
    mStr[length + copied] = '\0';
}

// Reconstructed from eboot.elf at 0x254AC0.
void FixedString::replace(const char* from, const char* to) {
    for (auto start = find(from); start != npos; start = find(from, start)) {
        replace(start, std::strlen(from), to);
        start += std::strlen(to);
        if (start >= std::strlen(mStr)) {
            break;
        }
    }
}

// Reconstructed from eboot.elf at 0x254C30.
void FixedString::replace(unsigned long start, unsigned long count, const char* str) {
    const auto length = std::strlen(mStr);
    if (start + count > length) {
        count = length - start;
    }
    const long added = std::strlen(str);
    auto newLength = length + added - count;
    reserve(newLength);
    if (capacity() < newLength) {
        newLength = capacity();
    }
    const long moved = newLength - start - added;
    if (moved > 0) {
        std::memmove(mStr + start + added, mStr + start + count, moved);
    }
    mStr[newLength] = '\0';
    long copied = capacity() - start;
    if (copied > added) {
        copied = added;
    }
    if (copied > 0) {
        std::memcpy(mStr + start, str, copied);
    }
}

// Reconstructed from eboot.elf at 0x254D10.
void FixedString::replace(char from, const char* to) {
    const char text[2] = {from, '\0'};
    replace(text, to);
}

// Reconstructed from eboot.elf at 0x254D50.
void FixedString::insert(unsigned long start, const char* str) {
    replace(start, 0, str);
}

// Reconstructed from eboot.elf at 0x254E20.
void FixedString::insert(unsigned long start, const FixedString& str) {
    replace(start, 0, str.mStr);
}

// Reconstructed from eboot.elf at 0x254EF0.
void FixedString::insert(unsigned long start, unsigned long count, char c) {
    auto newLength = std::strlen(mStr) + count;
    reserve(newLength);
    if (capacity() < newLength) {
        newLength = capacity();
    }
    const long moved = newLength + 1 - start - count;
    if (moved > 0) {
        std::memmove(mStr + start + count, mStr + start, moved);
    }
    auto filled = capacity() - start;
    if (filled >= count) {
        filled = count;
    }
    if (filled != 0) {
        std::memset(mStr + start, c, filled);
    }
}

// Reconstructed from eboot.elf at 0x254FA0.
void FixedString::erase() {
    mStr[0] = '\0';
}

// Reconstructed from eboot.elf at 0x254FB0.
void FixedString::erase(unsigned long start) {
    if (start < capacity()) {
        mStr[start] = '\0';
    }
}

// Reconstructed from eboot.elf at 0x254FD0.
void FixedString::erase(unsigned long start, unsigned long count) {
    if (count == 0) {
        return;
    }
    char* const at = mStr + start;
    const auto rest = std::strlen(mStr) - (start + count);
    if (rest != 0) {
        std::memmove(at, at + count, rest + 1);
    } else {
        *at = '\0';
    }
}

// Reconstructed from eboot.elf at 0x255030.
void FixedString::ReplaceAll(char from, char to) {
    for (char* current = mStr; *current != '\0'; ++current) {
        if (*current == from) {
            *current = to;
        }
    }
}

// Reconstructed from eboot.elf at 0x255060.
char& FixedString::operator[](unsigned long index) {
    return mStr[index];
}

// Reconstructed from eboot.elf at 0x255070.
char& FixedString::rindex(long offset) {
    return mStr[capacity() + offset];
}

// Reconstructed from eboot.elf at 0x255080.
String::String() {
    mStr = EmptyText();
}

// Reconstructed from eboot.elf at 0x2550C0.
String::String(const char* str) : String() {
    *this = str;
}

// Reconstructed from eboot.elf at 0x255150.
String::String(Symbol symbol) : String() {
    *this = symbol.Str();
}

// Reconstructed from eboot.elf at 0x2551E0.
String::String(const String& other) : String() {
    *this = other.mStr;
}

// Reconstructed from eboot.elf at 0x255280.
String::String(String&& other) {
    mStr = other.mStr;
    other.mStr = &gEmptyString.text;
}

// Reconstructed from eboot.elf at 0x2552C0.
String& String::operator=(String&& other) {
    if (&other != this) {
        FreeText();
        mStr = other.mStr;
        other.mStr = &gEmptyString.text;
    }
    return *this;
}

// Reconstructed from eboot.elf at 0x255310.
String::String(const FixedString& other) : String() {
    *this = other.c_str();
}

// Reconstructed from eboot.elf at 0x2553B0.
void String::reserve(unsigned long new_capacity) {
    const auto old_capacity = capacity();
    if (new_capacity <= old_capacity) {
        return;
    }

    auto* allocation = static_cast<unsigned char*>(
        MemOrPoolAlloc(new_capacity + sizeof(unsigned int) + 1, "StringBuf", 0));
    auto* replacement = reinterpret_cast<char*>(allocation + sizeof(unsigned int));
    std::memcpy(replacement, mStr, old_capacity + 1);
    replacement[new_capacity] = '\0';
    FreeText();
    mStr = replacement;
    *reinterpret_cast<unsigned int*>(allocation) = static_cast<unsigned int>(new_capacity);
}

// Reconstructed from eboot.elf at 0x255430.
String::String(unsigned long count, char c) : String() {
    if (count != 0) {
        reserve(count);
        for (unsigned long i = 0; i < count; ++i) {
            mStr[i] = c;
        }
    }
    mStr[count] = '\0';
}

// Reconstructed from eboot.elf at 0x2554D0.
String::String(char c) : String() {
    reserve(1);
    mStr[0] = c;
    mStr[1] = '\0';
}

// Reconstructed from eboot.elf at 0x255550.
String::~String() {
    FreeText();
}

void String::FreeText() {
    const auto old_capacity = capacity();
    if (old_capacity != 0) {
        MemOrPoolFree(
            old_capacity + sizeof(unsigned int) + 1,
            reinterpret_cast<unsigned int*>(mStr) - 1,
            nullptr);
    }
}

// Reconstructed from eboot.elf at 0x2555C0.
String String::operator+(const char* str) const {
    String result(*this);
    result += str;
    return result;
}

// Reconstructed from eboot.elf at 0x2556F0.
String String::operator+(Symbol symbol) const {
    String result(*this);
    result += symbol.Str();
    return result;
}

// Reconstructed from eboot.elf at 0x255820.
String String::operator+(char c) const {
    String result(*this);
    result += c;
    return result;
}

// Reconstructed from eboot.elf at 0x255940.
String String::operator+(const FixedString& str) const {
    String result(*this);
    result += str.c_str();
    return result;
}

// Reconstructed from eboot.elf at 0x255A70.
void String::Set(char c) {
    reserve(2);
    mStr[2] = '\0';
    mStr[0] = c;
    mStr[1] = '\0';
}

// Reconstructed from eboot.elf at 0x255AB0.
void String::resize(unsigned long length) {
    reserve(length);
    mStr[length] = '\0';
}

// Reconstructed from eboot.elf at 0x255AE0. The vector is not cleared.
int String::split(const char* delimiters, eastl::vector<String>& pieces) const {
    unsigned long start = 0;
    for (auto end = find_first_of(delimiters); end != npos;
         end = find_first_of(delimiters, start)) {
        if (end > start) {
            pieces.push_back(substr(start, end - start));
        }
        start = end + 1;
    }
    if (start < std::strlen(mStr)) {
        pieces.push_back(substr(start, std::strlen(mStr) - start));
    }
    return pieces.size();
}

// Reconstructed from eboot.elf at 0x255D90.
String String::substr(unsigned long start, unsigned long count) const {
    if (start + count >= capacity()) {
        return String(mStr + start);
    }
    String result(count, ' ');
    std::strncpy(result.mStr, mStr + start, count);
    return result;
}

// Reconstructed from eboot.elf at 0x256080. Each string's capacity word is
// rewritten after the exchange.
void String::swap(String& other) {
    char* const text = mStr;
    const unsigned int textCapacity = capacity();
    mStr = other.mStr;
    reinterpret_cast<unsigned int*>(mStr)[-1] = other.capacity();
    other.mStr = text;
    reinterpret_cast<unsigned int*>(text)[-1] = textCapacity;
}

// Reconstructed from eboot.elf at 0x2560B0.
char* PrintBytes(long value, char* buffer, unsigned long size) {
    unsigned long scaled = value < 0 ? -value : value;
    unsigned long remainder = 0;
    unsigned long unit = 0;
    if (scaled >= 1000) {
        unsigned long previous;
        do {
            previous = scaled;
            ++unit;
            scaled /= 1000;
        } while (previous >= 1000000 && unit < 8);
        remainder = previous % 1000;
    }
    if (value < 0) {
        *buffer++ = '-';
        --size;
    }
    WriteWithCommas(scaled, buffer);
    if (scaled <= 9 && unit != 0) {
        std::strncat(buffer, ".", size);
        const char decimals[4] = {
            static_cast<char>('0' + remainder / 100),
            static_cast<char>('0' + remainder % 100 / 10),
            static_cast<char>('0' + remainder % 10),
            '\0',
        };
        std::strncat(buffer, decimals, size);
    }
    std::strncat(buffer, sByteUnits[unit], size);
    return buffer;
}

// Reconstructed from eboot.elf at 0x2562F0.
char* PrintIntWithCommas(long value, char* buffer, unsigned long) {
    char* out = buffer;
    unsigned long magnitude = value;
    if (value < 0) {
        *out++ = '-';
        magnitude = -value;
    }
    WriteWithCommas(magnitude, out);
    return buffer;
}

// Reconstructed from eboot.elf at 0x256410.
const char* IntToStaticString(int value) {
    if (value < 0) {
        return "-1";
    }
    return sStaticInts[value];
}

// Reconstructed from eboot.elf at 0x256430. The returned pointer is the
// buffer itself, capacity word included, not the text after it.
char* IntToOrdinalStaticString(int value, char* buffer, int size) {
    const int magnitude = value < 0 ? -value : value;
    FixedString text(buffer, size);
    text << value;
    const int digit = magnitude % 10;
    text << sOrdinalSuffixes[digit < 5 ? digit : 4];
    return buffer;
}

// Reconstructed from eboot.elf at 0x256510.
long StringToInt(const char* str, bool* failed) {
    if (failed != nullptr) {
        *failed = false;
    }
    while (std::isspace(*str)) {
        ++str;
    }
    bool negative = false;
    if (*str == '+') {
        ++str;
    } else if (*str == '-') {
        negative = true;
        ++str;
    }
    long value = 0;
    if (*str == '\0') {
        if (failed != nullptr) {
            *failed = true;
        }
    } else {
        for (; *str >= '0' && *str <= '9'; ++str) {
            value = value * 10 + (*str - '0');
        }
        if (*str != '\0' && failed != nullptr) {
            *failed = true;
        }
    }
    return negative ? -value : value;
}

// Reconstructed from eboot.elf at 0x2565E0.
void RemoveSpaces(char* out, int size, const char* in) {
    char* const last = out + size - 1;
    char* current = out;
    if (last > out && *in != '\0') {
        bool previousSpace = true;
        do {
            const char c = *in++;
            const bool space = c == ' ';
            if (!previousSpace || !space) {
                *current++ = c;
            }
            previousSpace = space;
        } while (current < last && *in != '\0');
        if (current > out && current[-1] == ' ') {
            --current;
        }
    }
    *current = '\0';
}

// Reconstructed from eboot.elf at 0x256650.
void RemoveSpaces(String& str) {
    int spaces = 0;
    for (int i = 0; i < static_cast<int>(std::strlen(str.c_str())); ++i) {
        if (str.c_str()[i] == ' ') {
            ++spaces;
        } else if (spaces > 0) {
            const int start = i - spaces;
            if (start == 0) {
                str.erase(0, spaces);
                i = 0;
            } else if (spaces >= 2) {
                str.erase(start + 1, spaces - 1);
                i = start + 1;
            }
            spaces = 0;
        }
    }
    if (spaces > 0) {
        str.erase(std::strlen(str.c_str()) - spaces);
    }
}

// Reconstructed from eboot.elf at 0x256740.
void FilterString(char* out, int size, const char* in, const char* allowed, char replacement) {
    long count = 0;
    if (size >= 2) {
        while (in[count] != '\0') {
            const char c = in[count];
            out[count] = std::strchr(allowed, c) != nullptr ? c : replacement;
            if (++count >= size - 1) {
                break;
            }
        }
    }
    out[count] = '\0';
}

// Reconstructed from eboot.elf at 0x2567D0.
bool StrNCopy(char* out, const char* in, int size) {
    int remaining = size - 1;
    while (*in != '\0' && remaining != 0) {
        *out++ = *in++;
        --remaining;
    }
    *out = '\0';
    return remaining != 0 || *in == '\0';
}
