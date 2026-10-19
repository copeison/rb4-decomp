#pragma once

#include <cstddef>

#include "utl/streams/BinStream.h"

// A versioned block inside another stream (utl/RevisionStream.o): BeginWrite
// writes a 'z' byte and the revision, and the destructor closes the block
// with a '{' byte; BeginRead checks the marker and that the stored revision
// lies between the minimum and the current revision. Reads and writes pass
// through to the wrapped stream. The vtable is at 0x19A5108.
// Only the members the persistence module uses are declared; the class is
// not reconstructed.
class RevisionStream : public BinStream {
public:
    // Why BeginRead failed (mError). Names not in the reference map.
    enum ReadError {
        kReadOk = 0,
        // The stored revision is older than the minimum.
        kReadTooOld = -1,
        // The stored revision is newer than the current one.
        kReadTooNew = -2,
        // The wrapped stream failed reading the header.
        kReadFailed = -3,
        // The block does not start with the 'z' marker.
        kReadNoHeader = -4,
    };

    // Wraps `base`, writing `revision` and accepting stored revisions from
    // `minRevision` to `revision`. The map has the same signature.
    RevisionStream(BinStream& base, int revision, int minRevision);  // 0x1183580
    // Finishes the block: writes the closing '{' after a write, and after a
    // successful read checks it, recording kReadNoHeader when it is
    // missing.
    ~RevisionStream() override;  // 0x11835F0

    void Flush() override;
    int Tell() override;
    EofType Eof() override;
    bool Fail() override;
    const char* Name() const override;
    void ReadImpl(void* data, unsigned long size) override;
    void WriteImpl(const void* data, unsigned long size) override;
    void SeekImpl(long offset, SeekType origin) override;

    // Reads the marker and the stored revision. False, with mError set,
    // when the block cannot be read.
    bool BeginRead();  // 0x1183820
    // Writes the marker and the current revision.
    void BeginWrite();  // 0x11838D0

    // Field names are not in the reference map.
    BinStream* mBase;
    int mRevision;
    int mMinRevision;
    // The revision BeginRead found; -1 until then. The Revision operations
    // compare against it.
    int mReadRevision;
    int mError;  // A ReadError.
    // Skips the closing check after a read. Set by FinishRead in the map's
    // build; nothing in the persistence module sets it.
    bool mSkipFinish;
    // 1 after a read began, 2 after a write began.
    int mMode;
    bool mFinished;
};

static_assert(offsetof(RevisionStream, mBase) == 40);
static_assert(offsetof(RevisionStream, mReadRevision) == 56);
static_assert(offsetof(RevisionStream, mError) == 60);
static_assert(offsetof(RevisionStream, mMode) == 68);
static_assert(sizeof(RevisionStream) == 80);

namespace Revision {

// A read that depends on the block's stored revision. Name in the reference
// map; its slots are the destructors and Read.
class RevisionOp {
public:
    virtual ~RevisionOp() {}
    virtual void Read(RevisionStream& stream) = 0;
};

// Reads the value only when the block was written at revision `asOf` or
// later, leaving it unchanged otherwise. The instantiations are emitted in
// the game code: AsOfOp<float> (vtable 0x1963408, Read 0x9D4430) and
// AsOfOp<bool> (vtable 0x1963430, Read 0x9D4750).
template <typename T>
class AsOfOp : public RevisionOp {
public:
    AsOfOp(T* value, int asOf) : mValue(value), mAsOf(asOf) {}
    ~AsOfOp() override {}

    void Read(RevisionStream& stream) override {
        if (stream.mReadRevision >= mAsOf) {
            stream >> *mValue;
        }
    }

    // Field names are not in the reference map.
    T* mValue;
    int mAsOf;
};

// Reads the value only when the block was written at a revision from
// `asOf` to `until`, for a value later revisions dropped. Name and field
// names not in the reference map. The game code emits RangeOp<int> (vtable
// 0x19634D0, Read 0x9D4C00).
template <typename T>
class RangeOp : public RevisionOp {
public:
    RangeOp(T* value, int asOf, int until) : mValue(value), mAsOf(asOf), mUntil(until) {}
    ~RangeOp() override {}

    void Read(RevisionStream& stream) override {
        if (stream.mReadRevision >= mAsOf && stream.mReadRevision <= mUntil) {
            stream >> *mValue;
        }
    }

    T* mValue;
    int mAsOf;
    int mUntil;
};

// Runs the operation on the stream. The map also has an lvalue overload.
RevisionStream& operator>>(RevisionStream& stream, RevisionOp&& op);  // 0x1183960

}  // namespace Revision
