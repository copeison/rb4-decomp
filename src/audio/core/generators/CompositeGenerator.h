#pragma once

#include <cstddef>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/output/AudioRenderTarget.h"

// Generator that groups the voices of one sound (audio/CompositeGenerator.o,
// 0x40AF0 to 0x41E6B). Every emitter keeps one as the parent of its sounds.
// The playback calls forward to the children, which are linked through
// their pool node. A child added while another thread holds the kill lock
// waits in the pending list until the next call that takes the lock. The
// vtable is at 0x18DFFB8; the object is 160 bytes.
class CompositeGenerator : public AudioGenerator {
public:
    // Inlined into CompositeGeneratorManager::_InitGeneratorPool at 0xE140.
    CompositeGenerator() : mGain(1.0F), mMute(false) {}
    // Slots 20-21 at 0x41D90 and 0x41DA0; 0x41D90 jumps to the body at
    // 0xE600. The map emits it inline.
    ~CompositeGenerator() override {}

    void Pause() override;                            // slot 0: 0x40AF0
    void Continue() override;                         // slot 1: 0x40CE0
    // Asks every child to stop. At 0x40E10.
    void Stop() override;                             // slot 2
    // The longest child's elapsed time. At 0x41210.
    float GetElapsedMs() override;                    // slot 4
    // The binary reads the children's elapsed time here as well. At
    // 0x41350.
    float GetTimelineMs() override;                   // slot 5
    void SeekToMs(float ms) override;                 // slot 7: 0x41490
    // Whether any child took the parameter. At 0x415C0.
    bool SetParameter(Symbol name, float value) override;  // slot 10
    // The first child's value that is found. At 0x41700.
    bool GetParameter(Symbol name, float& value) override;  // slot 11
    void SetGain(float gain, float fadeSecs, PostFadeOption option) override;  // slot 12: 0x41840
    // The first child's gain, or the last gain set without children. At
    // 0x41980.
    float GetGain() const override;                   // slot 13
    // The map has SetMute(bool). At 0x419F0.
    void SetMute(bool mute, bool immediate) override;  // slot 14
    // Slot 15 at 0x41D80. The map emits it inline.
    bool GetMute() const override {
        return mMute;
    }
    // Releases the children that finished and stops once none is left. At
    // 0x41B50.
    bool Poll() override;                             // slot 22
    // Returns the generator to its pool: the map's inline
    // AudioGeneratorManager::FreeGenerator(AudioGenerator*), under the kill
    // lock. At 0x41CE0.
    void Release() override;                          // slot 23
    // Slot 28 at 0x41DC0. The map emits it inline.
    void _InitTypeId() override {
        kTypeId = Symbol("CompositeGenerator");
    }
    // Kills every child. A child still locked by a caller moves to the
    // default emitter's composite generator unless this one belongs to that
    // emitter. At 0x40F40.
    void Kill() override;                             // slot 29
    // Slot 30 at 0x41E10: the first child that is of the type, or holds
    // one. The map emits it inline.
    AudioGenerator* GetGeneratorOfType(Symbol type) override {
        for (LinkedListSizeTracked::Node* node = mChildren.mNext; node != mChildren.Sentinel();
             node = node->mNext) {
            AudioGenerator* found = AudioGeneratorManager::GeneratorList::Owner(node)
                                        ->GetGeneratorOfType(type);
            if (found != nullptr) {
                return found;
            }
        }
        return nullptr;
    }
    Symbol GetTypeId() override;                      // slot 31: 0x41E60

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

    // At 0x19C8480.
    static Symbol kTypeId;
    // Guards the pending list. At 0x19C8498.
    static CritSec mCompositeGenChildListLock;

    // Field names are not in the reference map.
    // A second list node, constructed and unlinked with the generator; no
    // list that holds it was found.
    LinkedListSizeTracked::Node mCompositeNode;
    AudioGeneratorManager::GeneratorList mChildren;
    // Children added while another thread held the kill lock.
    AudioGeneratorManager::GeneratorList mPendingChildren;
    float mGain;  // The last SetGain.
    bool mMute;   // The last SetMute.
};

static_assert(offsetof(CompositeGenerator, mCompositeNode) == 80);
static_assert(offsetof(CompositeGenerator, mChildren) == 104);
static_assert(offsetof(CompositeGenerator, mPendingChildren) == 128);
static_assert(offsetof(CompositeGenerator, mGain) == 152);
static_assert(offsetof(CompositeGenerator, mMute) == 156);
static_assert(sizeof(CompositeGenerator) == 160);

// Pool of CompositeGenerators that the sound manager creates with 256
// entries. The map emits every member in SoundManager.o; this build keeps
// them at 0xDD10 through 0xE3C0. The vtable is at 0x18DCCB8.
class CompositeGeneratorManager : public AudioGeneratorManager {
public:
    // Inlined into SoundManager::_InitCompositeGenMgr (0x1360).
    explicit CompositeGeneratorManager(int poolSize) {
        mPoolSize = poolSize;
    }

    // Composite generators are only allocated directly. At 0xDD10.
    AudioGenerator* Play(const PlayArgs& args) override;  // slot 0
    int GetIndex() override;               // slot 6: 0xDD30
    Symbol GetId() override;               // slot 7: 0xDD40
    Symbol GetResourceExt() override;      // slot 8: 0xDDE0, ".--none--"
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
    // AudioEmitter*), emitted in SoundManager.o; this build inlines it.
    CompositeGenerator* _AllocateGenerator(AudioEmitter* emitter) {
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

    // "CompositeGeneratorManager", at 0x19B0098; GetId builds its symbol
    // from a copy of the string.
    static const char* kIdStr;

    CompositeGenerator* mPool;  // Name not in the reference map.
};

static_assert(offsetof(CompositeGeneratorManager, mPool) == 64);
static_assert(sizeof(CompositeGeneratorManager) == 72);
