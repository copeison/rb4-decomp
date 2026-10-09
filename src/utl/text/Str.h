#pragma once

#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"
#include "utl/text/TextStream.h"

// Text view over caller-provided storage (utl/Str.o). The capacity word is
// stored in the four bytes before the text. Printing to it appends. The
// vtable is at 0x18E7FE0; its destructors are inline (0x151180, 0x1512E0).
// This build defines the editing methods the map gives to String (replace,
// insert) on FixedString, among its other methods.
class FixedString : public TextStream {
public:
    // A data symbol in the map's build.
    static constexpr unsigned long npos = static_cast<unsigned long>(-1);

    FixedString() {}
    // Views `size` bytes at `buffer`: the capacity word, then the text and
    // its terminator. Inline; IntToOrdinalStaticString builds one. Name not
    // in the reference map.
    FixedString(char* buffer, int size) {
        *reinterpret_cast<unsigned int*>(buffer) = size - 5;
        mStr = buffer + sizeof(unsigned int);
        mStr[0] = '\0';
    }
    ~FixedString() override {}

    // Slot 2: a thunk at 0x5F3C0 to operator+=, shared by every subclass.
    // Inline in the map's build.
    void Print(const char* str) override {
        *this += str;
    }
    // Slot 3 at 0x5F3D0: fixed storage does not grow. Inline in the map's
    // build.
    virtual void reserve(unsigned long capacity) {
        static_cast<void>(capacity);
    }

    // Replaces the text through reserve, truncating to the capacity; null
    // empties it. Out of line in the map's build; inlined into every caller
    // in this one (String's constructors, RndMemOverlay's constructor at
    // 0x6E54A0).
    FixedString& operator=(const char* str) {
        if (str != mStr) {
            char* end = mStr;
            if (str != nullptr && str[0] != '\0') {
                unsigned long length = __builtin_strlen(str);
                reserve(length);
                if (capacity() < length) {
                    length = capacity();
                }
                __builtin_memmove(mStr, str, length);
                end = mStr + length;
            }
            *end = '\0';
        }
        return *this;
    }
    // Appends one character through reserve; a full string is left
    // unchanged.
    FixedString& operator+=(char c);  // 0x254280
    // Appends through reserve, truncating to the capacity.
    FixedString& operator+=(const char* str);  // 0x2542E0
    char operator[](unsigned long index) const;  // 0x254360
    // Null never matches.
    bool operator==(const char* str) const;  // 0x254370
    bool operator!=(const char* str) const;  // 0x254390
    bool operator==(const FixedString& other) const;  // 0x2543B0
    bool operator!=(const FixedString& other) const;  // 0x2543D0
    bool operator==(Symbol symbol) const;  // 0x2543F0
    bool operator!=(Symbol symbol) const;  // 0x254410
    // The character `offset` places from the capacity end.
    char rindex(long offset) const;  // 0x254430
    bool operator<(const FixedString& other) const;  // 0x254440
    bool operator>(const FixedString& other) const;  // 0x254460

    // The searches return npos when nothing matches.
    unsigned long find(char c, unsigned long start) const;  // 0x254480
    unsigned long find(char c) const;  // 0x2544C0
    unsigned long find(const char* str) const;  // 0x254500
    unsigned long find(const char* str, unsigned long start) const;  // 0x254530
    unsigned long find_first_of(const char* chars, unsigned long start) const;  // 0x254560
    unsigned long find_first_of(const char* chars) const;  // 0x2545D0
    // Searches backwards from the character before `end`.
    unsigned long find_last_of(char c, unsigned long end) const;  // 0x254640
    unsigned long find_last_of(char c) const;  // 0x254670
    // Name not in the reference map.
    unsigned long find_last_of(const char* chars, unsigned long end) const;  // 0x2546C0
    unsigned long find_last_of(const char* chars) const;  // 0x254730
    unsigned long rfind(const char* str) const;  // 0x2547C0
    bool contains(const char* str) const;  // 0x254850
    // Prefix and suffix tests; null is false. Names not in the reference
    // map.
    bool startswith(const char* str) const;  // 0x254880
    bool endswith(const char* str) const;  // 0x2548C0
    // strncmp of `count` characters at `start`; null gives -1.
    int compare(unsigned long start, unsigned long count, const char* str) const;  // 0x254930
    FixedString& ToLower();  // 0x254950
    FixedString& ToUpper();  // 0x2549B0

    // Appends at most `count` characters through reserve. Name not in the
    // reference map.
    void append(const char* str, unsigned long count);  // 0x254A10
    // Replaces every occurrence of `from` with `to`. The map has this on
    // String.
    void replace(const char* from, const char* to);  // 0x254AC0
    // Replaces `count` characters at `start` with `str`, truncating to the
    // capacity. The map has this on String.
    void replace(unsigned long start, unsigned long count, const char* str);  // 0x254C30
    // Replaces every `from` character with `to`. Name not in the reference
    // map.
    void replace(char from, const char* to);  // 0x254D10
    // The map has the insertions on String.
    void insert(unsigned long start, const char* str);  // 0x254D50
    void insert(unsigned long start, const FixedString& str);  // 0x254E20
    void insert(unsigned long start, unsigned long count, char c);  // 0x254EF0
    // Empties the text, keeping the storage.
    void erase();  // 0x254FA0
    // Truncates at `start` when it is inside the capacity.
    void erase(unsigned long start);  // 0x254FB0
    void erase(unsigned long start, unsigned long count);  // 0x254FD0
    void ReplaceAll(char from, char to);  // 0x255030
    char& operator[](unsigned long index);  // 0x255060
    char& rindex(long offset);  // 0x255070

    const char* c_str() const {
        return mStr;
    }

protected:
    unsigned int capacity() const {  // Name not in the reference map.
        return reinterpret_cast<const unsigned int*>(mStr)[-1];
    }

    char* mStr;
};

static_assert(sizeof(FixedString) == 16);

// Heap string. Empty strings share a static zero-capacity buffer; the heap
// buffers come from MemOrPoolAlloc with the label "StringBuf". The vtable
// is at 0x18EF668.
class String : public FixedString {
public:
    String();  // 0x255080
    explicit String(const char* str);  // 0x2550C0
    explicit String(Symbol symbol);  // 0x255150
    String(const String& other);  // 0x2551E0
    // Takes the other string's text and leaves it the shared empty buffer.
    String(String&& other);  // 0x255280
    // Frees the text and takes the other string's. Name not in the
    // reference map.
    String& operator=(String&& other);  // 0x2552C0
    explicit String(const FixedString& other);  // 0x255310
    // `count` copies of `c`.
    String(unsigned long count, char c);  // 0x255430
    // Name not in the reference map.
    explicit String(char c);  // 0x2554D0
    ~String() override;  // slots 0-1: 0x255550, 0x255580

    using FixedString::operator=;

    // Slot 3 at 0x2553B0.
    void reserve(unsigned long capacity) override;

    String operator+(const char* str) const;  // 0x2555C0
    String operator+(Symbol symbol) const;  // 0x2556F0
    String operator+(char c) const;  // 0x255820
    String operator+(const FixedString& str) const;  // 0x255940
    void Set(char c);  // 0x255A70
    // Reserves the length and terminates the text there.
    void resize(unsigned long length);  // 0x255AB0
    // Appends the non-empty pieces between delimiter characters and returns
    // the vector's size.
    int split(const char* delimiters, eastl::vector<String>& pieces) const;  // 0x255AE0
    // A count reaching the capacity takes the rest of the text.
    String substr(unsigned long start, unsigned long count) const;  // 0x255D90
    void swap(String& other);  // 0x256080

private:
    void FreeText();  // Name not in the reference map.
};

static_assert(sizeof(String) == 16);

// FixedString over an inline buffer of N characters and a terminator; the
// capacity word sits in front of the buffer. The constructor and destructor
// are inline. StackString<256>'s vtable is at 0x18E6AC8.
template <int N>
class StackString : public FixedString {
public:
    StackString() {
        mStr = mBuffer;
        mCapacity = N;
        mBuffer[0] = '\0';
    }
    // Copies the text, truncated to the capacity; null is empty.
    explicit StackString(const char* str) : StackString() {
        if (str != nullptr && *str != '\0') {
            unsigned long length = __builtin_strlen(str);
            if (length > N) {
                length = N;
            }
            __builtin_memcpy(mBuffer, str, length);
            mBuffer[length] = '\0';
        }
    }

    using FixedString::operator=;

private:
    // Field names are not in the reference map.
    unsigned int mCapacity;
    char mBuffer[N + 1];
};

static_assert(sizeof(StackString<256>) == 280);

// Writes the value with comma thousands separators, scaled to the largest
// unit (B, KB, MB ... YB, the table at 0x18EF6A0) that leaves it at least
// 1; single-digit scaled values get three decimals. Returns the text after
// any minus sign. The map has PrintBytes(unsigned long) in os/MemMgr.o; this
// build's version takes a buffer and sits in Str.o, so the name is a
// best fit.
char* PrintBytes(long value, char* buffer, unsigned long size);  // 0x2560B0
// Writes the value in decimal with comma thousands separators into
// `buffer` and returns it. The map's signature is PrintIntWithCommas(char*,
// unsigned long, long); this build takes the value first and ignores the
// size.
char* PrintIntWithCommas(long value, char* buffer, unsigned long size);  // 0x2562F0

// Interned decimal text for 0-129 (the table at 0x18EF6F0). Negative values
// map to "-1"; larger ones are not checked.
const char* IntToStaticString(int value);  // 0x256410
// Writes the value and its ordinal suffix (by the last digit only) into
// `size` bytes at `buffer`, viewed as a FixedString, and returns `buffer`.
// The map's signature is IntToOrdinalStaticString(int).
char* IntToOrdinalStaticString(int value, char* buffer, int size);  // 0x256430
// Parses an optionally signed decimal integer after leading white space.
// `failed`, when given, reports empty text or trailing characters. Name not
// in the reference map.
long StringToInt(const char* str, bool* failed);  // 0x256510

// Copies `in` without leading, trailing or repeated spaces.
void RemoveSpaces(char* out, int size, const char* in);  // 0x2565E0
void RemoveSpaces(String& str);  // 0x256650
// Copies `in`, replacing characters missing from `allowed`.
void FilterString(char* out, int size, const char* in, const char* allowed, char replacement);  // 0x256740
// Copies at most size - 1 characters; returns false when truncated.
bool StrNCopy(char* out, const char* in, int size);  // 0x2567D0
