#pragma once

#include <cstddef>

class MemStream;
class PitchDetector;

// One of the game's microphones (rb_game/GameMic.o, 0xA38BA0-0xA399F0): the
// game side of a connected microphone, which pitch-detects its samples and
// keeps the energy the vocal track reads. It can play a WAV file instead of
// the microphone and record what it hears. GameMicManager creates one per
// connected microphone. Not polymorphic; the object is 32864 bytes. Only
// the small members are reconstructed: the sample processing reads the
// game's microphone channels (the object at 0x1ADF350), which are not
// modelled.
class GameMic {
public:
    // Takes a free slot among six recorders and a pitch detector at the
    // channel's sample rate (16000 without a microphone).
    explicit GameMic(int micID);  // 0xA38BA0
    // Writes the recordings, frees the recorder slot and the detector.
    ~GameMic();  // 0xA38FB0

    // Plays the WAV file in place of the microphone; null goes back to the
    // microphone. Returns the sample rate the detector uses.
    int SetInputFile(const char* path);  // 0xA38DE0
    // Empty in this build.
    void Reset();  // 0xA390E0
    // The microphone channel of the microphone id, from the object at
    // 0x1ADF350 (slot 8). Its class is not modelled; the map's GetMyMic()
    // returns a Mic*. Not reconstructed.
    void* GetMyMic();  // 0xA390F0
    void SetEnablePitchDetection(bool enable);  // 0xA39110
    // The continuous samples of the last update and their count.
    void AccessContinuousSamples(const short*& samples, int& count) const;  // 0xA39120
    // Empty in this build.
    void KillThread();  // 0xA39140
    // Rewinds the input file, if one plays.
    void StartDetection();  // 0xA39150
    // Mutes the microphone channel. Not reconstructed.
    void SetMute(bool mute);  // 0xA39170
    // Empty in this build.
    void UpdateThread();  // 0xA391B0
    // Analyses the latest samples. Not reconstructed.
    void ThreadProcessOneFrame();  // 0xA391C0
    // Processes a frame, reads the input file's samples for the elapsed
    // time and publishes the detector's results, the energy among them. Not
    // reconstructed.
    void Update();  // 0xA39610

    // Field names are not in the reference map.
    int mMicID;
    // The recorder slot taken from the six at 0x1AEF328.
    int mRecorderSlot;
    bool mDetectionFlags[2];
    // Write "mic_output_fonix.wav" and "mic_record_<id>.raw" on deletion.
    bool mOutputFonix;
    bool mRecordRaw;
    // The input file's sample rate; 0 when the microphone plays.
    int mInputSampleRate;
    MemStream* mInputStream;
    MemStream* mRecordStream;
    // The detector's working values, which Update publishes below: a state
    // value, the energy and the pitch (-1 for none), then the gain. The
    // names are inferred from Update's copies and the evidence is weak.
    int mDetectedState;
    PitchDetector* mPitchDetector;
    unsigned char mDetectorScratch[8];
    float mDetectedEnergy;
    float mDetectedPitch;
    float mGain;
    // The published values; the vocal track reads the energy through
    // GameMicManager::GetEnergyForMic.
    float mEnergy;
    int mState;
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
static_assert(offsetof(GameMic, mPitchDetector) == 40);
static_assert(offsetof(GameMic, mEnergy) == 68);
static_assert(offsetof(GameMic, mRecentSamples) == 80);
static_assert(offsetof(GameMic, mContinuousSamples) == 16464);
static_assert(offsetof(GameMic, mContinuousCount) == 32852);
static_assert(sizeof(GameMic) == 32864);
