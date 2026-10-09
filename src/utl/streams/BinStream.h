#pragma once

#include <cstddef>
#include <cstdint>

#include "os/memory/MemMgr.h"

class Rand2;
class Symbol;

enum SeekType {
    kSeekBegin = 0,
    kSeekCur = 1,
    kSeekEnd = 2,
};

enum EofType {
    NotEof = 0,
    IsEof = 1,
};

// Common 40-byte binary stream base. Its vtable at 0x18EF010 leaves the
// stream-specific operations pure.
class BinStream {
public:
    BinStream(bool littleEndian, int platform);  // 0x21A680
    virtual ~BinStream();                        // slots 0-1: 0x21A6B0, 0x21A6D0

    virtual void Flush() = 0;  // slot 2
    virtual int Tell() = 0;    // slot 3
    // Slot 4 at 0xD7960 is empty in every recovered stream. Name not in the
    // reference map.
    virtual void Reserved4() {}
    virtual EofType Eof() = 0;  // slot 5
    virtual bool Fail() = 0;    // slot 6
    // Slot 7 at 0x219CD0.
    virtual const char* Name() const;
    // Slot 8 at 0xD7990.
    virtual long Size() const {
        return 0;
    }
    // Slot 9 at 0x21AB60. Returns the requested size, or zero once the
    // stream has failed.
    virtual unsigned long ReadAsync(void* data, unsigned long size);
    // Slot 10 at 0xD79A0.
    virtual bool Cached() const {
        return false;
    }
    // Slot 11 at 0x219C30. Rewrites the 64-bit size field at the given
    // position with the number of bytes that follow it. The map has
    // WriteSkipMark(unsigned long, bool); name not in the reference map.
    virtual void PatchSize(long position);
    virtual void ReadImpl(void* data, unsigned long size) = 0;         // slot 12
    virtual void WriteImpl(const void* data, unsigned long size) = 0;  // slot 13
    virtual void SeekImpl(long offset, SeekType origin) = 0;           // slot 14

    DELETE_OVERLOAD

    void Read(void* data, unsigned long size);
    void ReadEndian(void* data, int size);
    void WriteEndian(const void* data, int size);
    void Write(const void* data, unsigned long size);  // 0x219DF0
    void Seek(long offset, SeekType origin);
    BinStream& operator>>(Symbol& symbol);

    // Data members are public so the layout asserts below can reach them.
    // +8 and +12 are set to -1 and +16 to false at construction; no
    // recovered code reads them. Names not in the reference map.
    int mUnknown8;
    int mUnknown12;
    bool mUnknown16;
    // Nonzero when values must be byte-swapped. The map names the constructor
    // parameter littleEndian.
    int mLittleEndian;
    Rand2* mCrypto;  // Owned.
    int mPlatform;

private:
    void Decrypt(void* data, unsigned long size);
    void WriteEncrypted(const void* data, unsigned long size);
};

static_assert(offsetof(BinStream, mLittleEndian) == 20);
static_assert(offsetof(BinStream, mCrypto) == 24);
static_assert(offsetof(BinStream, mPlatform) == 32);
static_assert(sizeof(BinStream) == 40);
