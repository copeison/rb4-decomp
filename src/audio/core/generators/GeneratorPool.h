#pragma once

#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/output/AudioRenderTarget.h"
#include "utl/containers/Std.h"
#include "os/threading/CritSec.h"

// Pool operations shared by the generator managers. Each manager has its own
// copy of these members in the binary (for example _InitGeneratorPool at
// 0x268910, 0x26A690, 0x26E000, 0x26F280 and 0x271190 for the FMOD managers
// and 0x438B0 for FusionGeneratorManager); the helpers keep one source for
// the shared sequence. Names not in the reference map.
namespace GeneratorPool {

// LockIfOwned: retains a pooled generator whose handle still matches.
template <class Generator>
Generator* LockIfOwned(
    AudioGeneratorManager& manager, Generator* pool, unsigned int handle, int index) {
    ScopedCritSecPtr tracker(&manager.mCritSec);
    if (manager.mPoolSize >= index && pool[index].mHandle == handle) {
        ++pool[index].mRefCount;
        return &pool[index];
    }
    return nullptr;
}

// SendStopToAllGenerators.
template <class Generator>
void SendStop(AudioGeneratorManager& manager, Generator* pool) {
    ScopedCritSecPtr tracker(&manager.mCritSec);
    for (int index = 0; index < manager.mPoolSize; ++index) {
        pool[index].Stop();
    }
}

// SendKillToAllGenerators.
template <class Generator>
void SendKill(AudioGeneratorManager& manager, Generator* pool) {
    ScopedCritSecPtr tracker(&manager.mCritSec);
    for (int index = 0; index < manager.mPoolSize; ++index) {
        pool[index].KillLocked();
    }
}

// EASTL vector<unsigned int> view filled with the active handles.
struct HandleList {
    unsigned int* mBegin;
    unsigned int* mEnd;
    unsigned int* mCapacity;
    void* mAllocator;
};

// Inlined EASTL push_back. Capacity doubles, starting at one.
inline void PushHandle(HandleList& handles, unsigned int handle) {
    if (handles.mEnd >= handles.mCapacity) {
        const long count = handles.mEnd - handles.mBegin;
        const long capacity = count != 0 ? count * 2 : 1;
        auto* storage = static_cast<unsigned int*>(
            HmxAllocator::gStlAllocator.allocate(capacity * sizeof(unsigned int)));
        for (long index = 0; index < count; ++index) {
            storage[index] = handles.mBegin[index];
        }
        if (handles.mBegin != nullptr) {
            HmxAllocator::gStlAllocator.deallocate(
                handles.mBegin, (handles.mCapacity - handles.mBegin) * sizeof(unsigned int));
        }
        handles.mBegin = storage;
        handles.mEnd = storage + count;
        handles.mCapacity = storage + capacity;
    }
    *handles.mEnd++ = handle;
}

// Collects every handle with the active bit set. The pool is not locked.
template <class Generator>
void GetActiveHandles(AudioGeneratorManager& manager, Generator* pool, void* list) {
    auto& handles = *static_cast<HandleList*>(list);
    handles.mEnd = handles.mBegin;
    for (int index = 0; index < manager.mPoolSize; ++index) {
        const unsigned int handle = pool[index].mHandle;
        if (static_cast<int>(handle) < 0) {
            PushHandle(handles, handle);
        }
    }
}

// _InitGeneratorPool: builds the pool and links every voice into the free
// list.
template <class Generator>
void Init(AudioGeneratorManager& manager, Generator*& pool) {
    pool = new Generator[manager.mPoolSize];
    for (int index = 0; index < manager.mPoolSize; ++index) {
        pool[index].Init(&manager, index);
        manager.mFreeList.PushBack(pool[index]);
    }
}

// _DeleteGeneratorPool: frees the pool only when every voice is idle.
template <class Generator>
bool Delete(AudioGeneratorManager& manager, Generator* pool) {
    ScopedCritSecPtr tracker(&manager.mCritSec);
    if (manager.mFreeList.mSize != static_cast<unsigned long>(manager.mPoolSize)) {
        return false;
    }
    delete[] pool;
    return true;
}

// Takes the first idle voice and binds it to a render target and emitter.
template <class Generator>
Generator* Allocate(
    AudioGeneratorManager& manager, AudioRenderTarget* target, AudioEmitter* emitter) {
    AudioRenderTarget* boundTarget = gAudioRenderTargets.mDefault;
    ScopedCritSecPtr tracker(&manager.mCritSec);
    if (manager.mFreeList.mSize == 0) {
        return nullptr;
    }
    if (target != nullptr) {
        boundTarget = target;
    }
    auto* generator = static_cast<Generator*>(manager.mFreeList.PopFront());
    generator->mEmitter = emitter;
    generator->mRenderTarget = boundTarget;
    generator->GetNewHandle();
    return generator;
}

// AudioGenerator::Release: returns a voice to its manager's free list.
inline void Release(AudioGenerator& generator) {
    AudioGeneratorManager& manager = *generator.mManager;
    ScopedCritSecPtr tracker(&manager.mCritSec);
    generator.mHandle &= ~kGeneratorHandleActive;
    generator.mEmitter = nullptr;
    if (generator.mPoolNode.mList != &manager.mFreeList) {
        manager.mFreeList.PushBack(generator);
    }
}

}  // namespace GeneratorPool
