#include "audio/fmod/playback/FmodBufferedStreamGenerator.h"

#include "audio/fmod/playback/FmodAudioBusGenerator.h"
#include "audio/fmod/playback/FmodGeneratorPool.h"
#include "audio/fmod/resources/FmodAudioStreamResource.h"

namespace {

// Requests in this format with streaming set use the buffered path. Name not
// in the reference map.
constexpr int kBufferedStreamFormat = 3;

}  // namespace

Symbol FmodBufferedStreamGenerator::sTypeId("");

// Only the fields named here are reconstructed; the inlined pool constructor
// at 0x26E000 also sets up the channel buffers and the sync state.
FmodBufferedStreamGenerator::FmodBufferedStreamGenerator() : mBusGenerator(nullptr) {}

// Reconstructed from eboot.elf at 0x26AFF0. The bus side renders stereo in
// 128-sample blocks at the engine sample rate.
void FmodBufferedStreamGenerator::Init(AudioGeneratorManager* manager, int index) {
    _InitTypeId();
    mManager = manager;
    mIndex = index;
    mBusGenerator = nullptr;
    AudioBus::Prepare(static_cast<float>(Audio::GetSamplesPerSecond()), 2, 128, false);
}

// Reconstructed from eboot.elf at 0x26BD40.
void FmodBufferedStreamGenerator::Pause() {
    mBusGenerator->Pause();
    mState = kStatePaused;
}

// Reconstructed from eboot.elf at 0x26BD70.
void FmodBufferedStreamGenerator::Continue() {
    mBusGenerator->Continue();
    mState = kStatePlaying;
}

// Reconstructed from eboot.elf at 0x26C150.
void FmodBufferedStreamGenerator::Stop() {
    mBusGenerator->Stop();
    mState = kStateStopping;
}

// Reconstructed from eboot.elf at 0x26BDA0.
float FmodBufferedStreamGenerator::GetElapsedMs() {
    return 0.0F;
}

// Reconstructed from eboot.elf at 0x26BDB0.
float FmodBufferedStreamGenerator::GetTimelineMs() {
    return mTimelineMs;
}

// Reconstructed from eboot.elf at 0x26C040. The render thread reads the
// speed under the bus lock.
void FmodBufferedStreamGenerator::SetSpeed(float speed, bool) {
    LockBus();
    mSpeed = speed;
    UnlockBus();
}

// Reconstructed from eboot.elf at 0x26C080.
float FmodBufferedStreamGenerator::GetSpeed(bool* changing) {
    if (changing != nullptr) {
        *changing = false;
    }
    return mSpeed;
}

// Reconstructed from eboot.elf at 0x26E320.
void FmodBufferedStreamGenerator::SetGain(float gain, float fadeSecs, PostFadeOption option) {
    if (mBusGenerator != nullptr) {
        mBusGenerator->SetGain(gain, fadeSecs, option);
    }
}

// Reconstructed from eboot.elf at 0x26E340.
float FmodBufferedStreamGenerator::GetGain() const {
    return mBusGenerator != nullptr ? mBusGenerator->GetGain() : 0.0F;
}

// Reconstructed from eboot.elf at 0x26E360.
void FmodBufferedStreamGenerator::SetMute(bool mute, bool immediate) {
    if (mBusGenerator != nullptr) {
        mBusGenerator->SetMute(mute, immediate);
    }
}

// Reconstructed from eboot.elf at 0x26E380.
bool FmodBufferedStreamGenerator::GetMute() const {
    return mBusGenerator != nullptr && mBusGenerator->GetMute();
}

// Reconstructed from eboot.elf at 0x26C180.
void FmodBufferedStreamGenerator::Kill() {
    mBusGenerator->KillLocked();
    mState = kStateStopped;
}

// Reconstructed from eboot.elf at 0x26E3A0.
void FmodBufferedStreamGenerator::_InitTypeId() {
    sTypeId = Symbol("FmodBufferedStreamGenerator");
}

// Reconstructed from eboot.elf at 0x26E3F0.
AudioGenerator* FmodBufferedStreamGenerator::GetGeneratorOfType(Symbol type) {
    return type == sTypeId ? this : nullptr;
}

// Reconstructed from eboot.elf at 0x26E410.
Symbol FmodBufferedStreamGenerator::GetTypeId() {
    return sTypeId;
}

// Reconstructed from eboot.elf at 0x26D940. The vectorized original computes
// the same polynomial from float-rounded coefficients.
float FmodBufferedStreamGenerator::_InterpolateOptimal(
    const float* samples, float fraction) const {
    const float z = fraction - 0.5F;
    const float even1 = samples[3] + samples[2];
    const float odd1 = samples[3] - samples[2];
    const float even2 = samples[4] + samples[1];
    const float odd2 = samples[4] - samples[1];
    const float even3 = samples[5] + samples[0];
    const float odd3 = samples[5] - samples[0];

    const float c0 = even1 * 0.426859825850F + even2 * 0.0723812356591F +
        even3 * 0.000758930807933F;
    const float c1 = odd1 * 0.358317732811F + odd2 * 0.204516440630F +
        odd3 * 0.00562658812851F;
    const float c2 = even1 * -0.217009171844F + even2 * 0.200513765216F +
        even3 * 0.0164954103529F;
    const float c3 = odd1 * -0.251127153635F + odd2 * 0.0422302596271F +
        odd3 * 0.0248872749507F;
    const float c4 = even1 * 0.0416694656014F + even2 * -0.0625042021275F +
        even3 * 0.0208347346634F;
    const float c5 = odd1 * 0.0834979936481F + odd2 * -0.0417491272092F +
        odd3 * 0.00834987871349F;
    return ((((c5 * z + c4) * z + c3) * z + c2) * z + c1) * z + c0;
}

// Reconstructed from eboot.elf at 0x26AB60. Only streaming requests in the
// buffered format are accepted.
AudioGenerator* FmodBufferedStreamGeneratorManager::Play(const PlayArgs& args) {
    ResourcePtr<FmodAudioStreamResource> resource = FmodAudioStreamResource::Find(args.mName);
    if (!resource || resource->Fail()) {
        return nullptr;
    }
    if (args.mFormat != kBufferedStreamFormat || !args.mStreaming) {
        return nullptr;
    }
    return _AllocateAndSetUpGenerator(resource, args);
}

// Reconstructed from eboot.elf at 0x26AC30.
FmodBufferedStreamGenerator* FmodBufferedStreamGeneratorManager::_AllocateAndSetUpGenerator(
    ResourcePtr<FmodAudioStreamResource> resource, const PlayArgs& args) {
    AudioEmitterCom* emitter =
        args.mEmitter != nullptr ? args.mEmitter : gSoundManager.GetDefault2DEmitter();
    AudioRenderTarget* target = gAudioRenderTargets.Find(args.mRenderTarget, true);
    auto* generator =
        FmodGeneratorPool::Allocate<FmodBufferedStreamGenerator>(*this, target, emitter);
    if (generator == nullptr) {
        return nullptr;
    }
    return generator->Setup(resource, args) ? generator : nullptr;
}

// Reconstructed from eboot.elf at 0x26AAC0.
void FmodBufferedStreamGeneratorManager::Init() {
    AudioGeneratorManager::Init();
    mReaderThread.StartAsyncPoll();
}

// Reconstructed from eboot.elf at 0x26DC00.
int FmodBufferedStreamGeneratorManager::GetIndex() {
    return mManagerIndex;
}

// Reconstructed from eboot.elf at 0x26DC10.
Symbol FmodBufferedStreamGeneratorManager::GetId() {
    static Symbol sId("");
    if (sId == Symbol("")) {
        sId = Symbol("FmodBufferedStreamGeneratorManager");
    }
    return sId;
}

// Reconstructed from eboot.elf at 0x26DCB0.
Symbol FmodBufferedStreamGeneratorManager::GetResourceExt() {
    static Symbol sExt("");
    if (sExt == Symbol("")) {
        sExt = Symbol(".mp3");
    }
    return sExt;
}

// Reconstructed from eboot.elf at 0x26DD50.
AudioGenerator* FmodBufferedStreamGeneratorManager::LockIfOwned(unsigned int handle, int index) {
    return FmodGeneratorPool::LockIfOwned(*this, mPool, handle, index);
}

// Reconstructed from eboot.elf at 0x26DDC0.
void FmodBufferedStreamGeneratorManager::SendStopToAllGenerators() {
    FmodGeneratorPool::SendStop(*this, mPool);
}

// Reconstructed from eboot.elf at 0x26DE30.
void FmodBufferedStreamGeneratorManager::SendKillToAllGenerators() {
    FmodGeneratorPool::SendKill(*this, mPool);
}

// Reconstructed from eboot.elf at 0x26DEA0.
void FmodBufferedStreamGeneratorManager::GetActiveHandles(void* handles) {
    FmodGeneratorPool::GetActiveHandles(*this, mPool, handles);
}

// Reconstructed from eboot.elf at 0x26DFF0.
void FmodBufferedStreamGeneratorManager::_SetManagerIndex(int index) {
    mManagerIndex = index;
}

// Reconstructed from eboot.elf at 0x26E000.
void FmodBufferedStreamGeneratorManager::_InitGeneratorPool() {
    FmodGeneratorPool::Init(*this, mPool);
}

// Reconstructed from eboot.elf at 0x26E270.
bool FmodBufferedStreamGeneratorManager::_DeleteGeneratorPool() {
    return FmodGeneratorPool::Delete(*this, mPool);
}

// Reconstructed from eboot.elf at 0x26AAE0.
FmodBufferedStreamGeneratorManager::~FmodBufferedStreamGeneratorManager() {
    mReaderThread.QuitAsyncPoll();
}
