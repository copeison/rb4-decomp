#include "audio/core/formats/WaveFile.h"

#include <algorithm>
#include <cstring>

#include "utl/containers/Vector.h"
#include "utl/streams/FileStream.h"

namespace {

// FileStream mode Write opens its file with. Name not in the reference map.
constexpr auto kWaveWriteFileMode = static_cast<FileMode>(4);
// PCM format tag and the canonical format chunk size.
constexpr unsigned short kWaveFormatPcm = 1;
constexpr int kWaveFormatChunkSize = 16;
// Bytes of the RIFF body besides the samples and the marker chunks: the
// WAVE tag, the format chunk and the data, cue and LIST chunk headers.
constexpr int kRiffOverhead = 52;
// Copy size of WriteDataChunk. Name not in the reference map.
constexpr int kDataCopySize = 0x10000;

// A cue point as ReadMarkers collects it, sorted by frame. Name not in the
// reference map.
struct CuePoint {
    int mFrame;
    int mID;

    bool operator<(const CuePoint& other) const {
        return mFrame < other.mFrame;
    }
};

// A label read from the "adtl" list. Name not in the reference map.
struct CueLabel {
    String mText;
    int mID;
};

// Label text rounded up to an even length, as RIFF pads chunks.
int PaddedTextSize(const String& text) {
    int size = static_cast<int>(std::strlen(text.c_str())) + 1;
    return (size & 1) != 0 ? size + 1 : size;
}

}  // namespace

// Reconstructed from eboot.elf at 0xD6240.
void WaveFileMarker::WriteCue(BinStream& stream) {
    int value = mID;
    stream.WriteEndian(&value, sizeof(value));
    value = mFrame;
    stream.WriteEndian(&value, sizeof(value));
    stream.Write(&kWaveDataChunkID, sizeof(kWaveDataChunkID));
    value = 0;
    stream.WriteEndian(&value, sizeof(value));
    value = 0;
    stream.WriteEndian(&value, sizeof(value));
    value = mFrame;
    stream.WriteEndian(&value, sizeof(value));
}

// Reconstructed from eboot.elf at 0xD6310. The text is padded to an even
// length.
void WaveFileMarker::WriteLabel(BinStream& stream) {
    int length = static_cast<int>(std::strlen(mName.c_str()));
    stream.Write(&kWaveLabelChunkID, sizeof(kWaveLabelChunkID));
    int value = length + 5;
    stream.WriteEndian(&value, sizeof(value));
    value = mID;
    stream.WriteEndian(&value, sizeof(value));
    stream.Write(mName.c_str(), length + 1);
    if (((length + 1) & 1) != 0) {
        char pad = 0;
        stream.Write(&pad, 1);
    }
}

// Reconstructed from eboot.elf at 0xD63E0.
WaveFile::WaveFile(BinStream& stream) : mMarkers() {
    mListChunk = new IListChunk(stream, true);
    ReadFormat();
    ReadMarkers();
    ReadNumSamples();
}

// Reconstructed from eboot.elf at 0xD6470.
void WaveFile::ReadFormat() {
    mListChunk->Reset();
    mListChunk->Next(kWaveFormatChunkID);
    IDataChunk chunk(*mListChunk);
    chunk.ReadEndian(&mFormat, sizeof(mFormat));
    chunk.ReadEndian(&mNumChannels, sizeof(mNumChannels));
    chunk.ReadEndian(&mSampleRate, sizeof(mSampleRate));
    chunk.ReadEndian(&mAvgBytesPerSec, sizeof(mAvgBytesPerSec));
    chunk.ReadEndian(&mBlockAlign, sizeof(mBlockAlign));
    chunk.ReadEndian(&mBitsPerSample, sizeof(mBitsPerSample));
}

// Reconstructed from eboot.elf at 0xD6540. A cue point without a label gets
// an empty name. The binary sorts with EASTL's sort; std::sort stands in.
void WaveFile::ReadMarkers() {
    eastl::vector<CuePoint> cues;
    eastl::vector<CueLabel> labels;
    mListChunk->Reset();
    if (mListChunk->Next(kWaveCueChunkID) == nullptr) {
        return;
    }

    int numCues = 0;
    {
        IDataChunk chunk(*mListChunk);
        chunk.ReadEndian(&numCues, sizeof(numCues));
        for (int i = 0; i < numCues; ++i) {
            CuePoint cue;
            chunk.ReadEndian(&cue.mID, sizeof(cue.mID));
            chunk.ReadEndian(&cue.mFrame, sizeof(cue.mFrame));
            cues.push_back(cue);
            // Skips the chunk tag and the three offsets.
            chunk.Seek(16, kSeekCur);
        }
    }
    if (numCues == 0) {
        return;
    }

    mListChunk->Reset();
    if (mListChunk->Next(kWaveAdditionalChunkID) != nullptr) {
        IListChunk list(*mListChunk);
        for (int i = 0; i < numCues; ++i) {
            list.Next(kWaveLabelChunkID);
            IDataChunk chunk(list);
            long length = chunk.mHeader->mLength - 4L;
            int id;
            chunk.ReadEndian(&id, sizeof(id));
            String text;
            text.resize(length);
            chunk.Read(const_cast<char*>(text.c_str()), length);
            labels.push_back(CueLabel{text, id});
        }
    }

    std::sort(cues.begin(), cues.end());
    for (int i = 0; i < numCues; ++i) {
        String name;
        for (int j = 0; j < static_cast<int>(labels.size()); ++j) {
            if (labels[j].mID == cues[i].mID) {
                name = labels[j].mText.c_str();
                break;
            }
        }
        mMarkers.push_back(WaveFileMarker{cues[i].mFrame, cues[i].mID, name});
    }
}

// Reconstructed from eboot.elf at 0xD6BD0.
void WaveFile::ReadNumSamples() {
    mListChunk->Reset();
    mNumSamples = mListChunk->Next(kWaveDataChunkID)->mLength / mBlockAlign;
}

// Reconstructed from eboot.elf at 0xD6C10.
WaveFile::WaveFile(int sampleRate, int bitsPerSample, int numChannels)
    : mFormat(kWaveFormatPcm),
      mNumChannels(static_cast<unsigned short>(numChannels)),
      mSampleRate(sampleRate),
      mBitsPerSample(static_cast<unsigned short>(bitsPerSample)),
      mNumSamples(0),
      mMarkers(),
      mListChunk(nullptr) {
    mBlockAlign = static_cast<unsigned short>(mNumChannels * mBitsPerSample / 8);
    mAvgBytesPerSec = mBlockAlign * sampleRate;
}

// Reconstructed from eboot.elf at 0xD6C60.
WaveFile::~WaveFile() {
    delete mListChunk;
}

// Inlined into WaveFile::RiffSize at 0xD6E70 and WaveFile::WriteCueChunk at
// 0xD7370.
int WaveFile::CueChunkSize() const {
    return static_cast<int>(mMarkers.size()) * 24 + 4;
}

// Reconstructed from eboot.elf at 0xD6DF0.
int WaveFile::LablSize() const {
    int size = 4;
    for (int i = 0; i < static_cast<int>(mMarkers.size()); ++i) {
        size += PaddedTextSize(mMarkers[i].mName) + 12;
    }
    return size;
}

// Reconstructed from eboot.elf at 0xD6E70.
int WaveFile::RiffSize() const {
    return DataSize() + CueChunkSize() + LablSize() + kRiffOverhead;
}

// Reconstructed from eboot.elf at 0xD6F30.
void WaveFile::Write(const char* path) {
    FileStream file(path, kWaveWriteFileMode, false);
    WriteRiffHeader(file);
    WriteFormatChunk(file);
    WriteDataChunk(file);
    WriteCueChunk(file);
    WriteLablChunk(file);
}

// Reconstructed from eboot.elf at 0xD6FD0.
void WaveFile::WriteRiffHeader(BinStream& stream) {
    stream.Write(&kRiffChunkID, sizeof(kRiffChunkID));
    int size = RiffSize();
    stream.WriteEndian(&size, sizeof(size));
    stream.Write(&kWaveChunkID, sizeof(kWaveChunkID));
}

// Reconstructed from eboot.elf at 0xD7100.
void WaveFile::WriteFormatChunk(BinStream& stream) {
    stream.Write(&kWaveFormatChunkID, sizeof(kWaveFormatChunkID));
    int size = kWaveFormatChunkSize;
    stream.WriteEndian(&size, sizeof(size));
    stream.WriteEndian(&mFormat, sizeof(mFormat));
    stream.WriteEndian(&mNumChannels, sizeof(mNumChannels));
    stream.WriteEndian(&mSampleRate, sizeof(mSampleRate));
    stream.WriteEndian(&mAvgBytesPerSec, sizeof(mAvgBytesPerSec));
    stream.WriteEndian(&mBlockAlign, sizeof(mBlockAlign));
    stream.WriteEndian(&mBitsPerSample, sizeof(mBitsPerSample));
}

// Reconstructed from eboot.elf at 0xD7200.
void WaveFile::WriteDataChunk(BinStream& stream) {
    int size = DataSize();
    stream.Write(&kWaveDataChunkID, sizeof(kWaveDataChunkID));
    stream.WriteEndian(&size, sizeof(size));
    WaveFileData data(*this);
    char buffer[kDataCopySize];
    for (int copied = 0; copied < size;) {
        int count = size - copied;
        if (count > kDataCopySize) {
            count = kDataCopySize;
        }
        data.Read(buffer, count);
        stream.Write(buffer, count);
        copied += count;
    }
    if ((size & 1) != 0) {
        char pad = 0;
        stream.Write(&pad, 1);
    }
}

// Reconstructed from eboot.elf at 0xD7370.
void WaveFile::WriteCueChunk(BinStream& stream) {
    stream.Write(&kWaveCueChunkID, sizeof(kWaveCueChunkID));
    int value = CueChunkSize();
    stream.WriteEndian(&value, sizeof(value));
    int numMarkers = static_cast<int>(mMarkers.size());
    value = numMarkers;
    stream.WriteEndian(&value, sizeof(value));
    for (int i = 0; i < numMarkers; ++i) {
        mMarkers[i].WriteCue(stream);
    }
}

// Reconstructed from eboot.elf at 0xD74F0.
void WaveFile::WriteLablChunk(BinStream& stream) {
    stream.Write(&kListChunkID, sizeof(kListChunkID));
    int size = LablSize();
    stream.WriteEndian(&size, sizeof(size));
    stream.Write(&kWaveAdditionalChunkID, sizeof(kWaveAdditionalChunkID));
    for (int i = 0; i < static_cast<int>(mMarkers.size()); ++i) {
        mMarkers[i].WriteLabel(stream);
    }
}

// Reconstructed from eboot.elf at 0xD7630. The data size is patched later.
void WaveFile::WriteFileHeader(BinStream& stream) {
    WriteRiffHeader(stream);
    WriteFormatChunk(stream);
    stream.Write(&kWaveDataChunkID, sizeof(kWaveDataChunkID));
    int size = 0;
    stream.WriteEndian(&size, sizeof(size));
}

// Reconstructed from eboot.elf at 0xD76B0.
void WaveFile::PatchDataSize(BinStream& stream, int numFrames) {
    mNumSamples = numFrames;
    stream.Seek(0, kSeekBegin);
    stream.Write(&kRiffChunkID, sizeof(kRiffChunkID));
    int size = DataSize() + 28;
    stream.WriteEndian(&size, sizeof(size));
    stream.Write(&kWaveChunkID, sizeof(kWaveChunkID));
    WriteFormatChunk(stream);
    size = DataSize();
    stream.Write(&kWaveDataChunkID, sizeof(kWaveDataChunkID));
    stream.WriteEndian(&size, sizeof(size));
}

// Reconstructed from eboot.elf at 0xD77B0.
IListChunk& WaveFile::PrepareToProvideData() {
    mListChunk->Reset();
    mListChunk->Next(kWaveDataChunkID);
    return *mListChunk;
}

// Reconstructed from eboot.elf at 0xD77E0.
WaveFileData::WaveFileData(WaveFile& wave) : IDataChunk(wave.PrepareToProvideData()), mWave(&wave) {}

// Reconstructed from eboot.elf at 0xD7830.
WaveFileData::~WaveFileData() {}

// Reconstructed from eboot.elf at 0xD7860.
WaveHeader::WaveHeader() : mFormatSize(kWaveFormatChunkSize), mFormat(kWaveFormatPcm) {
    std::memcpy(mRiffTag, "RIFF", sizeof(mRiffTag));
    std::memcpy(mWaveTag, "WAVE", sizeof(mWaveTag));
    std::memcpy(mFormatTag, "fmt ", sizeof(mFormatTag));
    std::memcpy(mDataTag, "data", sizeof(mDataTag));
}

// Reconstructed from eboot.elf at 0xD7890. The header goes out at once with
// empty sizes.
WaveFileWriter::WaveFileWriter(BinStream& stream, int sampleRate, int numChannels) : mStream(&stream) {
    mHeader.mRiffSize = 36;
    mHeader.mNumChannels = static_cast<unsigned short>(numChannels);
    mHeader.mSampleRate = sampleRate;
    mHeader.mAvgBytesPerSec = sampleRate * 2;
    mHeader.mBlockAlign = 2;
    mHeader.mBitsPerSample = 16;
    mHeader.mDataSize = 0;
    mStream->Write(&mHeader, sizeof(mHeader));
}

// Reconstructed from eboot.elf at 0xD78F0.
WaveFileWriter::~WaveFileWriter() {
    mStream->Seek(0, kSeekBegin);
    mStream->Write(&mHeader, sizeof(mHeader));
}

// Reconstructed from eboot.elf at 0xD7920.
bool WaveFileWriter::Write(unsigned char* data, int size) {
    mHeader.mDataSize += size;
    mHeader.mRiffSize += size;
    mStream->Write(data, size);
    return mStream->Fail();
}
