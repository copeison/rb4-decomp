#pragma once

#include <cstddef>

class BinStream;

// RIFF wave file description (audio/WaveFile.o). The class has not been
// reconstructed; only the members RecordingAudioRenderTarget calls are
// declared. The object is 64 bytes.
class WaveFile {
public:
    WaveFile(int sampleRate, int bitsPerSample, int numChannels);  // 0xD6C10
    ~WaveFile();                                                   // 0xD6C60

    // Writes the RIFF, format and data headers.
    void WriteFileHeader(BinStream& stream);  // 0xDB630
    // Rewrites the size fields once the sample data is complete.
    void PatchDataSize(BinStream& stream, int numFrames);  // 0xD76B0

    unsigned char mUnknown0[64];  // Field names are not in the reference map.
};

static_assert(sizeof(WaveFile) == 64);
