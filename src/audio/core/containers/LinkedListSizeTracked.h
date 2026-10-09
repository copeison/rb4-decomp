#pragma once

#include <cstddef>
#include <cstdint>

// Intrusive doubly linked list that tracks its size and lets each node know
// the list holding it. The map names the template
// LinkedListSizeTracked::List<T, &T::member>; its out-of-line members have
// not been reconstructed. These inline operations reproduce the sequences
// the audio code inlines.
namespace LinkedListSizeTracked {

class ListBase;

class Node {
public:
    Node* mNext;
    Node* mPrev;
    ListBase* mList;  // Null while the node is unlinked.

    void InitUnlinked() {  // Name not in the reference map.
        mNext = this;
        mPrev = this;
        mList = nullptr;
    }
};

static_assert(sizeof(Node) == 24);

// The list head is laid out like the first two links of a node, so the
// sentinel can be addressed as one.
class ListBase {
public:
    Node* mNext;
    Node* mPrev;
    unsigned long mSize;

    Node* Sentinel() {  // Name not in the reference map.
        return reinterpret_cast<Node*>(this);
    }
    const Node* Sentinel() const {  // Name not in the reference map.
        return reinterpret_cast<const Node*>(this);
    }

    void Init() {  // Name not in the reference map.
        mNext = Sentinel();
        mPrev = Sentinel();
        mSize = 0;
    }

    bool Empty() const {  // Name not in the reference map.
        return mNext == Sentinel();
    }

    void PushBack(Node& node) {  // Name not in the reference map.
        node.mList = this;
        ++mSize;
        node.mPrev = mPrev;
        node.mNext = Sentinel();
        mPrev->mNext = &node;
        mPrev = &node;
    }

    // Unlinks a node and leaves it self-linked.
    void Remove(Node& node) {  // Name not in the reference map.
        node.mList = nullptr;
        --mSize;
        node.mNext->mPrev = node.mPrev;
        node.mPrev->mNext = node.mNext;
        node.mNext = &node;
        node.mPrev = &node;
    }

    Node* PopFront() {  // Name not in the reference map.
        Node* node = mNext;
        Remove(*node);
        return node;
    }
};

static_assert(offsetof(ListBase, mSize) == 16);
static_assert(sizeof(ListBase) == 24);

template <class T, Node T::*Member>
class List : public ListBase {
public:
    static T* Owner(Node* node) {  // Name not in the reference map.
        const auto base = reinterpret_cast<T*>(alignof(T) * 64);
        const auto offset = reinterpret_cast<std::uintptr_t>(&(base->*Member)) -
            reinterpret_cast<std::uintptr_t>(base);
        return reinterpret_cast<T*>(reinterpret_cast<char*>(node) - offset);
    }

    void PushBack(T& item) {  // Name not in the reference map.
        ListBase::PushBack(item.*Member);
    }
    void Remove(T& item) {  // Name not in the reference map.
        ListBase::Remove(item.*Member);
    }
    T* PopFront() {  // Name not in the reference map.
        return Owner(ListBase::PopFront());
    }
};

}  // namespace LinkedListSizeTracked
