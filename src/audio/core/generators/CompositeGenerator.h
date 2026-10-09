#pragma once

#include <cstddef>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/output/AudioRenderTarget.h"

// Generator that groups the voices of one sound (audio/CompositeGenerator.o).
// Only the members the sound manager uses are declared. Children are linked
// through their pool node.
class CompositeGenerator : public AudioGenerator {
public:
    // Adds the child, to the pending list when another thread holds the
    // kill lock. At 0x41160.
    void AddGenerator(AudioGenerator* generator);
    // Whether the generator has children. At 0x41B40.
    bool IsPlaying() const;
    // Moves the pending children to the child list. At 0x40C20.
    void _AddPendingChildren();

    // Takes the other generator's children under the kill lock, clearing
    // their emitter. The map emits it out of line in SoundManager.o; this
    // build inlines it into 0x7E90.
    void _TakeOwnershipOfChildren(CompositeGenerator& other) {
        ScopedCritSec lock(gGeneratorKillCritSec);
        other._AddPendingChildren();
        while (!other.mChildren.Empty()) {
            AudioGenerator* child = other.mChildren.PopFront();
            child->mEmitter = nullptr;
            AddGenerator(child);
        }
    }

    // Field names are not in the reference map.
    // The generator state between the base and the lists; not
    // reconstructed.
    unsigned char mCompositeState[24];
    AudioGeneratorManager::GeneratorList mChildren;
    // Children added while another thread held the kill lock.
    AudioGeneratorManager::GeneratorList mPendingChildren;
};

static_assert(offsetof(CompositeGenerator, mChildren) == 104);
static_assert(offsetof(CompositeGenerator, mPendingChildren) == 128);

// Pool of CompositeGenerators that the sound manager creates with 256
// entries. The map emits every member in SoundManager.o; this build keeps
// them at 0xDD10 through 0xE3C0. The vtable is at 0x18DCCB8.
class CompositeGeneratorManager : public AudioGeneratorManager {
public:
    // Inlined into SoundManager::_InitCompositeGenMgr (0x1360).
    explicit CompositeGeneratorManager(int poolSize) {
        mPoolSize = poolSize;
    }

    AudioGenerator* Play(const PlayArgs& args) override;  // slot 0: 0xDD10
    int GetIndex() override;               // slot 6: 0xDD30
    Symbol GetId() override;               // slot 7: 0xDD40
    Symbol GetResourceExt() override;      // slot 8: 0xDDE0
    AudioGenerator* LockIfOwned(unsigned int handle, int index) override;  // slot 9: 0xDE80
    void SendStopToAllGenerators() override;  // slot 10: 0xDF00
    void SendKillToAllGenerators() override;  // slot 11: 0xDF70
    void GetActiveHandles(void* handles) override;  // slot 12: 0xDFE0
    void _SetManagerIndex(int index) override;      // slot 13: 0xE130
    void _InitGeneratorPool() override;    // slot 14: 0xE140
    bool _DeleteGeneratorPool() override;  // slot 15: 0xE300
    ~CompositeGeneratorManager() override;  // slots 16-17: 0xE3B0, 0xE3C0

    // Takes an idle generator for the emitter, bound to the default render
    // target. The map's AudioGeneratorManager::_AllocateGenerator(
    // AudioEmitterCom*), emitted in SoundManager.o; this build inlines it.
    CompositeGenerator* _AllocateGenerator(AudioEmitterCom* emitter) {
        AudioRenderTarget* target = gAudioRenderTargets.mDefault;
        ScopedCritSec lock(mCritSec);
        if (mFreeList.mSize == 0) {
            return nullptr;
        }
        auto* generator = static_cast<CompositeGenerator*>(mFreeList.PopFront());
        generator->mEmitter = emitter;
        generator->mRenderTarget = target;
        generator->GetNewHandle();
        return generator;
    }

    CompositeGenerator* mPool;  // Name not in the reference map.
};

static_assert(offsetof(CompositeGeneratorManager, mPool) == 64);
static_assert(sizeof(CompositeGeneratorManager) == 72);
