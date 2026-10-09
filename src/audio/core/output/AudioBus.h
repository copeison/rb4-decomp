#pragma once

#include <atomic>
#include <cstddef>
#include <kernel.h>

#include "audio/core/buffers/AudioBuffer.h"
#include "audio/core/containers/LinkedListSizeTracked.h"
#include "audio/core/system/Audio.h"
#include "os/threading/CritSec.h"

// Client of an AudioMixer. Each 128-sample block is prepared on every
// callable and then made once per mix. The map emits the class's inline
// members in the FmodAudioBusGenerator object; this build keeps them out of
// line at 0x47650 through 0x47730. The vtable is at 0x18E0A98; the object is
// 40 bytes.
class AudioBusCallable {
public:
    // Inlined into AudioBusGenerator's constructor at 0xE0490.
    AudioBusCallable() {
        mMakeGuard = 0;
    }
    virtual ~AudioBusCallable();  // slots 0-1: 0x476C0, 0x47730
    // Slot 2.
    virtual bool _PrepareToMakeSamples(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) = 0;
    // Slot 3 at 0x47650.
    virtual bool _MakeSamples(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock);
    // Slot 4 at 0x47660: whether the callable renders a virtual
    // instrument. Only AudioBusGenerator overrides it, forwarding
    // AudioBus::IsVirtualInstrument of its source; nothing in this build
    // calls it. Name not in the reference map.
    virtual bool IsVirtualInstrument();

    // Claimed by the first caller of _MakeSamples in each block. Name not in
    // the reference map.
    std::atomic<int> mMakeGuard;
    LinkedListSizeTracked::Node _mCallbackNode;
};

static_assert(offsetof(AudioBusCallable, mMakeGuard) == 8);
static_assert(offsetof(AudioBusCallable, _mCallbackNode) == 16);
static_assert(sizeof(AudioBusCallable) == 40);

class AudioBus;

// Object told when an AudioBus it holds is destroyed. No implementation is
// identified in this build; slots 0-1 are taken to be the destructor pair
// that Itanium places first, which is weak evidence. Name not in the
// reference map.
class AudioBusOwner {
public:
    virtual ~AudioBusOwner();
    // Slot 2: called from ~AudioBus at 0xBF0A0 with the dying bus.
    virtual void OnBusDestroyed(AudioBus* bus);
};

// Audio source rendered by an AudioBusGenerator, such as an
// FmodBufferedStreamGenerator (audio/AudioBus.o). The vtable is at 0x18E5A78;
// the object is 192 bytes.
class AudioBus {
public:
    AudioBus();            // 0xBEF90
    virtual ~AudioBus();   // slots 0-1: 0xBF0A0, 0xBF1F0
    // Slot 2 at 0xBF210. Allocates the bus buffer only when the block size
    // is nonzero and allocate is set.
    virtual void Prepare(float sampleRate, unsigned int numChannels, unsigned int blockSize, bool allocate);
    // Slot 3 at 0xBF2B0. Name not in the reference map.
    virtual void SetSampleRate(float sampleRate);
    // Slot 4. The map defines AudioBus::Process(AudioBuffer<float>&); in this
    // build the slot is pure.
    virtual bool Process(AudioBuffer<float>& buffer) = 0;
    // Slots 5-9. The map emits these five inline members in the
    // FusionGenerator object.
    virtual void LockBus() {  // slot 5: 0x43BA0
        mBusLock.Enter();
    }
    // Slot 6 at 0x43BD0. Only a busy mutex counts as failure.
    virtual bool TryLockBus() {
        if (scePthreadMutexTrylock(&mBusLock.mCritSec) == SCE_KERNEL_ERROR_EBUSY) {
            return false;
        }
        ++mBusLock.mEntryCount;
        return true;
    }
    virtual void UnlockBus() {  // slot 7: 0x43C00
        mBusLock.Exit();
    }
    virtual CritSec* GetBusLock() {  // slot 8: 0x43C20
        return &mBusLock;
    }
    virtual void TearDown() {}  // slot 9: 0x43C30
    // Slot 10 at 0x52330: false here; the instrument buses, among them
    // FusionSampler (0x43C40 for the vtable at 0x18E4D68), return true.
    // Name not in the reference map.
    virtual bool IsVirtualInstrument() {
        return false;
    }

    // Field names are not in the reference map.
    AudioBuffer<float> mBuffer;
    // Zeroed by the constructor and Prepare and read nowhere in this build;
    // the names record only that. The evidence is weak.
    int mPrepareResetA;
    int mPrepareResetB;
    int mBlockSize;      // Samples per rendered block.
    int mNumChannels;
    double mSampleRate;
    double mSecondsPerSample;
    AudioBusOwner* mOwner;
    CritSec mBusLock;
};

static_assert(offsetof(AudioBus, mBuffer) == 8);
static_assert(offsetof(AudioBus, mPrepareResetA) == 136);
static_assert(offsetof(AudioBus, mBlockSize) == 144);
static_assert(offsetof(AudioBus, mNumChannels) == 148);
static_assert(offsetof(AudioBus, mSampleRate) == 152);
static_assert(offsetof(AudioBus, mSecondsPerSample) == 160);
static_assert(offsetof(AudioBus, mOwner) == 168);
static_assert(offsetof(AudioBus, mBusLock) == 176);
static_assert(sizeof(AudioBus) == 192);
