#include "audio/core/formats/Chunks.h"

#include "utl/text/TextStream.h"

namespace {

// Whether two tags match. Name not in the reference map.
bool SameTag(const ChunkID& a, const ChunkID& b) {
    return std::strncmp(a.mStr, b.mStr, sizeof(a.mStr)) == 0;
}

// The BinStream platform chunks report. Name not in the reference map.
constexpr int kChunkStreamPlatform = 3;

}  // namespace

// Reconstructed from eboot.elf at 0xE2110.
void ChunkHeader::Read(BinStream& stream) {
    stream.Read(&mID, sizeof(mID));
    stream.ReadEndian(&mLength, sizeof(mLength));
    if (SameTag(mID, kListChunkID) || SameTag(mID, kRiffChunkID)) {
        stream.Read(&mID, sizeof(mID));
        mIsList = true;
        mLength -= 4;
    } else {
        mIsList = false;
    }
}

// Reconstructed from eboot.elf at 0xE21A0. Prints the tag and the length,
// such as "LIST:adtl<52>".
void ChunkHeader::Print(TextStream& stream) const {
    TextStream& out = mIsList ? stream << "LIST:" : stream;
    out << mID.mStr[0] << mID.mStr[1] << mID.mStr[2] << mID.mStr[3];
    out << "<" << mLength << ">";
}

// Reconstructed from eboot.elf at 0xE2230.
IDataChunk::IDataChunk(IListChunk& parent)
    : BinStream(false, kChunkStreamPlatform),
      mParent(&parent),
      mBaseStream(parent.mStream),
      mHeader(nullptr),
      mFailed(false),
      mEof(false) {
    mHeader = new ChunkHeader(*parent.CurSubChunkHeader());
    mStartMarker = mBaseStream->Tell();
    mEndMarker = mHeader->mLength + mStartMarker;
    parent.Lock();
}

// Reconstructed from eboot.elf at 0xE22D0.
const ChunkHeader* IListChunk::CurSubChunkHeader() const {
    return mSubHeaderValid ? &mSubHeader : nullptr;
}

// Reconstructed from eboot.elf at 0xE22E0.
void IListChunk::Lock() {
    mLocked = true;
}

// Reconstructed from eboot.elf at 0xE22F0.
IDataChunk::IDataChunk(BinStream& stream)
    : BinStream(false, kChunkStreamPlatform),
      mParent(nullptr),
      mBaseStream(&stream),
      mHeader(nullptr),
      mFailed(false),
      mEof(false) {
    ChunkHeader* header = new ChunkHeader();
    header->Read(stream);
    mHeader = header;
    mStartMarker = mBaseStream->Tell();
    mEndMarker = mStartMarker + mHeader->mLength;
}

// Reconstructed from eboot.elf at 0xE23F0 (0xE2440 deletes).
IDataChunk::~IDataChunk() {
    if (mParent != nullptr) {
        mParent->UnLock();
    }
    delete mHeader;
}

// Reconstructed from eboot.elf at 0xE2430.
void IListChunk::UnLock() {
    mLocked = false;
}

// Reconstructed from eboot.elf at 0xE2490. Offsets from the end are
// negative.
void IDataChunk::SeekImpl(long offset, SeekType origin) {
    if (Fail()) {
        return;
    }
    switch (origin) {
    case kSeekBegin:
        if (mHeader->mLength < offset) {
            mFailed = true;
        }
        mBaseStream->Seek(mStartMarker + offset, kSeekBegin);
        break;
    case kSeekCur:
        mBaseStream->Seek(offset, kSeekCur);
        break;
    case kSeekEnd:
        if (-static_cast<long>(mHeader->mLength) > offset) {
            mFailed = true;
        }
        mBaseStream->Seek(mEndMarker + offset, kSeekBegin);
        break;
    default:
        break;
    }
    mEof = mBaseStream->Eof() != NotEof;
}

// Reconstructed from eboot.elf at 0xE2530.
int IDataChunk::Tell() {
    if (Fail()) {
        return -1;
    }
    return mBaseStream->Tell() - mStartMarker;
}

// Reconstructed from eboot.elf at 0xE2570.
void IDataChunk::ReadImpl(void* data, unsigned long size) {
    long remaining = mEndMarker - static_cast<long>(mBaseStream->Tell());
    if (static_cast<int>(size) < remaining) {
        mBaseStream->Read(data, size);
        return;
    }
    mBaseStream->Read(data, remaining);
    mEof = true;
}

// Reconstructed from eboot.elf at 0xE25D0. A list read from the stream takes
// its header there; otherwise it spans the stream from its position to the
// end.
IListChunk::IListChunk(BinStream& stream, bool readHeader)
    : mParent(nullptr),
      mStream(&stream),
      mHeader(nullptr),
      mLocked(false),
      mSubHeader(),
      mSubHeaderValid(false),
      mRecentlyReset(true) {
    ChunkHeader* header;
    if (readHeader) {
        header = new ChunkHeader();
        header->Read(stream);
    } else {
        int start = stream.Tell();
        stream.Seek(0, kSeekEnd);
        int end = stream.Tell();
        stream.Seek(start, kSeekBegin);
        header = new ChunkHeader(kListChunkID, end - start, true);
    }
    mHeader = header;
    mStartMarker = mStream->Tell();
    Init();
}

// Reconstructed from eboot.elf at 0xE2760.
void IListChunk::Init() {
    mEndMarker = mHeader->mLength + mStartMarker;
    if (mParent != nullptr) {
        mParent->Lock();
    }
    Reset();
}

// Reconstructed from eboot.elf at 0xE27B0.
IListChunk::IListChunk(IListChunk& parent)
    : mParent(&parent),
      mStream(parent.mStream),
      mHeader(nullptr),
      mLocked(false),
      mSubHeader(),
      mSubHeaderValid(false),
      mRecentlyReset(true) {
    mHeader = new ChunkHeader(*parent.CurSubChunkHeader());
    mStartMarker = mStream->Tell();
    Init();
}

// Reconstructed from eboot.elf at 0xE2860.
IListChunk::~IListChunk() {
    if (mParent != nullptr) {
        mParent->UnLock();
    }
    delete mHeader;
}

// Reconstructed from eboot.elf at 0xE2880.
void IListChunk::Reset() {
    mStream->Seek(mStartMarker, kSeekBegin);
    mSubChunkMarker = mStartMarker;
    mSubHeaderValid = false;
    mRecentlyReset = true;
}

// Reconstructed from eboot.elf at 0xE28B0.
const ChunkHeader* IListChunk::Next() {
    mRecentlyReset = false;
    if (mSubChunkMarker >= mEndMarker) {
        mSubHeaderValid = false;
        return nullptr;
    }
    mSubHeaderValid = true;
    mStream->Seek(mSubChunkMarker, kSeekBegin);
    mSubHeader.Read(*mStream);
    int size = mSubHeader.mLength + (mSubHeader.mIsList ? 12 : 8);
    ChunkID id = mSubHeader.mID;
    if (!SameTag(id, kMidiTrackChunkID)) {
        size += size % 2;
    }
    mSubChunkMarker += size;
    return &mSubHeader;
}

// Reconstructed from eboot.elf at 0xE29E0.
const ChunkHeader* IListChunk::Next(ChunkID id) {
    while (Next() != nullptr) {
        ChunkID found = mSubHeader.mID;
        if (SameTag(id, found)) {
            return &mSubHeader;
        }
    }
    return nullptr;
}
