#pragma once

#include <new>
#include <utility>

#include "utl/containers/Std.h"

namespace eastl {

// EASTL's list links, in the subset reconstructed code uses.
struct ListNodeBase {
    ListNodeBase* mpNext;
    ListNodeBase* mpPrev;
};

// EASTL's doubly linked list with a cached size: the anchor node, the size
// and the allocator (32 bytes). Nodes hold the links and then the value.
template <typename T, typename Allocator = HmxAllocator::allocator>
class list {
public:
    struct node_type : ListNodeBase {
        T mValue;
    };

    class iterator {
    public:
        explicit iterator(ListNodeBase* node = nullptr) : mpNode(node) {}
        T& operator*() const {
            return static_cast<node_type*>(mpNode)->mValue;
        }
        T* operator->() const {
            return &static_cast<node_type*>(mpNode)->mValue;
        }
        iterator& operator++() {
            mpNode = mpNode->mpNext;
            return *this;
        }
        bool operator==(const iterator& other) const {
            return mpNode == other.mpNode;
        }
        bool operator!=(const iterator& other) const {
            return mpNode != other.mpNode;
        }

        ListNodeBase* mpNode;
    };

    list() : mSize(0), mAllocator("EASTL list") {
        mNode.mpNext = &mNode;
        mNode.mpPrev = &mNode;
    }
    // Copies the values node by node, as EASTL's copy constructor does (for
    // example in FusionVoicePool's destructor at 0xA08A0).
    list(const list& other) : mSize(0), mAllocator(other.mAllocator) {
        mNode.mpNext = &mNode;
        mNode.mpPrev = &mNode;
        for (const ListNodeBase* node = other.mNode.mpNext; node != &other.mNode; node = node->mpNext) {
            push_back(static_cast<const node_type*>(node)->mValue);
        }
    }
    list& operator=(const list&) = delete;
    ~list() {
        DoClear();
    }

    unsigned long size() const {
        return mSize;
    }
    bool empty() const {
        return mSize == 0;
    }
    iterator begin() {
        return iterator(mNode.mpNext);
    }
    iterator end() {
        return iterator(&mNode);
    }

    // Links a new node before the anchor.
    void push_back(const T& value) {
        auto* node = static_cast<node_type*>(mAllocator.allocate(sizeof(node_type)));
        new (&node->mValue) T(value);
        node->mpNext = &mNode;
        node->mpPrev = mNode.mpPrev;
        mNode.mpPrev->mpNext = node;
        mNode.mpPrev = node;
        ++mSize;
    }

    // Frees every node and leaves the anchor linked to itself.
    void clear() {
        DoClear();
        mNode.mpNext = &mNode;
        mNode.mpPrev = &mNode;
        mSize = 0;
    }

    iterator erase(iterator position) {
        ListNodeBase* const node = position.mpNode;
        ListNodeBase* const next = node->mpNext;
        next->mpPrev = node->mpPrev;
        node->mpPrev->mpNext = next;
        static_cast<node_type*>(node)->mValue.~T();
        mAllocator.deallocate(node, sizeof(node_type));
        --mSize;
        return iterator(next);
    }

    ListNodeBase mNode;
    unsigned long mSize;
    Allocator mAllocator;

private:
    void DoClear() {
        ListNodeBase* node = mNode.mpNext;
        while (node != &mNode) {
            ListNodeBase* const next = node->mpNext;
            static_cast<node_type*>(node)->mValue.~T();
            mAllocator.deallocate(node, sizeof(node_type));
            node = next;
        }
    }
};

}  // namespace eastl
