#pragma once

#include "audio/core/buffers/AudioBuffer.h"
#include "audio/core/resources/AudioSampleResource.h"

class FusionSampler;
class SmbPitchShift;

// Renders one encoded format of AudioData into a buffer. The map places the
// base in audio/AudioData.o, beside PcmAudioDecoder; the class has not been
// reconstructed, and only what FusionVoice uses is declared. PcmAudioDecoder's
// vtable is at 0x18DCEF0 and the Mogg decoder's at 0x18E6008.
class AudioDecoder {
public:
    // A PCM or Mogg decoder; null for other formats, XMA among them. At
    // 0xD4540, in audio/AudioData.o.
    static AudioDecoder* NewDecoderForFormat(AudioData::EncodedFormat format);

    // Default channel map and gains for Render.
    static const unsigned int kOneToOneIOMap[32];
    static const float kUnityGainMap[32];

    virtual ~AudioDecoder();  // slots 0-1
    virtual bool CanDecode(AudioData::EncodedFormat format) const;  // slot 2
    // Slot 3: the slot-4 form without a track map or an owner. The map has
    // SetAudioData(AudioData const*); this build passes the pitch shifter
    // that time-stretches the data.
    virtual void SetAudioData(const AudioData* data, SmbPitchShift* pitchShift);
    // Slot 4: also binds the keyzone's "track_map" array and the sampler
    // that plays it. The types of the last two parameters are not
    // established.
    virtual void SetAudioData(
        const AudioData* data,
        SmbPitchShift* pitchShift,
        const void* trackMap,
        const FusionSampler* sampler);
    virtual void SetFrame(unsigned int frame);  // slot 5
    // Slot 6: renders the buffer's frames from a source position and returns
    // the next position. The map has Render(AudioBuffer<float>&, double,
    // double, bool, unsigned int const*, float const*) const; this build
    // adds the pitch ratio and the time ratio for pitch-shifted data. The
    // parameter names are inferred from FusionVoice::Process.
    virtual double Render(
        AudioBuffer<float>& buffer,
        double position,
        double rate,
        double pitchRatio,
        double timeRatio,
        bool loop,
        const unsigned int* channelMap,
        const float* gains);
    // Slot 7: renders without the pitch shifter; SmbPitchShift::Render
    // calls it. This is the map's Render signature.
    virtual double Render(
        AudioBuffer<float>& buffer,
        double position,
        double rate,
        bool loop,
        const unsigned int* channelMap,
        const float* gains);
};

// The PCM decoder (audio/AudioData.o). Only its construction is declared.
// The object is 48 bytes.
class PcmAudioDecoder : public AudioDecoder {
public:
    PcmAudioDecoder();  // 0x12C00

    // The decoder's state after the vtable, cleared by the constructor; not
    // reconstructed.
    unsigned char mDecoderState[40];
};

static_assert(sizeof(PcmAudioDecoder) == 48);

// The Mogg (Ogg Vorbis) decoder. The constructor allocates the
// "MoggAudioPrivateDecoder" state and a 800 KiB "VorbisPool", and looks up
// the default render target's "mogg" object. Only its construction is
// declared. Name not in the reference map; the object is 120 bytes.
class MoggAudioDecoder : public AudioDecoder {
public:
    MoggAudioDecoder();  // 0xD4B60

    // The decoder's state after the vtable; not reconstructed.
    unsigned char mDecoderState[112];
};

static_assert(sizeof(MoggAudioDecoder) == 120);
