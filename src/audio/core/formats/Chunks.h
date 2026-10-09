#pragma once

#include <cstddef>
#include <cstring>

#include "utl/streams/BinStream.h"

class TextStream;

// RIFF chunk readers (audio/Chunks.o and audio/ChunkIDs.o).

// A four-character chunk tag.
class ChunkID {
public:
    // Inlined into the static initializer at 0xE2040. Name not in the
    // reference map.
    explicit ChunkID(const char* tag) {
        std::memcpy(mStr, tag, sizeof(mStr));
    }

    char mStr[4];  // Name not in the reference map.
};

// Chunk tags set by the static initializer at 0xE2040 (ChunkIDs.cpp). Names
// not in the reference map.
extern ChunkID kListChunkID;            // 0x19E26C8 "LIST"
extern ChunkID kRiffChunkID;            // 0x19E26CC "RIFF"
extern ChunkID kMidiChunkID;            // 0x19E26D0 "MIDI"
extern ChunkID kMidiHeaderChunkID;      // 0x19E26D4 "MThd"
extern ChunkID kMidiTrackChunkID;       // 0x19E26D8 "MTrk"
extern ChunkID kWaveChunkID;            // 0x19E26DC "WAVE"
extern ChunkID kWaveFormatChunkID;      // 0x19E26E0 "fmt "
extern ChunkID kWaveDataChunkID;        // 0x19E26E4 "data"
extern ChunkID kWaveFactChunkID;        // 0x19E26E8 "fact"
extern ChunkID kWaveInstChunkID;        // 0x19E26EC "inst"
extern ChunkID kWaveSampleChunkID;      // 0x19E26F0 "smpl"
extern ChunkID kWaveCueChunkID;         // 0x19E26F4 "cue "
extern ChunkID kWaveLabelChunkID;       // 0x19E26F8 "labl"
extern ChunkID kWaveTextChunkID;        // 0x19E26FC "ltxt"
extern ChunkID kWaveAdditionalChunkID;  // 0x19E2700 "adtl"

// The tag and the length of a chunk. A RIFF or LIST chunk's header also
// takes its form tag, which the length then excludes. Field names are not in
// the reference map.
class ChunkHeader {
public:
    // Inlined wherever a header is created. Name not in the reference map.
    ChunkHeader() : mID("????"), mLength(0), mIsList(false) {}
    ChunkHeader(const ChunkID& id, int length, bool isList) : mID(id), mLength(length), mIsList(isList) {}

    void Read(BinStream& stream);             // 0xE2110
    void Print(TextStream& stream) const;     // 0xE21A0

    ChunkID mID;
    int mLength;
    bool mIsList;
};

static_assert(sizeof(ChunkHeader) == 12);

// Walks the subchunks of a RIFF or LIST chunk. A nested list locks its parent
// while it exists. The object is 56 bytes. Field names are not in the
// reference map.
class IListChunk {
public:
    // Reads the list's header from the stream, or without readHeader treats
    // the rest of the stream as one list.
    IListChunk(BinStream& stream, bool readHeader);  // 0xE25D0
    // Opens the parent's current subchunk as a nested list.
    explicit IListChunk(IListChunk& parent);  // 0xE27B0
    ~IListChunk();                            // 0xE2860

    // Sets the end from the header, locks the parent and rewinds. At
    // 0xE2760; inlined into the constructors.
    void Init();
    void Lock();    // 0xE22E0
    void UnLock();  // 0xE2430
    // Rewinds to the first subchunk.
    void Reset();  // 0xE2880
    // Advances to the next subchunk; null at the end. Subchunks are padded to
    // an even length except MIDI tracks.
    const ChunkHeader* Next();  // 0xE28B0
    // Advances to the next subchunk with the tag; null when none is left.
    const ChunkHeader* Next(ChunkID id);  // 0xE29E0
    const ChunkHeader* CurSubChunkHeader() const;  // 0xE22D0

    IListChunk* mParent;
    BinStream* mStream;
    ChunkHeader* mHeader;
    int mStartMarker;
    int mEndMarker;
    bool mLocked;
    ChunkHeader mSubHeader;
    bool mSubHeaderValid;
    bool mRecentlyReset;
    int mSubChunkMarker;  // Stream position of the next subchunk.
};

static_assert(offsetof(IListChunk, mLocked) == 32);
static_assert(offsetof(IListChunk, mSubHeader) == 36);
static_assert(offsetof(IListChunk, mSubHeaderValid) == 48);
static_assert(offsetof(IListChunk, mSubChunkMarker) == 52);
static_assert(sizeof(IListChunk) == 56);

// Reads the current subchunk of an IListChunk, or a chunk at a stream's
// position, as a stream of its own. The vtable is at 0x18E6490; the object
// is 80 bytes. Field names are not in the reference map.
class IDataChunk : public BinStream {
public:
    explicit IDataChunk(IListChunk& parent);  // 0xE2230
    explicit IDataChunk(BinStream& stream);   // 0xE22F0
    ~IDataChunk() override;                   // slots 0-1: 0xE23F0, 0xE2440

    // Slot 2 at 0xD7950.
    void Flush() override {}
    // Slot 3 at 0xE2530: the position within the chunk, or -1 after a
    // failure.
    int Tell() override;
    // Slot 5 at 0xD7970.
    EofType Eof() override {
        return static_cast<EofType>(mEof);
    }
    // Slot 6 at 0xD7980.
    bool Fail() override {
        return mFailed;
    }
    // Slot 12 at 0xE2570: a read past the chunk's end is cut short and sets
    // the end flag.
    void ReadImpl(void* data, unsigned long size) override;
    // Slot 13 at 0xD79B0: chunks are read-only.
    void WriteImpl(const void* data, unsigned long size) override {
        static_cast<void>(data);
        static_cast<void>(size);
    }
    // Slot 14 at 0xE2490: seeking outside the chunk fails the stream.
    void SeekImpl(long offset, SeekType origin) override;

    IListChunk* mParent;
    BinStream* mBaseStream;
    ChunkHeader* mHeader;  // Copy of the chunk's header.
    int mStartMarker;
    int mEndMarker;
    bool mFailed;
    bool mEof;
};

static_assert(offsetof(IDataChunk, mParent) == 40);
static_assert(offsetof(IDataChunk, mHeader) == 56);
static_assert(offsetof(IDataChunk, mFailed) == 72);
static_assert(sizeof(IDataChunk) == 80);
