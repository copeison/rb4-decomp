#pragma once

#include <_pthread.h>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <new>

// The pthread key behind a TLSValue and the function that destroys a
// thread's value. Name not in the reference map; the map has only the
// TLSValue<T> instantiations.
struct TLSKey {
    // A thread's slot: the key it belongs to and the thread's value. Name not
    // in the reference map.
    struct Node {
        TLSKey* mKey;
        void* mValue;
    };

    explicit TLSKey(void (*destroy)(void*)) : mDestroy(destroy) {
        scePthreadKeyCreate(&mKey, DestroyNode);
    }
    ~TLSKey() {
        scePthreadKeyDelete(mKey);
    }

    // The key's destructor, run when a thread exits: destroys the value and
    // frees the slot. Every instantiation is folded into the copy at
    // 0xF9750.
    static void DestroyNode(void* slot) {
        auto* node = static_cast<Node*>(slot);
        if (node->mKey->mDestroy != nullptr) {
            node->mKey->mDestroy(node->mValue);
        }
        scePthreadSetspecific(node->mKey->mKey, nullptr);
        free(node);
    }

    // Field names are not in the reference map.
    ScePthreadKey mKey;
    void (*mDestroy)(void*);
};

static_assert(offsetof(TLSKey, mDestroy) == 8);

// A per-thread value created on first use (the map's TLSValue<T>). Each
// thread's value is built by mCreate, a std::function holding the
// constructor's lambda, and kept in a malloc'd slot under the key.
template <typename T>
class TLSValue {
public:
    TLSValue() : mKey(Destroy) {
        mCreate = [] { return new (malloc(sizeof(T))) T(); };
    }

    // The calling thread's value, created on first use.
    T* Get() {
        auto* node = static_cast<TLSKey::Node*>(scePthreadGetspecific(mKey.mKey));
        T* value;
        if (node == nullptr || (value = static_cast<T*>(node->mValue)) == nullptr) {
            value = mCreate();
            auto* slot = static_cast<TLSKey::Node*>(malloc(sizeof(TLSKey::Node)));
            slot->mKey = &mKey;
            slot->mValue = value;
            scePthreadSetspecific(mKey.mKey, slot);
        }
        return value;
    }
    // The calling thread's value, or null when it has none yet. Name not in
    // the reference map; MemAlloc (0x37AE70) inlines it.
    T* Peek() {
        auto* node = static_cast<TLSKey::Node*>(scePthreadGetspecific(mKey.mKey));
        return node != nullptr ? static_cast<T*>(node->mValue) : nullptr;
    }
    T& operator*() {
        return *Get();
    }
    T* operator->() {
        return Get();
    }

private:
    // The key's destroy function: frees the value.
    static void Destroy(void* value) {
        if (value != nullptr) {
            static_cast<T*>(value)->~T();
            free(value);
        }
    }

    unsigned char mPadding0[8];  // Never read or written.
    TLSKey mKey;
    std::function<T*()> mCreate;
};

static_assert(sizeof(TLSValue<int>) == 0x50);
