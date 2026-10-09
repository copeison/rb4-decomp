#pragma once

class DataNode;
class FixedString;
class Symbol;

// Formats into the per-thread MakeString ring buffer (utl/MakeString.o);
// the result stays valid after the formatter is destroyed. Each operator<<
// formats one argument with the format text up to the next specifier and
// appends the result.
class FormatString {
public:
    // The argument type of the pending specifier. Names not in the
    // reference map.
    enum Type {
        kFormatInt = 0,    // Any other conversion, such as d, x, c or p.
        kFormatStr = 1,    // s.
        kFormatFloat = 2,  // a, f or g.
        kFormatNone = 3,   // No specifier is left.
    };

    FormatString();                                             // 0x247290
    explicit FormatString(const char* fmt);                     // 0x2472C0
    // Copying is a no-op in this build.
    FormatString(const FormatString& other);                    // 0x2474F0
    FormatString& operator=(const FormatString& other);         // 0x247500
    ~FormatString();                                            // 0x247510

    // Copies the format into the calling thread's next format slot and,
    // when `updateType` is set, finds its first specifier. The binary does
    // not read `buf`; the constructor leaves its register unset.
    void InitializeWithFmt(const char* fmt, char* buf, bool updateType);  // 0x247310

    FormatString& operator<<(void* value);               // 0x247670
    FormatString& operator<<(unsigned int value);        // 0x247780
    FormatString& operator<<(unsigned long value);       // 0x247890
    FormatString& operator<<(long value);                // 0x2479A0
    // The two 64-bit overloads at 0x247AB0 and 0x247BC0 have no callers and
    // are not in the reference map; their signedness is inferred from their
    // place after the long overload.
    FormatString& operator<<(unsigned long long value);  // 0x247AB0
    FormatString& operator<<(long long value);           // 0x247BC0
    FormatString& operator<<(int value);                 // 0x247CD0
    // Converts the node to the pending specifier's type.
    FormatString& operator<<(const DataNode& value);     // 0x247DE0
    FormatString& operator<<(const char* value);         // 0x247F90
    FormatString& operator<<(float value);               // 0x2480A0
    FormatString& operator<<(double value);              // 0x2481B0
    FormatString& operator<<(const FixedString& value);  // 0x2482C0
    FormatString& operator<<(const Symbol& value);       // 0x2483D0
    // Appends the format text after the last specifier and returns the
    // buffer.
    const char* Str();  // 0x2484E0

private:
    // Moves to the format text after the last formatted specifier and finds
    // the next specifier and its type. Inlined into every operator<<.
    void _UpdateType();  // 0x2475C0

    // Formats one argument into the buffer with the format text up to the
    // next specifier. Name not in the reference map; the binary repeats
    // this body in every operator<<.
    template <typename T>
    FormatString& Format(T value);

    // Field names are not in the reference map.
    char* mFmt;      // The format text still to format.
    Type mType;      // The pending specifier's type.
    char* mBuf;      // From MakeStringBuf.
    int mBufSize;    // The space left in mBuf; starts at 4096.
    char* mNextFmt;  // The next specifier after mFmt's, or null.
    char* mFmtBuf;   // The format's copy in the thread's format slot.
};

static_assert(sizeof(FormatString) == 48);

// The calling thread's next 4096-byte MakeString buffer, emptied. The
// buffers are reused round-robin.
char* MakeStringBuf();  // 0x2471A0

// Formats one value. Inlined into every caller; the map's instantiations,
// such as MakeString<int>(char const*, int const&), take the value by
// reference.
template <typename T>
inline const char* MakeString(const char* fmt, T value) {
    FormatString format(fmt);
    format << value;
    return format.Str();
}
