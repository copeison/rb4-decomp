#pragma once

#include <cstddef>

#include "utl/containers/Vector.h"
#include "utl/streams/BinStream.h"

// A stream over a growable memory buffer (utl/MemStream.o). Its vtable is at
// 0x18EF308. Only the declarations DynamicComBase needs are modelled; the
// methods are not reconstructed. Field names are not in the reference map.
class MemStream : public BinStream {
public:
    // Passes platform 3 to BinStream.
    explicit MemStream(bool littleEndian);  // 0x248550
    ~MemStream() override;                  // slots 0-1: 0x2489C0, 0x248A00

    void Flush() override;           // slot 2: 0x248A50, empty
    int Tell() override;             // slot 3: 0x248A60
    EofType Eof() override;          // slot 5: 0x248A70
    bool Fail() override;            // slot 6: 0x248A90
    long Size() const override;      // slot 8: 0x248AA0
    void ReadImpl(void* data, unsigned long size) override;         // slot 12: 0x2485B0
    void WriteImpl(const void* data, unsigned long size) override;  // slot 13: 0x248600
    void SeekImpl(long offset, SeekType origin) override;           // slot 14: 0x248920

    // Resizes the buffer. Name not in the reference map; the evidence is
    // weak.
    void Resize(unsigned long size);  // 0x248960

    // The read and write position.
    long mTell;
    eastl::vector<char> mBuffer;
};

static_assert(offsetof(MemStream, mTell) == 40);
static_assert(offsetof(MemStream, mBuffer) == 48);
static_assert(sizeof(MemStream) == 80);
