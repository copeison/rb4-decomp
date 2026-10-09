#pragma once

// Text view over caller-provided storage. The capacity word is stored in the
// four bytes before the text.
class FixedString {
public:
    virtual ~FixedString() {}

    // Slot 2 at 0x2542E0: appends through reserve, truncating to capacity.
    virtual FixedString& operator+=(const char* str);
    // Slot 3.
    virtual void reserve(unsigned long capacity) = 0;

    const char* c_str() const {
        return mStr;
    }

protected:
    unsigned int capacity() const {  // Name not in the reference map.
        return reinterpret_cast<const unsigned int*>(mStr)[-1];
    }

    char* mStr;
};

// Heap string. Empty strings share a static zero-capacity buffer.
class String : public FixedString {
public:
    String();
    explicit String(const char* str);  // 0x2550C0
    ~String() override;                // slots 0-1: 0x255550, 0x255580

    // Slot 2 is a thunk at 0x5F3C0 to the FixedString implementation.
    String& operator+=(const char* str) override;
    // Slot 3 at 0x2553B0.
    void reserve(unsigned long capacity) override;

private:
    void FreeText();  // Name not in the reference map.
};

static_assert(sizeof(String) == 16);

// Interned decimal text for small integers at 0x256410. Negative values map
// to "-1"; non-negative values index the table at 0x18EF6F0.
const char* IntToStaticString(int value);
