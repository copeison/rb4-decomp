#pragma once

#include <cstddef>
#include <vector>

#include "audio/core/formats/Chunks.h"
#include "utl/text/Str.h"

class BinStream;
class IListChunk;
class WaveFile;

// A cue point of a wave file and its label. Field names are not in the
// reference map.
struct WaveFileMarker {
    // Writes one 24-byte entry of the "cue " chunk. Inlined into
    // WaveFile::WriteCueChunk at 0xD7370; out of line at 0xD6240.
    void WriteCue(BinStream& stream);
    // Writes the "labl" subchunk. At 0xD6310.
    void WriteLabel(BinStream& stream);

    int mFrame;
    int mID;
    String mName;
};

static_assert(offsetof(WaveFileMarker, mName) == 8);
static_assert(sizeof(WaveFileMarker) == 24);

// RIFF wave file description (audio/WaveFile.o): the PCM format, the length
// in frames and the markers. A file read from a stream keeps its chunk
// reader so the samples can be copied out later. The object is 64 bytes.
// Field names are not in the reference map.
class WaveFile {
public:
    // Reads the format, the markers and the length.
    explicit WaveFile(BinStream& stream);  // 0xD63E0
    // Describes 16-bit or other integer PCM for writing.
    WaveFile(int sampleRate, int bitsPerSample, int numChannels);  // 0xD6C10
    ~WaveFile();                                                   // 0xD6C60

    void ReadFormat();      // 0xD6470
    // Pairs the "cue " points, sorted by frame, with the "labl" texts of
    // the "adtl" list.
    void ReadMarkers();     // 0xD6540
    void ReadNumSamples();  // 0xD6BD0

    // Sizes of the marker chunks and of the whole RIFF body.
    int CueChunkSize() const;  // Inlined into its callers.
    int LablSize() const;      // 0xD6DF0
    int RiffSize() const;      // 0xD6E70

    // Writes the whole file to a new file at the path.
    void Write(const char* path);  // 0xD6F30
    void WriteRiffHeader(BinStream& stream);   // 0xD6FD0
    void WriteFormatChunk(BinStream& stream);  // 0xD7100
    // Copies the samples from the source file's data chunk.
    void WriteDataChunk(BinStream& stream);  // 0xD7200
    void WriteCueChunk(BinStream& stream);   // 0xD7370
    void WriteLablChunk(BinStream& stream);  // 0xD74F0

    // Writes the RIFF, format and data headers.
    void WriteFileHeader(BinStream& stream);  // 0xD7630
    // Rewrites the headers once the sample data is complete. The RIFF size
    // written here leaves out the marker chunks.
    void PatchDataSize(BinStream& stream, int numFrames);  // 0xD76B0
    // Positions the chunk reader on the data chunk. At 0xD77B0; the map
    // gives no return type.
    IListChunk& PrepareToProvideData();

    // The size of the sample data in bytes. Name not in the reference map.
    int DataSize() const {
        return mNumSamples * mNumChannels * mBitsPerSample / 8;
    }

    unsigned short mFormat;
    unsigned short mNumChannels;
    int mSampleRate;
    int mAvgBytesPerSec;
    unsigned short mBlockAlign;
    unsigned short mBitsPerSample;
    int mNumSamples;
    std::vector<WaveFileMarker> mMarkers;
    IListChunk* mListChunk;
};

static_assert(offsetof(WaveFile, mNumSamples) == 16);
static_assert(offsetof(WaveFile, mMarkers) == 24);
static_assert(offsetof(WaveFile, mListChunk) == 56);
static_assert(sizeof(WaveFile) == 64);

// Streams a wave file's samples from its data chunk. The vtable is at
// 0x18E60C8; the object is 88 bytes.
class WaveFileData : public IDataChunk {
public:
    explicit WaveFileData(WaveFile& wave);  // 0xD77E0
    ~WaveFileData() override;               // slots 0-1: 0xD7830, 0xD7840

    WaveFile* mWave;  // Name not in the reference map.
};

static_assert(sizeof(WaveFileData) == 88);

// The canonical 44-byte header of a PCM wave file. Field names are not in
// the reference map.
struct WaveHeader {
    WaveHeader();  // 0xD7860

    char mRiffTag[4];
    int mRiffSize;
    char mWaveTag[4];
    char mFormatTag[4];
    int mFormatSize;
    unsigned short mFormat;
    unsigned short mNumChannels;
    int mSampleRate;
    int mAvgBytesPerSec;
    unsigned short mBlockAlign;
    unsigned short mBitsPerSample;
    char mDataTag[4];
    int mDataSize;
};

static_assert(offsetof(WaveHeader, mDataTag) == 36);
static_assert(sizeof(WaveHeader) == 44);

// Writes a 16-bit wave file to a stream as the samples arrive and patches the
// header when destroyed. Field names are not in the reference map.
class WaveFileWriter {
public:
    // The byte rate assumes one channel.
    WaveFileWriter(BinStream& stream, int sampleRate, int numChannels);  // 0xD7890
    ~WaveFileWriter();  // 0xD78F0

    // Returns whether the stream failed. The map gives no return type.
    bool Write(unsigned char* data, int size);  // 0xD7920

    WaveHeader mHeader;
    BinStream* mStream;
};

static_assert(offsetof(WaveFileWriter, mStream) == 48);
