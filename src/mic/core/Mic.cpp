#include "mic/core/Mic.h"

namespace {

// The detector level is scaled into [0, 1] and smoothed, following a rise
// faster than a fall.
constexpr float kLevelScale = 0.002F;
constexpr float kLevelFallWeight = 0.1F;
constexpr float kLevelRiseWeight = 0.3F;

}  // namespace

// Reconstructed from eboot.elf at 0xE1530. The rings free their samples.
Mic::~Mic() {}

// Reconstructed from eboot.elf at 0xE15F0.
void Mic::ResetBuffersAndStart() {
    mRecentBuffer.Reset();
    mContinuousBuffer.Reset();
    Start();
}

// Reconstructed from eboot.elf at 0xE1650.
void Mic::ResetBuffersAndStartPlayback(const PlayArgs& args) {
    mRecentBuffer.Reset();
    mContinuousBuffer.Reset();
    StartPlayback(args);
}

// Reconstructed from eboot.elf at 0xE16B0.
void Mic::_HandleMicRemoval() {
    Stop();
}

// Reconstructed from eboot.elf at 0xE16C0.
int Mic::GetRecentSamples(short* samples, int count) {
    int size = mRecentBuffer.mSize;
    int start = (mRecentBuffer.mWritePos - count + size) % size;
    int tail = size - start;
    int first = tail > count ? count : tail;
    std::memcpy(samples, mRecentBuffer.mBuffer + start, sizeof(short) * first);
    if (tail < count) {
        std::memcpy(samples + first, mRecentBuffer.mBuffer, sizeof(short) * (count - first));
    }
    return count;
}

// Reconstructed from eboot.elf at 0xE1740.
int Mic::ReadContinuousSamples(short* samples, int count) {
    int available = 0;
    int pending = mContinuousBuffer.mWritePos - mContinuousBuffer.mReadPos;
    if (pending != 0) {
        available = pending > 0 ? pending : pending + mContinuousBuffer.mSize;
    }
    if (available <= count) {
        count = available;
    }
    if (count == 0) {
        return 0;
    }
    int tail = mContinuousBuffer.mSize - mContinuousBuffer.mReadPos;
    int first = count <= tail ? count : tail;
    std::memcpy(samples, mContinuousBuffer.mBuffer + mContinuousBuffer.mReadPos, sizeof(short) * first);
    if (count > tail) {
        std::memcpy(samples + first, mContinuousBuffer.mBuffer, sizeof(short) * (count - first));
    }
    mContinuousBuffer.mReadPos = (mContinuousBuffer.mReadPos + count) % mContinuousBuffer.mSize;
    return count;
}

// Reconstructed from eboot.elf at 0xE17F0.
void Mic::SkipContinuousSamples() {
    mContinuousBuffer.mReadPos = mContinuousBuffer.mWritePos;
}

// Reconstructed from eboot.elf at 0xE1800.
int Mic::GetOverflowCount() const {
    return mOverflowCount;
}

// Reconstructed from eboot.elf at 0xE1810.
void Mic::Poll() {
    _Poll();
    if (GetStatus() != 0 && mAnalysisEnabled) {
        _AnalyzeRecentSamples();
    }
}

// Reconstructed from eboot.elf at 0xE1850.
void Mic::_AnalyzeRecentSamples() {
    mPitchDetector.mComputePitch = true;
    if (mPitchDetector.mSampleRate != mSampleRate) {
        mPitchDetector.SetSampleRate(mSampleRate);
    }
    GetRecentSamples(mAnalysisSamples, kAnalysisSize);
    float pitch = 0.0F;
    float level = 0.0F;
    float energy = 0.0F;
    mPitchDetector.AnalyzeBlock(
        GetName().Str(), mAnalysisSamples, kAnalysisSize, mDetectorSensitivity, mDetectorGain, pitch, energy,
        level, mInputPeak);
    float scaled = energy * kLevelScale;
    float clamped = scaled > 1.0F ? 1.0F : (0.0F > scaled ? 0.0F : scaled);
    float weight = clamped > mLevel ? kLevelRiseWeight : kLevelFallWeight;
    mLevel = (1.0F - weight) * mLevel + weight * clamped;
    mWindowLevel = level;
    mPitch = pitch;
}

// Reconstructed from eboot.elf at 0xE1A00.
int Mic::GetStatus() const {
    return 0;
}

// Reconstructed from eboot.elf at 0xE1A10.
int Mic::GetType() const {
    return 0;
}

// Reconstructed from eboot.elf at 0xE1A20.
void Mic::StopPlayback() {}

// Reconstructed from eboot.elf at 0xE1A30.
int Mic::GetDroppedSamples() {
    return 0;
}

// Reconstructed from eboot.elf at 0xE1A40.
void Mic::MicThreadPoll() {}

// Reconstructed from eboot.elf at 0xE1A50.
float Mic::GetSensitivity() const {
    return 0.0F;
}

// Reconstructed from eboot.elf at 0xE1A60. The name is interned on first
// use.
Symbol Mic::GetName() const {
    static Symbol generic_usb;
    if (generic_usb == Symbol()) {
        generic_usb = Symbol("generic_usb");
    }
    return generic_usb;
}

// Reconstructed from eboot.elf at 0xE1B00.
void Mic::StartPlayback(const PlayArgs&) {}
