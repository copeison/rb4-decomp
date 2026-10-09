#pragma once

#include <cstddef>

#include "os/files/File.h"
#include "utl/streams/BinStream.h"

class StreamChecksum;

// 592-byte binary stream over an engine file. Its vtable is at 0x18EF280. The
// optional checksum is attached by its owner and updated with every
// successful read.
class FileStream : public BinStream {
public:
    // Reconstructed from eboot.elf at 0x2443A0. The map's third parameter is
    // a bool; this build passes the byte-swap flag there.
    FileStream(const char* path, FileMode mode, bool littleEndian);
    ~FileStream() override;  // slots 0-1: 0x2444A0, 0x244580

    void Flush() override;              // 0x2446A0
    int Tell() override;                // 0x2446E0
    EofType Eof() override;             // 0x2446F0
    bool Fail() override;               // 0x244710
    const char* Name() const override;  // 0x244800
    long Size() const override;         // 0x244720
    void ReadImpl(void* data, unsigned long size) override;         // 0x244610
    void WriteImpl(const void* data, unsigned long size) override;  // 0x244670
    void SeekImpl(long offset, SeekType origin) override;           // 0x2446B0

    void DeleteChecksum();

    void* mFile;
    char mFilename[512];
    bool mFail;
    long mSize;
    StreamChecksum* mChecksum;
    long mBytesChecksummed;
};

static_assert(offsetof(FileStream, mFile) == 40);
static_assert(offsetof(FileStream, mFilename) == 48);
static_assert(offsetof(FileStream, mFail) == 560);
static_assert(offsetof(FileStream, mSize) == 568);
static_assert(offsetof(FileStream, mChecksum) == 576);
static_assert(sizeof(FileStream) == 592);
