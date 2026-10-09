#include "audio/core/generators/CompositeGenerator.h"

#include <kernel.h>

#include "audio/core/system/SoundManager.h"

Symbol CompositeGenerator::kTypeId;
CritSec CompositeGenerator::mCompositeGenChildListLock;
const char* CompositeGeneratorManager::kIdStr = "CompositeGeneratorManager";

namespace {

using ChildList = AudioGeneratorManager::GeneratorList;

// The child that owns a list node. Name not in the reference map.
AudioGenerator* Child(LinkedListSizeTracked::Node* node) {
    return ChildList::Owner(node);
}

}  // namespace

// Reconstructed from eboot.elf at 0x40AF0.
void CompositeGenerator::Pause() {
    ScopedCritSec lock(gGeneratorKillCritSec);
    _AddPendingChildren();
    for (LinkedListSizeTracked::Node* node = mChildren.mNext; node != mChildren.Sentinel();
         node = node->mNext) {
        Child(node)->Pause();
    }
    mState = kStatePaused;
}

// Reconstructed from eboot.elf at 0x40C20.
void CompositeGenerator::_AddPendingChildren() {
    if (mPendingChildren.mSize == 0) {
        return;
    }
    ScopedCritSec lock(mCompositeGenChildListLock);
    for (unsigned long count = mPendingChildren.mSize; count != 0; --count) {
        mChildren.PushBack(*mPendingChildren.PopFront());
    }
}

// Reconstructed from eboot.elf at 0x40CE0.
void CompositeGenerator::Continue() {
    ScopedCritSec lock(gGeneratorKillCritSec);
    _AddPendingChildren();
    for (LinkedListSizeTracked::Node* node = mChildren.mNext; node != mChildren.Sentinel();
         node = node->mNext) {
        Child(node)->Continue();
    }
    mState = kStatePlaying;
}

// Reconstructed from eboot.elf at 0x40E10.
void CompositeGenerator::Stop() {
    ScopedCritSec lock(gGeneratorKillCritSec);
    _AddPendingChildren();
    for (LinkedListSizeTracked::Node* node = mChildren.mNext; node != mChildren.Sentinel();
         node = node->mNext) {
        Child(node)->Stop();
    }
    mState = kStateStopping;
}

// Reconstructed from eboot.elf at 0x40F40. KillLocked calls it with the kill
// lock held.
void CompositeGenerator::Kill() {
    _AddPendingChildren();
    LinkedListSizeTracked::Node* node = mChildren.mNext;
    while (node != mChildren.Sentinel()) {
        AudioGenerator* child = Child(node);
        child->KillLocked();
        node = node->mNext;
        mChildren.Remove(*child);
        if (child->TryDeactivateHandle()) {
            child->Release();
        } else if (mEmitter != theSoundManager.GetDefault2DEmitter()) {
            child->mEmitter = nullptr;
            static_cast<CompositeGenerator*>(
                theSoundManager.GetDefault2DEmitter()->GetCompositeGenerator())
                ->AddGenerator(child);
        }
    }
    mState = kStateStopped;
}

// Reconstructed from eboot.elf at 0x41160. The kill lock is only tried, so
// that a child started from a stopping generator's callback does not wait.
void CompositeGenerator::AddGenerator(AudioGenerator* generator) {
    if (scePthreadMutexTrylock(&gGeneratorKillCritSec.mCritSec) == SCE_KERNEL_ERROR_EBUSY) {
        ScopedCritSec lock(mCompositeGenChildListLock);
        mPendingChildren.PushBack(*generator);
    } else {
        mChildren.PushBack(*generator);
        scePthreadMutexUnlock(&gGeneratorKillCritSec.mCritSec);
    }
}

// Reconstructed from eboot.elf at 0x41210.
float CompositeGenerator::GetElapsedMs() {
    ScopedCritSec lock(gGeneratorKillCritSec);
    _AddPendingChildren();
    float elapsed = 0.0F;
    for (LinkedListSizeTracked::Node* node = mChildren.mNext; node != mChildren.Sentinel();
         node = node->mNext) {
        float childElapsed = Child(node)->GetElapsedMs();
        elapsed = childElapsed > elapsed ? childElapsed : elapsed;
    }
    return elapsed;
}

// Reconstructed from eboot.elf at 0x41350.
float CompositeGenerator::GetTimelineMs() {
    ScopedCritSec lock(gGeneratorKillCritSec);
    _AddPendingChildren();
    float timeline = 0.0F;
    for (LinkedListSizeTracked::Node* node = mChildren.mNext; node != mChildren.Sentinel();
         node = node->mNext) {
        float childElapsed = Child(node)->GetElapsedMs();
        timeline = childElapsed > timeline ? childElapsed : timeline;
    }
    return timeline;
}

// Reconstructed from eboot.elf at 0x41490.
void CompositeGenerator::SeekToMs(float ms) {
    ScopedCritSec lock(gGeneratorKillCritSec);
    _AddPendingChildren();
    for (LinkedListSizeTracked::Node* node = mChildren.mNext; node != mChildren.Sentinel();
         node = node->mNext) {
        Child(node)->SeekToMs(ms);
    }
}

// Reconstructed from eboot.elf at 0x415C0.
bool CompositeGenerator::SetParameter(Symbol name, float value) {
    ScopedCritSec lock(gGeneratorKillCritSec);
    _AddPendingChildren();
    bool set = false;
    for (LinkedListSizeTracked::Node* node = mChildren.mNext; node != mChildren.Sentinel();
         node = node->mNext) {
        set |= Child(node)->SetParameter(name, value);
    }
    return set;
}

// Reconstructed from eboot.elf at 0x41700.
bool CompositeGenerator::GetParameter(Symbol name, float& value) {
    ScopedCritSec lock(gGeneratorKillCritSec);
    _AddPendingChildren();
    for (LinkedListSizeTracked::Node* node = mChildren.mNext; node != mChildren.Sentinel();
         node = node->mNext) {
        if (Child(node)->GetParameter(name, value)) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x41840.
void CompositeGenerator::SetGain(float gain, float fadeSecs, PostFadeOption option) {
    ScopedCritSec lock(gGeneratorKillCritSec);
    _AddPendingChildren();
    for (LinkedListSizeTracked::Node* node = mChildren.mNext; node != mChildren.Sentinel();
         node = node->mNext) {
        Child(node)->SetGain(gain, fadeSecs, option);
    }
    mGain = gain;
}

// Reconstructed from eboot.elf at 0x41980.
float CompositeGenerator::GetGain() const {
    ScopedCritSec lock(gGeneratorKillCritSec);
    if (mChildren.mSize != 0) {
        return ChildList::Owner(mChildren.mNext)->GetGain();
    }
    return mGain;
}

// Reconstructed from eboot.elf at 0x419F0.
void CompositeGenerator::SetMute(bool mute, bool immediate) {
    ScopedCritSec lock(gGeneratorKillCritSec);
    _AddPendingChildren();
    for (LinkedListSizeTracked::Node* node = mChildren.mNext; node != mChildren.Sentinel();
         node = node->mNext) {
        Child(node)->SetMute(mute, immediate);
    }
    mMute = mute;
}

// Reconstructed from eboot.elf at 0x41B40.
bool CompositeGenerator::IsPlaying() const {
    return mChildren.mSize != 0;
}

// Reconstructed from eboot.elf at 0x41B50.
bool CompositeGenerator::Poll() {
    ScopedCritSec lock(gGeneratorKillCritSec);
    LinkedListSizeTracked::Node* node = mChildren.mNext;
    while (node != mChildren.Sentinel()) {
        AudioGenerator* child = Child(node);
        node = node->mNext;
        if (!child->Poll() && child->TryDeactivateHandle()) {
            mChildren.Remove(*child);
            child->Release();
        }
    }
    _AddPendingChildren();
    if (mChildren.mSize == 0) {
        mState = kStateStopped;
    }
    return mChildren.mSize != 0;
}

// Reconstructed from eboot.elf at 0x41CE0.
void CompositeGenerator::Release() {
    ScopedCritSec lock(gGeneratorKillCritSec);
    AudioGeneratorManager& manager = *mManager;
    ScopedCritSec managerLock(manager.mCritSec);
    mHandle &= ~kGeneratorHandleActive;
    mEmitter = nullptr;
    if (mPoolNode.mList != &manager.mFreeList) {
        manager.mFreeList.PushBack(*this);
    }
}

// Reconstructed from eboot.elf at 0x41E60.
Symbol CompositeGenerator::GetTypeId() {
    return kTypeId;
}

// Reconstructed from eboot.elf at 0xDD10.
AudioGenerator* CompositeGeneratorManager::Play(const PlayArgs&) {
    return nullptr;
}

// Reconstructed from eboot.elf at 0xDD30.
int CompositeGeneratorManager::GetIndex() {
    return mManagerIndex;
}

// Reconstructed from eboot.elf at 0xDD40.
Symbol CompositeGeneratorManager::GetId() {
    static Symbol sId("");
    if (sId == Symbol("")) {
        sId = Symbol("CompositeGeneratorManager");
    }
    return sId;
}

// Reconstructed from eboot.elf at 0xDDE0.
Symbol CompositeGeneratorManager::GetResourceExt() {
    static Symbol sExt("");
    if (sExt == Symbol("")) {
        sExt = Symbol(".--none--");
    }
    return sExt;
}

// Reconstructed from eboot.elf at 0xDE80.
AudioGenerator* CompositeGeneratorManager::LockIfOwned(unsigned int handle, int index) {
    ScopedCritSec lock(mCritSec);
    if (mPoolSize >= index && mPool[index].mHandle == handle) {
        ++mPool[index].mRefCount;
        return &mPool[index];
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0xDF00.
void CompositeGeneratorManager::SendStopToAllGenerators() {
    ScopedCritSec lock(mCritSec);
    for (int index = 0; index < mPoolSize; ++index) {
        mPool[index].Stop();
    }
}

// Reconstructed from eboot.elf at 0xDF70.
void CompositeGeneratorManager::SendKillToAllGenerators() {
    ScopedCritSec lock(mCritSec);
    for (int index = 0; index < mPoolSize; ++index) {
        mPool[index].KillLocked();
    }
}

// Reconstructed from eboot.elf at 0xDFE0. The pool is not locked.
void CompositeGeneratorManager::GetActiveHandles(void* handles) {
    auto& list = *static_cast<eastl::vector<unsigned int>*>(handles);
    list.clear();
    for (int index = 0; index < mPoolSize; ++index) {
        unsigned int handle = mPool[index].mHandle;
        if (static_cast<int>(handle) < 0) {
            list.push_back(handle);
        }
    }
}

// Reconstructed from eboot.elf at 0xE130.
void CompositeGeneratorManager::_SetManagerIndex(int index) {
    mManagerIndex = index;
}

// Reconstructed from eboot.elf at 0xE140.
void CompositeGeneratorManager::_InitGeneratorPool() {
    mPool = new CompositeGenerator[mPoolSize];
    for (int index = 0; index < mPoolSize; ++index) {
        mPool[index].Init(this, index);
        mFreeList.PushBack(mPool[index]);
    }
}

// Reconstructed from eboot.elf at 0xE300. The pool is freed only when every
// generator is idle.
bool CompositeGeneratorManager::_DeleteGeneratorPool() {
    ScopedCritSec lock(mCritSec);
    if (mFreeList.mSize != static_cast<unsigned long>(mPoolSize)) {
        return false;
    }
    delete[] mPool;
    return true;
}

// Reconstructed from eboot.elf at 0xE3B0, which jumps to the base
// destructor.
CompositeGeneratorManager::~CompositeGeneratorManager() {}
