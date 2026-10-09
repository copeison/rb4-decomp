#include "audio/core/output/AudioRenderTarget.h"
#include "os/threading/CritSec.h"

// Reconstructed from eboot.elf at 0x1127B90.
void AudioMixer::SetSampleRate(int sampleRate) {
    mSampleRate = sampleRate;
}

// Reconstructed from eboot.elf at 0x1127880. Callables queued since the last
// mix are promoted first. Every block is then prepared on all callables, and
// each callable makes its samples at most once per block.
void AudioMixer::Mix(float, unsigned int numSamples, unsigned int mixCount, bool) {
    ScopedCritSecPtr tracker(&mCritSec);

    {
        ScopedCritSecPtr pendingTracker(&mPendingCritSec);
        while (!mPendingCallables.Empty()) {
            auto* callable = mPendingCallables.PopFront();
            mCallables.PushBack(*callable);
        }
    }

    auto* const sentinel = mCallables.Sentinel();
    int block = 0;
    while (numSamples != 0) {
        const bool lastBlock = numSamples == kBlockSamples;
        const float sampleRate = static_cast<float>(mSampleRate);
        for (auto* node = mCallables.mNext; node != sentinel; node = node->mNext) {
            CallableList::Owner(node)->_PrepareToMakeSamples(
                kBlockSamples, sampleRate, mixCount, block, lastBlock);
        }

        mMixCount = mixCount;
        mBlock = block;
        mLastBlock = lastBlock;

        for (auto* node = mCallables.mNext; node != sentinel; node = node->mNext) {
            CallableList::Owner(node)->mMakeGuard.store(0);
        }
        for (auto* node = mCallables.mNext; node != sentinel; node = node->mNext) {
            auto* callable = CallableList::Owner(node);
            if (callable->mMakeGuard.load() <= 0 &&
                callable->mMakeGuard.fetch_add(1) == 0) {
                callable->_MakeSamples(
                    kBlockSamples,
                    static_cast<float>(mSampleRate),
                    mMixCount,
                    mBlock,
                    mLastBlock);
            }
        }

        ++block;
        numSamples -= kBlockSamples;
    }
}
