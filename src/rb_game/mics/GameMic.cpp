#include "rb_game/mics/GameMic.h"

#include <cstring>
#include <ctime>

#include "audio/core/analysis/PitchDetector.h"
#include "audio/core/formats/WaveFile.h"
#include "mic/core/MicChannelMgr.h"
#include "os/files/File.h"
#include "rb_game/mics/GameMicManager.h"
#include "utl/streams/FileStream.h"
#include "utl/streams/MemStream.h"
#include "utl/text/HmxSnprintf.h"
#include "utl/time/TimeMgr.h"

// Writes 16-bit mono PCM samples as a wave file. Its object is not
// identified. Name not in the reference map.
void WriteWaveFile(const char* path, int sampleRate, const void* data, int size);  // 0x111D110

namespace {

// Which of the recorder slots a microphone holds. At 0x1AEF328. Name not
// in the reference map.
bool gRecorderSlotsUsed[GameMic::kNumRecorderSlots];

// The detector's rate without a microphone.
constexpr int kDefaultSampleRate = 16000;
// The energy is normalized by the gain times this.
constexpr float kEnergyScale = 500.0F;
// The smoothing of a rising and of a falling energy, indexed by whether it
// rises.
const float kEnergySmoothing[2] = {0.1F, 0.3F};
// The input file's continuous samples and the recordings stay below this
// many samples per frame.
constexpr int kMaxDroppedSamples = 48000;
// The size the recording stream reserves.
constexpr unsigned long kRecordReserve = 0x1C00000;

}  // namespace

// Reconstructed from eboot.elf at 0xA38BA0. SetInputFile(nullptr) is
// inlined.
GameMic::GameMic(int micID)
    : mMicID(micID),
      mRecorderSlot(-1),
      mDetectionFlags{true, true},
      mOutputFonix(false),
      mRecordRaw(false),
      mInputSampleRate(0),
      mInputStream(nullptr),
      mRecordStream(nullptr),
      mPitchDetector(nullptr),
      mDetectorScratch(),
      mGain(1.0F),
      mRecentCount(0),
      mContinuousCount(0),
      mFrameProcessed(false) {
    for (int slot = 0; slot < kNumRecorderSlots; slot++) {
        if (!gRecorderSlotsUsed[slot]) {
            mRecorderSlot = slot;
            gRecorderSlotsUsed[slot] = true;
            break;
        }
    }
    mOutputFonix = false;
    mRecordRaw = false;
    SetInputFile(nullptr);
    mEnergy = 0.0F;
    mDetectedEnergy = 0.0F;
    mPeak = 0.0F;
    mDetectedPeak = 0.0F;
    mPitch = -1.0F;
    mDetectedPitch = -1.0F;
    if (mOutputFonix || mRecordRaw) {
        mRecordStream = new MemStream(false);
        mRecordStream->mBuffer.reserve(kRecordReserve);
    }
    memset(mRecentSamples, 0, sizeof(mRecentSamples) + sizeof(mContinuousSamples));
}

// Reconstructed from eboot.elf at 0xA38DE0.
int GameMic::SetInputFile(const char* path) {
    int sampleRate;
    if (path != nullptr) {
        mOutputFonix = false;
        mRecordRaw = false;
        FileStream file(path, kRead, false);
        WaveFile wave(file);
        mInputSampleRate = wave.mSampleRate;
        WaveFileData data(wave);
        if (mInputStream == nullptr) {
            mInputStream = new MemStream(false);
        }
        mInputStream->Resize(wave.DataSize());
        data.Read(mInputStream->mBuffer.begin(), wave.DataSize());
        sampleRate = mInputSampleRate;
    } else {
        sampleRate = mMicID != -1 ? TheMicChannelMgr->GetMic(mMicID)->GetSampleRate() : kDefaultSampleRate;
        mInputSampleRate = 0;
    }
    delete mPitchDetector;
    mPitchDetector = nullptr;
    mPitchDetector = new PitchDetector(sampleRate);
    return sampleRate;
}

// Reconstructed from eboot.elf at 0xA38FB0.
GameMic::~GameMic() {
    if (mOutputFonix) {
        const char* const samples = mRecordStream->mBuffer.begin();
        WriteWaveFile("mic_output_fonix.wav", kDefaultSampleRate, samples, mRecordStream->Size());
    }
    if (mRecordRaw) {
        char name[64];
        HmxSnprintf(name, sizeof(name), "mic_record_%d.raw", mMicID);
        void* const file = FileOpen(name, kWrite);
        const char* const samples = mRecordStream->mBuffer.begin();
        FileWrite(file, samples, mRecordStream->Size());
        FileClose(file);
    }
    delete mRecordStream;
    mRecordStream = nullptr;
    delete mInputStream;
    mInputStream = nullptr;
    mInputSampleRate = 0;
    gRecorderSlotsUsed[mRecorderSlot] = false;
    delete mPitchDetector;
}

// Reconstructed from eboot.elf at 0xA390E0.
void GameMic::Reset() {}

// Reconstructed from eboot.elf at 0xA390F0.
MicChannel* GameMic::GetMyMic() {
    return TheMicChannelMgr->GetMic(mMicID);
}

// Reconstructed from eboot.elf at 0xA39110.
void GameMic::SetEnablePitchDetection(bool enable) {
    if (mPitchDetector != nullptr) {
        mPitchDetector->mComputePitch = enable;
    }
}

// Reconstructed from eboot.elf at 0xA39120.
void GameMic::AccessContinuousSamples(const short*& samples, int& count) const {
    samples = mContinuousSamples;
    count = mContinuousCount;
}

// Reconstructed from eboot.elf at 0xA39140.
void GameMic::KillThread() {}

// Reconstructed from eboot.elf at 0xA39150.
void GameMic::StartDetection() {
    if (mInputSampleRate != 0) {
        mInputStream->Seek(0, kSeekBegin);
    }
}

// Reconstructed from eboot.elf at 0xA39170.
void GameMic::SetMute(bool mute) {
    if (mMicID != -1) {
        GetMyMic()->SetMute(mute);
    }
}

// Reconstructed from eboot.elf at 0xA391B0.
void GameMic::UpdateThread() {}

// Reconstructed from eboot.elf at 0xA391C0. The release build keeps an
// unused clock() call.
void GameMic::ThreadProcessOneFrame() {
    float pitch = 0.0F;
    float energy = 0.0F;
    static_cast<void>(clock());
    int dropped = 0;
    if (mInputSampleRate == 0 && mMicID != -1) {
        MicChannel* const mic = GetMyMic();
        dropped = mic->GetDroppedSamples();
        const short* const recent = mic->GetRecentBuf(mRecentCount);
        if (mRecentCount > 0) {
            mRecentCount = mRecentCount < 0x2001 ? mRecentCount : 0x2000;
            memcpy(mRecentSamples, recent, mRecentCount * sizeof(short));
        }
        const short* const continuous = mic->GetContinuousBuf(mContinuousCount);
        if (mContinuousCount > 0) {
            mContinuousCount = mContinuousCount < 0x2001 ? mContinuousCount : 0x2000;
            memcpy(mContinuousSamples, continuous, mContinuousCount * sizeof(short));
        }
    }
    if (mRecentCount > 0) {
        MicChannel* const mic = GetMyMic();
        float level = 0.0F;
        PitchDetector* const detector = mPitchDetector;
        const char* const name = mic->GetName().Str();
        const float sensitivity =
            TheGameMicManager->mHighSensitivity ? TheGameMicManager->mFxValues[0] : mic->GetSensitivity();
        detector->AnalyzeBlock(name, mRecentSamples, mRecentCount, sensitivity, mic->mGain, pitch, energy, level,
                               mDetectedPeak);
        energy = energy / (mGain * kEnergyScale);
        if (energy < 0.0F) {
            energy = 0.0F;
        }
        if (energy > 1.0F) {
            energy = 1.0F;
        }
        const float smoothing = kEnergySmoothing[energy > mDetectedEnergy];
        mDetectedEnergy = (1.0F - smoothing) * mDetectedEnergy + smoothing * energy;
        mDetectedPitch = pitch;
        if ((mOutputFonix || mRecordRaw) && TheTimeMgr->mClock.Seconds() >= 0.0F) {
            if (static_cast<unsigned int>(dropped - 1) < kMaxDroppedSamples - 1) {
                if (mOutputFonix) {
                    short* const silence = new short[dropped]();
                    mRecordStream->Write(silence, dropped * sizeof(short));
                    delete[] silence;
                } else {
                    for (int i = dropped; i != 0; i--) {
                        short sample = 0;
                        mRecordStream->WriteEndian(&sample, sizeof(sample));
                        sample = static_cast<short>(32767.0F * mDetectedEnergy);
                        mRecordStream->WriteEndian(&sample, sizeof(sample));
                    }
                }
            }
            if (mOutputFonix) {
                mRecordStream->Write(mContinuousSamples, mContinuousCount * sizeof(short));
            } else {
                for (int i = 0; i < mContinuousCount; i++) {
                    const float level = mDetectedEnergy;
                    short sample = mContinuousSamples[i];
                    mRecordStream->WriteEndian(&sample, sizeof(sample));
                    sample = 1.0F < level ? 0x7FFF : static_cast<short>((level > 0.0F ? level : 0.0F) * 32767.0F);
                    mRecordStream->WriteEndian(&sample, sizeof(sample));
                }
            }
        }
    }
    if (mInputSampleRate == 0 && (mMicID == -1 || GetMyMic()->GetStatus() == 0)) {
        mDetectedPitch = 0.0F;
        mDetectedEnergy = 0.0F;
        mDetectedPeak = 0.0F;
    }
}

// Reconstructed from eboot.elf at 0xA39610. GetDataSampleRate is inlined.
void GameMic::Update() {
    ThreadProcessOneFrame();
    if (mInputSampleRate != 0) {
        const float seconds = TheTimeMgr->mClock.Seconds();
        const int position = static_cast<int>(GetDataSampleRate() * seconds);
        const int read = mInputStream->Tell() / 2;
        const int size = mInputStream->Size() / 2;
        int end = position < read ? read : position;
        if (position > size) {
            end = size;
        }
        const int count = end - static_cast<int>(static_cast<unsigned long>(mInputStream->Tell()) / 2);
        mContinuousCount = count < 0x2001 ? count : 0x2000;
        mInputStream->Read(mContinuousSamples, mContinuousCount * sizeof(short));
    }
    mEnergy = mDetectedEnergy;
    mPitch = mDetectedPitch;
    mPeak = mDetectedPeak;
    if (mMicID != -1) {
        mDetectionFlags[0] = GetMyMic()->GetStatus() != 1;
    }
}

// Reconstructed from eboot.elf at 0xA39750.
int GameMic::GetDataSampleRate() {
    if (mInputSampleRate != 0) {
        return mInputSampleRate;
    }
    if (mMicID == -1) {
        return kDefaultSampleRate;
    }
    return GetMyMic()->GetSampleRate();
}

// Reconstructed from eboot.elf at 0xA39790. A peak above 3000 lowers a
// strong block's gain toward 3000 / peak and raises a weak one's by a tenth.
void GameMic::NormalizingCopy(short* out, short* in, int count, float& gain) {
    if (count <= 0) {
        return;
    }
    float peak = 0.0F;
    for (int i = 0; i < count; i++) {
        const float sample = in[i] * gain;
        out[i] = static_cast<short>(sample);
        peak = sample > peak ? sample : peak;
    }
    if (peak > 3000.0F) {
        if (peak * 2.0 >= 30000.0) {
            gain = gain * 0.9F + 3000.0F / peak;
        } else {
            gain = gain * 1.1F;
        }
        gain = gain < 2.0F ? gain : 2.0F;
    }
}
