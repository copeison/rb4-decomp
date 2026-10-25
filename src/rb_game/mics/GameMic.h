#pragma once

#include <cstddef>

class MemStream;
class MicChannel;
class PitchDetector;

// One of the game's microphones (rb_game/GameMic.o, 0xA38BA0-0xA399F0): the
// game side of a connected microphone, which pitch-detects its samples and
// keeps the energy the vocal track reads. It can play a WAV file instead of
// the microphone and record what it hears. GameMicManager creates one per
// connected microphone. Not polymorphic; the object is 32864 bytes. The
// samples come from the game's microphone channels (TheMicChannelMgr,
// mic/core/MicChannelMgr.h).
class GameMic {
public:
    // The recorder slots the microphones share. Name not in the reference
    // map.
    static constexpr int kNumRecorderSlots = 6;

    // Takes a free recorder slot and a pitch detector at the channel's
    // sample rate (16000 without a microphone).
    explicit GameMic(int micID);  // 0xA38BA0
    // Writes the recordings, frees the recorder slot and the detector.
    ~GameMic();  // 0xA38FB0

    // Plays the WAV file in place of the microphone; null goes back to the
    // microphone. Replaces the detector with one at the new sample rate and
    // returns it.
    int SetInputFile(const char* path);  // 0xA38DE0
    // Empty in this build.
    void Reset();  // 0xA390E0
    // The microphone channel of the microphone id. The map's GetMyMic()
    // returns a Mic*.
    MicChannel* GetMyMic();  // 0xA390F0
    // Turns the detector's analysis on or off.
    void SetEnablePitchDetection(bool enable);  // 0xA39110
    // The continuous samples of the last update and their count.
    void AccessContinuousSamples(const short*& samples, int& count) const;  // 0xA39120
    // Empty in this build.
    void KillThread();  // 0xA39140
    // Rewinds the input file, if one plays.
    void StartDetection();  // 0xA39150
    // Mutes the microphone channel, when the microphone has one.
    void SetMute(bool mute);  // 0xA39170
    // Empty in this build.
    void UpdateThread();  // 0xA391B0
    // Fetches the channel's latest samples (unless the input file plays),
    // runs the detector over them, smooths the energy and records what it
    // heard: the samples when writing "mic_output_fonix.wav", else the
    // samples and the energy, with silence for the dropped samples. Without
    // a bound microphone the detected values are cleared.
    void ThreadProcessOneFrame();  // 0xA391C0
    // Processes a frame, reads the input file's samples up to the clock's
    // time and publishes the detected values.
    void Update();  // 0xA39610
    // The input file's sample rate, else the channel's (16000 without a
    // microphone).
    int GetDataSampleRate();  // 0xA39750
    // Copies the samples scaled by the gain, then adapts the gain to the
    // block's peak (at most 2). The binary keeps an uncalled copy.
    void NormalizingCopy(short* out, short* in, int count, float& gain);  // 0xA39790

    // Field names are not in the reference map.
    int mMicID;
    // The recorder slot taken from the six at 0x1AEF328.
    int mRecorderSlot;
    // The first is set by Update while the channel is not bound (status 1);
    // the second is not read. The names are weak.
    bool mDetectionFlags[2];
    // Write "mic_output_fonix.wav" and "mic_record_<id>.raw" on deletion.
    bool mOutputFonix;
    bool mRecordRaw;
    // The input file's sample rate; 0 when the microphone plays.
    int mInputSampleRate;
    MemStream* mInputStream;
    MemStream* mRecordStream;
    // The detector's working values, which Update publishes below: the
    // input's peak, the energy and the pitch (-1 for none), then the gain.
    // The names are inferred from Update's copies and PitchDetector's
    // outputs.
    float mDetectedPeak;
    PitchDetector* mPitchDetector;
    unsigned char mDetectorScratch[8];
    float mDetectedEnergy;
    float mDetectedPitch;
    float mGain;
    // The published values; the vocal track reads the energy through
    // GameMicManager::GetEnergyForMic.
    float mEnergy;
    float mPeak;
    float mPitch;
    // The latest samples and the continuous samples of the last update.
    short mRecentSamples[0x2000];
    short mContinuousSamples[0x2000];
    int mRecentCount;
    int mContinuousCount;
    // Cleared by the constructor; not read by the reconstructed code.
    bool mFrameProcessed;
};

static_assert(offsetof(GameMic, mInputSampleRate) == 12);
static_assert(offsetof(GameMic, mInputStream) == 16);
static_assert(offsetof(GameMic, mDetectedPeak) == 32);
static_assert(offsetof(GameMic, mPitchDetector) == 40);
static_assert(offsetof(GameMic, mDetectedEnergy) == 56);
static_assert(offsetof(GameMic, mGain) == 64);
static_assert(offsetof(GameMic, mEnergy) == 68);
static_assert(offsetof(GameMic, mPeak) == 72);
static_assert(offsetof(GameMic, mRecentSamples) == 80);
static_assert(offsetof(GameMic, mContinuousSamples) == 16464);
static_assert(offsetof(GameMic, mRecentCount) == 32848);
static_assert(offsetof(GameMic, mContinuousCount) == 32852);
static_assert(offsetof(GameMic, mFrameProcessed) == 32856);
static_assert(sizeof(GameMic) == 32864);
