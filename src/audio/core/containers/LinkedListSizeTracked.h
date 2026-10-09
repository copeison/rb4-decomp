#pragma once

#include <cstddef>
#include <cstdint>

// Intrusive doubly linked list that tracks its size and lets each node know
// the list holding it. The map names the template
// LinkedListSizeTracked::List<T, &T::member> and emits its destructor out of
// line (for example ~List() for AudioBusCallable); this build inlines every
// member. These inline operations reproduce the sequences the audio code
// inlines.
namespace LinkedListSizeTracked {

class ListBase;

class Node {
public:
    // Inlined into every owner's constructor, for example AudioBusGenerator's
    // at 0xE0490.
    Node() : mNext(this), mPrev(this), mList(nullptr) {}
    // Leaves the holding list, then unlinks itself, as every owner's
    // destructor inlines it (for example AudioGenerator's at 0xE4E0).
    inline ~Node();

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

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
    ListBase() {
        Init();
    }
    // Unlinks every node, then the head itself. Inlined, for example into
    // AudioGeneratorManager's destructor at 0xE780.
    ~ListBase() {
        if (mSize != 0) {
            for (unsigned long count = mSize; count != 0; --count) {
                Node* node = mNext;
                node->mList = nullptr;
                node->mNext->mPrev = node->mPrev;
                node->mPrev->mNext = node->mNext;
                node->mNext = node;
                node->mPrev = node;
            }
            mSize = 0;
        }
        mNext->mPrev = mPrev;
        mPrev->mNext = mNext;
    }

    ListBase(const ListBase&) = delete;
    ListBase& operator=(const ListBase&) = delete;

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

inline Node::~Node() {
    if (mList != nullptr) {
        mList->Remove(*this);
    }
    mNext->mPrev = mPrev;
    mPrev->mNext = mNext;
}

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
