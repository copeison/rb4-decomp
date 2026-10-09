#pragma once

#include <atomic>
#include <cstddef>

#include "audio/core/containers/LinkedListSizeTracked.h"
#include "audio/core/system/Audio.h"
#include "os/threading/CritSec.h"

// Client of an AudioMixer. Each 128-sample block is prepared on every
// callable and then made once per mix. The map emits the class's inline
// members in the FmodAudioBusGenerator object. The vtable is at 0x18E0A98;
// the object is 40 bytes.
class AudioBusCallable {
public:
    virtual ~AudioBusCallable();  // slots 0-1: 0x476C0, 0x47730
    // Slot 2.
    virtual bool _PrepareToMakeSamples(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) = 0;
    // Slot 3 at 0x47650.
    virtual bool _MakeSamples(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock);
    // Slot 4 at 0x47660. Name not in the reference map.
    virtual void Unknown4();

    // Claimed by the first caller of _MakeSamples in each block. Name not in
    // the reference map.
    std::atomic<int> mMakeGuard;
    LinkedListSizeTracked::Node _mCallbackNode;
};

static_assert(offsetof(AudioBusCallable, mMakeGuard) == 8);
static_assert(offsetof(AudioBusCallable, _mCallbackNode) == 16);
static_assert(sizeof(AudioBusCallable) == 40);

// Audio source rendered by an AudioBusGenerator, such as an
// FmodBufferedStreamGenerator. The vtable is at 0x18E5A78 and the
// constructor at 0xBEF90; the object is 192 bytes. Its methods have not been
// reconstructed.
template <class T>
class AudioBuffer;

class AudioBus {
public:
    AudioBus();
    virtual ~AudioBus();  // slots 0-1
    // Slot 2 at 0xBF210.
    virtual void Prepare(float sampleRate, unsigned int numChannels, unsigned int blockSize, bool unknown);
    virtual void Unknown3();  // slot 3: 0xBF2B0. Name not in the reference map.
    virtual bool Process(AudioBuffer<float>& buffer);  // slot 4
    // Slots 5-9. The map lists these five inline members; slot 5 and slot 7
    // are confirmed as the lock and unlock, the order of the others is
    // inferred.
    virtual void LockBus();     // slot 5: 0x43BA0
    virtual bool TryLockBus();  // slot 6: 0x43BD0
    virtual void UnlockBus();   // slot 7: 0x43C00
    virtual CritSec* GetBusLock();  // slot 8: 0x43C20
    virtual void TearDown();    // slot 9: 0x43C30
    virtual void Unknown10();   // slot 10: 0x52330. Name not in the reference map.

    // Field names are not in the reference map.
    LinkedListSizeTracked::ListBase mUnknown8;
    unsigned char mUnknown32[112];
    int mBlockSize;      // Samples per rendered block.
    double mSampleRate;
    unsigned char mUnknown160[16];
    CritSec mBusLock;
};

static_assert(offsetof(AudioBus, mBlockSize) == 144);
static_assert(offsetof(AudioBus, mSampleRate) == 152);
static_assert(offsetof(AudioBus, mBusLock) == 176);
static_assert(sizeof(AudioBus) == 192);
