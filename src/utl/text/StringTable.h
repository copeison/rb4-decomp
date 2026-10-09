#pragma once

#include "utl/containers/Vector.h"

// Packed storage for strings that live as long as the table
// (utl/StringTable.o). Strings are copied into the current buffer, each
// padded to the alignment; a full table adds a buffer as large as all the
// others.
class StringTable {
public:
    // One storage block. Field names are not in the reference map.
    struct Buf {
        unsigned long mSize;
        char* mStart;
        char* mFree;
    };

    // Starts with one buffer of `size` bytes when it is not zero.
    StringTable(const char* name, unsigned long size, unsigned long align);  // 0x25F860
    ~StringTable();                                                           // 0x25F9F0

    // Copies the string into the table and returns the copy.
    const char* Add(const char* str);  // 0x25FC50
    // Whether an equal string was added.
    bool Contains(const char* str) const;  // 0x25FE10
    // The bytes used in the buffers up to the current one.
    unsigned long UsedSize() const;  // 0x25FDC0

private:
    // Allocates a buffer of `size` bytes and makes it current.
    void _AddBuf(unsigned long size);  // 0x25F8D0

    // Field names are not in the reference map.
    unsigned long mAlign;
    eastl::vector<Buf> mBuffers;
    long mCurBuf;
    const char* mName;
};

static_assert(sizeof(StringTable) == 0x38);
