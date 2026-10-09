#pragma once

#include "utl/text/TextStream.h"

// Text view over caller-provided storage. The capacity word is stored in the
// four bytes before the text. Printing to it appends.
class FixedString : public TextStream {
public:
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

    // Replaces the text through reserve, truncating to the capacity.
    // Out of line in the map's build; inlined into every caller in this
    // one (for example RndMemOverlay's constructor at 0x6E54A0).
    FixedString& operator=(const char* str) {
        if (str != mStr) {
            unsigned long length = __builtin_strlen(str);
            reserve(length);
            if (capacity() < length) {
                length = capacity();
            }
            __builtin_memcpy(mStr, str, length);
            mStr[length] = '\0';
        }
        return *this;
    }
    // Appends through reserve, truncating to the capacity.
    FixedString& operator+=(const char* str);  // 0x2542E0
    // Appends one character through reserve; a full string is left
    // unchanged.
    FixedString& operator+=(char c);  // 0x254280
    // Empties the text, keeping the storage.
    void erase();  // 0x254FA0

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

// Heap string. Empty strings share a static zero-capacity buffer.
class String : public FixedString {
public:
    String();
    explicit String(const char* str);  // 0x2550C0
    // Copies the text. Its address in this build has not been located.
    String(const String& other);
    // Takes the other string's text and leaves it the shared empty buffer.
    String(String&& other);  // 0x255280
    ~String() override;                // slots 0-1: 0x255550, 0x255580

    using FixedString::operator=;

    // Slot 3 at 0x2553B0.
    void reserve(unsigned long capacity) override;
    // Reserves the length and terminates the text there. At 0x255AB0.
    void resize(unsigned long length);

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

// Writes the value in decimal with comma thousands separators into
// `buffer` and returns it. The map's signature is PrintIntWithCommas(char*,
// unsigned long, long); this build takes the value first.
char* PrintIntWithCommas(long value, char* buffer, unsigned long size);  // 0x2562F0

// Interned decimal text for small integers at 0x256410. Negative values map
// to "-1"; non-negative values index the table at 0x18EF6F0.
const char* IntToStaticString(int value);
