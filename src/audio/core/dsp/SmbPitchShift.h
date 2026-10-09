#pragma once

#include <cstddef>

template <class T>
class AudioBuffer;
class AudioData;
class AudioDecoder;

// Phase-vocoder pitch shifter after Stephan Bernsee's smbPitchShift (the
// string "core/audio/dsp/SmbPitchShift" at 0x124E20A). FusionVoicePool owns
// a set of them for keyzones that keep their timing, and the
// HMX.SmbPitchShift DSP owns one per instance. A decoder bound to it passes
// its output through Render. The class, its file and all member names are
// not in the reference map. The vtable is at 0x18E61E8 and has no type
// information; the object is 0x64070 bytes.
class SmbPitchShift {
public:
    static constexpr int kMaxFrameSize = 4096;
    static constexpr int kMaxChannels = 3;  // Left, right and their mix.

    // State of one channel's overlap-add, cleared as a block.
    struct ChannelState {
        void Clear();  // 0xDEF10, not called in this build.

        float mInFifo[kMaxFrameSize];
        float mOutFifo[kMaxFrameSize];
        float mFftWorkspace[2 * kMaxFrameSize];
        float mLastPhase[kMaxFrameSize / 2 + 1];
        float mSumPhase[kMaxFrameSize / 2 + 1];
        float mOutputAccum[2 * kMaxFrameSize];
        int mRover;  // Next free input slot; mLatency once primed.
    };

    // Inlined wherever one is created (FusionVoicePool's setup at 0xA12C0,
    // the DSP's create callback at 0x27FBE0): the shared window tables are
    // built on first use. Nothing else is initialized.
    SmbPitchShift() {
        if (!sTablesInitialized) {
            InitTables();
        }
    }
    virtual ~SmbPitchShift();  // slots 0-1: 0xE0350, 0xE0360
    // Slot 2 at 0xE0370, empty in this class. FusionVoicePool passes the
    // keyzone's "algorithm" (as_authored, default, elastique_pro,
    // elastique_eff_with_formant, elastique_mobile) and its formant mode
    // (1 with "maintain_formant", else 2). Name not in the reference map.
    virtual void SetTimeStretchMode(int algorithm, int formantMode);
    // Slot 3 at 0xE0380, empty in this class. No caller was found; the name
    // is a guess.
    virtual void Reset();
    // Slot 4 at 0xDF280: stores the rate per FFT bin and clears the state.
    virtual void SetSampleRate(float sampleRate);
    // Slot 5 at 0xDF250: input samples held back, mFrameSize - mStepSize.
    virtual int GetLatency();
    // Slots 6-7 at 0xE0390 and 0xE03A0 return false and 0. FusionVoice adds
    // the second to its position when the first is true. Names are guesses.
    virtual bool HasPositionOffset();
    virtual int GetPositionOffset();
    // Slot 8 at 0xDF300: renders the decoder's output at rate * timeRatio
    // and shifts it by pitchRatio / timeRatio, so the pitch follows
    // rate * pitchRatio while the timing follows rate * timeRatio. Returns
    // the decoder's next position. AudioDecoder's slot 6 forwards to it
    // with the same arguments.
    virtual double Render(
        AudioBuffer<float>& buffer,
        double position,
        double rate,
        double pitchRatio,
        double timeRatio,
        bool loop,
        const unsigned int* channelMap,
        const float* gains);
    // Slot 9 at 0xDFB90: shifts a stereo pair, analysing the bins below the
    // mono cutoff from the mix of both channels.
    virtual void ProcessStereo(
        long numSamples,
        const float* inLeft,
        const float* inRight,
        float* outLeft,
        float* outRight,
        float pitchShift);
    // Slot 10 at 0xDF2B0: SetSampleRate with the bound data's rate. The
    // decoder calls it when binding.
    virtual void UpdateSampleRate();

    // Sizes the FFT frame and the oversampling factor and clears the state.
    // At 0xDEF30.
    void Setup(int frameSize, int oversampling);
    void ClearBuffers();  // 0xDF230
    // Bins below this frequency share the first channel's analysis. At
    // 0xDF260.
    void SetMonoCutoff(float frequency);
    // Shifts one channel of stride-spaced samples in place or between
    // buffers; channels after the first reuse the first channel's analysis
    // below the mono cutoff. At 0xDF3C0.
    void ProcessChannel(
        long numSamples,
        const float* in,
        float* out,
        int channel,
        int stride,
        float pitchShift);
    // smbFft over 2 * mFrameSize interleaved values; sign -1 is the forward
    // transform. At 0xDF940.
    void Fft(float* buffer, long sign);

    // Builds the Hann windows for frame sizes 128 to 4096. At 0xDF040.
    static void InitTables();
    static bool sTablesInitialized;  // 0x19E26A0
    static float sWindow128[128];    // 0x19DA8A0
    static float sWindow256[256];    // 0x19DAAA0
    static float sWindow512[512];    // 0x19DAEA0
    static float sWindow1024[1024];  // 0x19DB6A0
    static float sWindow2048[2048];  // 0x19DC6A0
    static float sWindow4096[4096];  // 0x19DE6A0

    // Set by AudioDecoder::SetAudioData; the constructor leaves them unset.
    const AudioData* mAudioData;
    AudioDecoder* mDecoder;
    ChannelState mChannels[kMaxChannels];
    float mAnaFreq[kMaxFrameSize];
    float mAnaMagn[kMaxFrameSize];
    float mSynFreq[kMaxFrameSize];
    float mSynMagn[kMaxFrameSize];
    int mFrameSize;
    int mHalfFrameSize;
    int mOversampling;
    int mStepSize;
    int mLatency;
    double mExpectedPhase;  // Phase advance per step of bin 1.
    double mFreqPerBin;
    const float* mWindow;  // The table for mFrameSize.
    int mMonoCutoffBin;
};

static_assert(sizeof(SmbPitchShift::ChannelState) == 0x1C00C);
static_assert(offsetof(SmbPitchShift, mAudioData) == 0x8);
static_assert(offsetof(SmbPitchShift, mChannels) == 0x18);
static_assert(offsetof(SmbPitchShift, mAnaFreq) == 0x5403C);
static_assert(offsetof(SmbPitchShift, mSynMagn) == 0x6003C);
static_assert(offsetof(SmbPitchShift, mFrameSize) == 0x6403C);
static_assert(offsetof(SmbPitchShift, mLatency) == 0x6404C);
static_assert(offsetof(SmbPitchShift, mExpectedPhase) == 0x64050);
static_assert(offsetof(SmbPitchShift, mWindow) == 0x64060);
static_assert(offsetof(SmbPitchShift, mMonoCutoffBin) == 0x64068);
static_assert(sizeof(SmbPitchShift) == 0x64070);

// Name of a "timestretch_settings" algorithm: as_authored, default,
// elastique_pro, elastique_eff_with_formant or elastique_mobile, else
// "unrecognized". The table is at 0x18E6240. At 0xE03E0. Name not in the
// reference map.
const char* GetTimeStretchAlgorithmName(int algorithm);
// The inverse; unknown names give 1, "default". At 0xE0400. Name not in the
// reference map.
int GetTimeStretchAlgorithm(const char* name);
