#pragma once

#include "entity/resources/Resource.h"

class BinStream;
class String;

// Decoded or encoded audio owned by a sample resource (audio/AudioData.o).
// The class has not been reconstructed; its slots are declared so the call
// FusionVoicePool makes uses the recovered offset. The names come from
// MoggAudioData's implementations (vtable 0x18E5F88) and the map's
// PcmAudioData and AudioData members; names not in the reference map unless
// noted.
class AudioData {
public:
    // The values GetEncodedFormat returns; AudioDecoder takes them. Value
    // names not in the reference map.
    enum EncodedFormat : int {
        kEncodedPcm = 0,
        kEncodedXma = 1,
        kEncodedMogg = 2,
    };

    virtual ~AudioData();                          // slots 0-1
    virtual int GetMemoryUsage() const;            // slot 2
    virtual bool Save(BinStream& stream);          // slot 3: writes the data
    virtual void SetPath(const String& path);      // slot 4. In the map.
    virtual unsigned long GetDataSize() const;     // slot 5
    virtual int GetEncodedFormat() const;          // slot 6: PCM 0, XMA 1, Mogg 2
    // Slots 7-8: two counts after the sample rate; frames then channels is a
    // guess.
    virtual int GetNumFrames() const;
    virtual int GetNumChannels() const;
    virtual float GetSampleRate() const;           // slot 9
    virtual const String& GetPath() const;         // slot 10
    virtual AudioData* Clone() const;              // slot 11: copies the data
    virtual int GetSampleFormat() const;           // slot 12: 0 for compressed data
    // Slot 13: true without a sample rate. FusionVoicePool gives such a
    // keyzone no voice.
    virtual bool IsEmpty() const;
};

// Sample loaded for a sampler keyzone. The class has not been reconstructed;
// only the slot FusionVoicePool calls is declared.
class AudioSampleResource : public Resource {
public:
    // Slot 13, the first after Resource's. Name not in the reference map.
    virtual AudioData* GetAudioData();
};
