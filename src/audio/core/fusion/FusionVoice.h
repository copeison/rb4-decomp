#pragma once

#include <cstddef>

#include "audio/core/buffers/AudioBuffer.h"
#include "audio/core/dsp/BiquadFilter.h"
#include "audio/core/dsp/Ramper.h"
#include "audio/core/fusion/FusionPatchCom.h"
#include "audio/core/modulation/ADSR.h"
#include "audio/core/modulation/LFO.h"
#include "os/memory/MemMgr.h"

class AudioDecoder;
class FusionSampler;
class FusionVoicePool;
class SmbPitchShift;

// One sampler voice (audio/FusionVoice.o). FusionVoicePool allocates them in
// an array and hands them to its samplers. A voice renders one keyzone's
// sample through a decoder, shapes it with two envelopes, two LFOs and a
// ramped biquad filter, and pans it into a stereo pair. The object is 784
// bytes.
class FusionVoice {
public:
    // Voice arrays come from the tracked heap under the label "FusionVoice",
    // as FusionVoicePool's voice setup at 0xA1090 shows.
    static void* operator new[](unsigned long size) {
        return MemAlloc(size, "FusionVoice", 0);
    }
    static void operator delete[](void* allocation) {
        MemFree(allocation);
    }

    FusionVoice();   // 0x9D920
    ~FusionVoice();  // 0x9DD30

    void SetSampleRate(double sampleRate);  // 0x9DC50
    // The map has AssignIDs(FusionSampler const*, KeyzoneSettings const*,
    // unsigned int); this build also passes the voice's pitch shifter and
    // returns whether the voice took the note. At 0x9DE50.
    bool AssignIDs(
        const FusionSampler* sampler,
        const FusionPatchCom::KeyzoneSettings* keyzone,
        unsigned int id,
        SmbPitchShift* pitchShift);
    // Binds LFO 0, or LFO 1 for any other index. At 0x9E130.
    void SetupLFO(int index, const LFO::Settings& settings);
    void Release();      // 0x9E2E0
    void FastRelease();  // 0x9E380
    // Silences the voice and returns its pitch shifter to the pool. At
    // 0x9E440.
    void Kill();
    // The map has Init(char); this build also binds the pool and, unless
    // skipDecoders is set, creates the voice's Mogg and XMA decoders. At
    // 0x9E520.
    void Init(FusionVoicePool* pool, char id, bool skipDecoders);
    // The map's version is larger; this build only stores the offset, which
    // the next attack adds to the note's. At 0x9E6A0.
    void SetPitchOffset(double cents);
    // Starts the assigned keyzone's sample at a note, a gain and a start
    // offset. At 0x9EFB0.
    void AttackWithTargetNote(unsigned char note, float gain, double startOffsetMs);
    bool IsWaitingForAttack() const;  // 0x9F110
    bool IsInUse() const;             // 0x9F120
    // Adds numFrames of the voice to the two channels and returns the count,
    // or zero when the voice is idle. The map has Process(float**, unsigned
    // int, unsigned int, float); this build adds the tempo. FusionSampler
    // passes the block's channel count, which the voice ignores. At 0x9F2A0.
    unsigned int Process(float** channels, unsigned int numChannels, unsigned int numFrames, float speed, float tempo);
    // A null keyzone matches any. At 0xA0320.
    bool MatchesIDs(
        const FusionSampler* sampler,
        unsigned int id,
        const FusionPatchCom::KeyzoneSettings* keyzone);

    // Pitch offsets are clamped to twelve octaves either way. In the map;
    // 0x124C0A8 and 0x124C0B8 hold its folded uses.
    static const double kMaxPitchOffsetCents;

private:
    void _Attack();  // 0x9E160
    // Resets the LFOs, the gains and the filter for a new note. At 0x9E6B0.
    void _PrepareWithPitchOffsetAndGain(double pitchOffsetCents, float gain);
    // The LFO and envelope modulation of the filter, in octaves. Inlined
    // into _ApplyModsToFilter and _PrepareWithPitchOffsetAndGain in this
    // build.
    float _ComputeOctaveShift();
    // Moves the filter to the modulated cutoff; a large jump first ramps
    // the filter gain down. At 0x9EBA0.
    void _ApplyModsToFilter(bool force);
    // The two-by-two mix from the sample's channels to the outputs. No
    // caller in this build: _PrepareWithPitchOffsetAndGain and Process
    // inline it. At 0x9EE90.
    void _CalculatePanMix(float* leftToLeft, float* leftToRight, float* rightToLeft, float* rightToRight) const;
    // Ramp callbacks: ramp the coefficients to the new settings, then the
    // filter gain back to unity. At 0x9F150 and 0x9F250.
    static void _UpdateFilterSettings(void* voice);
    static void _RestoreFilterGain(void* voice);

public:
    // Field names are not in the reference map.
    bool mWaitingForAttack;  // Set by AssignIDs until the attack.
    char mId;                // Set by Init: 'A' plus the pool index.
    float mNote;
    float mGain;  // The attack gain times the keyzone volume.
    int mNoteId;  // Compared by MatchesIDs; -1 once killed.
    const FusionPatchCom::KeyzoneSettings* mKeyzone;
    const FusionSampler* mSampler;
    AudioDecoder* mPcmDecoder;
    AudioDecoder* mXmaDecoder;
    AudioDecoder* mMoggDecoder;
    AudioDecoder* mDecoder;  // The one that decodes the keyzone's sample.
    SmbPitchShift* mPitchShift;
    // The sampler's two envelopes: volume, then the assignable one.
    ADSR::State mAmpEnvelope;
    ADSR::State mModEnvelope;
    LFO mLFOs[2];
    BiquadFilter mFilter;
    float mFilterFrequency;  // The sampler's cutoff before modulation.
    BiquadFilter::Settings mFilterSettings;
    float mFilterOctaves;  // The last modulation applied.
    // Scales the filter output; dipped while the cutoff jumps.
    SPL::Ramper mFilterGain;
    // A view of the output channels, one control block at a time.
    AudioBuffer<float> mBuffer;
    float mPanMix[4];  // Left to left, left to right, right to left, right to right.
    float mOutputGain;
    float mLevels[2];  // Each channel's peak in the last Process.
    double mSampleRateRatio;  // Source frames per output sample.
    double mSampleRate;
    double mSecondsPerSample;
    double mPosition;  // In source frames.
    double mPitchRatio;  // From the note and the pitch offset.
    double mPlaybackRatio;
    double mPitchOffset;  // Cents, from SetPitchOffset.
    double mNumFrames;    // The sample's length.
    FusionVoicePool* mPool;  // Set by Init.
    // Tempo-synced keyzones: the beat the note started on, the speed
    // correction that keeps the sample on the beat, the position of the last
    // check and the position at the attack.
    float mStartBeat;
    float mTempoCorrection;
    double mLastSyncPosition;
    double mStartPosition;
};

static_assert(offsetof(FusionVoice, mNote) == 4);
static_assert(offsetof(FusionVoice, mNoteId) == 12);
static_assert(offsetof(FusionVoice, mKeyzone) == 16);
static_assert(offsetof(FusionVoice, mSampler) == 24);
static_assert(offsetof(FusionVoice, mPcmDecoder) == 32);
static_assert(offsetof(FusionVoice, mDecoder) == 56);
static_assert(offsetof(FusionVoice, mPitchShift) == 64);
static_assert(offsetof(FusionVoice, mAmpEnvelope) == 72);
static_assert(offsetof(FusionVoice, mModEnvelope) == 112);
static_assert(offsetof(FusionVoice, mLFOs) == 152);
static_assert(offsetof(FusionVoice, mFilter) == 248);
static_assert(offsetof(FusionVoice, mFilterFrequency) == 456);
static_assert(offsetof(FusionVoice, mFilterSettings) == 460);
static_assert(offsetof(FusionVoice, mFilterOctaves) == 476);
static_assert(offsetof(FusionVoice, mFilterGain) == 480);
static_assert(offsetof(FusionVoice, mBuffer) == 528);
static_assert(offsetof(FusionVoice, mPanMix) == 656);
static_assert(offsetof(FusionVoice, mOutputGain) == 672);
static_assert(offsetof(FusionVoice, mLevels) == 676);
static_assert(offsetof(FusionVoice, mSampleRateRatio) == 688);
static_assert(offsetof(FusionVoice, mPosition) == 712);
static_assert(offsetof(FusionVoice, mPitchOffset) == 736);
static_assert(offsetof(FusionVoice, mPool) == 752);
static_assert(offsetof(FusionVoice, mStartBeat) == 760);
static_assert(offsetof(FusionVoice, mStartPosition) == 776);
static_assert(sizeof(FusionVoice) == 784);
