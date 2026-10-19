#pragma once

class BinStream;
class String;
class TextStream;

// A 128-bit identifier as four 32-bit chunks (os/HxGuid.o,
// 0x246960-0x247110). The binary places the object in the utl range. The
// engine's users carry one through UserGuid.
class HxGuid {
public:
    // All chunks zero, the null id.
    HxGuid();  // 0x246960

    void Clear();  // 0x246970
    // Hashes the current date and time, the shared generator's next value
    // and the cycle counter, and keeps the first 16 bytes of the digest.
    void Generate();  // 0x246980
    bool IsNull() const;  // 0x246A80
    int Chunk32(int index) const;  // 0x246AB0
    void SetChunk32(int index, int value);  // 0x246AC0
    // The 32 hexadecimal digits, from the MakeString buffers.
    const char* ToString() const;  // 0x246AD0
    // The four hexadecimal digits of the last chunk's low half.
    const char* ToShortString() const;  // 0x246B60
    // Parses 32 hexadecimal digits, eight per chunk. A character counts as
    // a letter only above 'A', so an 'A' itself reads as 17.
    HxGuid& operator=(String str);  // 0x246BD0
    bool operator==(const HxGuid& other) const;  // 0x246E00
    bool operator!=(const HxGuid& other) const;  // 0x246E30
    // Orders the chunks as signed values, the first chunk first.
    bool operator<(const HxGuid& other) const;  // 0x246E60
    const int* Data() const;  // 0x246EA0
    void SetData(int chunk0, int chunk1, int chunk2, int chunk3);  // 0x246EB0
    // The serialized size: the revision and the four chunks.
    static int SaveSize();  // 0x246EC0
    // A shared null id, built on first use (0x19E7C88).
    static const HxGuid& NULL_GUID();  // 0x246ED0

    // Field name not in the reference map.
    int mData[4];
};

static_assert(sizeof(HxGuid) == 16);

// Writes revision 1 and the four chunks.
BinStream& operator<<(BinStream& stream, const HxGuid& guid);  // 0x246F10
// Reads the revision, which it ignores, and the four chunks.
BinStream& operator>>(BinStream& stream, HxGuid& guid);  // 0x246FD0
// Prints ToString's text.
TextStream& operator<<(TextStream& stream, const HxGuid& guid);  // 0x247070
