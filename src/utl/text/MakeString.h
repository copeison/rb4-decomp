#pragma once

class Symbol;

// Formats into the per-thread MakeString ring buffer; the result stays valid
// after the formatter is destroyed. Not reconstructed yet; the declarations
// follow the map's utl/MakeString.o.
class FormatString {
public:
    explicit FormatString(const char* fmt);  // 0x2472C0
    ~FormatString();                         // 0x247510

    FormatString& operator<<(void* value);          // 0x247670
    // 0x247890 takes a 64-bit integer.
    FormatString& operator<<(int value);            // 0x247CD0
    FormatString& operator<<(float value);          // 0x2480A0
    FormatString& operator<<(const Symbol& value);  // 0x2483D0
    const char* Str();                       // 0x2484E0

private:
    // Field names are not in the reference map.
    char* mFmt;              // The next format specifier.
    int mNextType;           // Starts at 3.
    char* mBuf;              // From MakeStringBuf.
    int mBufRemaining;       // Starts at 4096.
    char* mSavedFmtEnd;
    void* mUnknown40;
};

static_assert(sizeof(FormatString) == 48);

// Formats one value. Inlined into every caller. Name not in the reference
// map.
template <typename T>
inline const char* MakeString(const char* fmt, T value) {
    FormatString format(fmt);
    format << value;
    return format.Str();
}
