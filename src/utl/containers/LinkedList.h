#pragma once

// Intrusive circular doubly linked list. The map names the template
// LinkedList::List<T, T::ListNode>: the second argument is a type of the
// element class that reaches the element's link. Only the inline operations
// the reconstructed code uses are declared; member names other than List
// and Node are not in the reference map.
namespace LinkedList {

// One link. A node starts linked to itself and unlinks itself when
// destroyed.
class Node {
public:
    Node() : mNext(this), mPrev(this) {}
    ~Node() {
        mNext->mPrev = mPrev;
        mPrev->mNext = mNext;
    }

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    // Unlinks the node and leaves it linked to itself.
    void Remove() {
        mNext->mPrev = mPrev;
        mPrev->mNext = mNext;
        mNext = this;
        mPrev = this;
    }

    Node* mNext;
    Node* mPrev;
};

static_assert(sizeof(Node) == 16);

// The list head is a Node; the list is empty while the head is linked to
// itself. Access is a type with static ToNode and FromNode functions mapping
// an element to its link and back.
template <class T, class Access>
class List {
public:
    class iterator {
    public:
        explicit iterator(Node* node) : mNode(node) {}

        T& operator*() const {
            return *Access::FromNode(mNode);
        }
        T* operator->() const {
            return Access::FromNode(mNode);
        }
        iterator& operator++() {
            mNode = mNode->mNext;
            return *this;
        }
        bool operator==(const iterator& other) const {
            return mNode == other.mNode;
        }
        bool operator!=(const iterator& other) const {
            return mNode != other.mNode;
        }

    private:
        Node* mNode;
    };

    // The destructor unlinks the head (for RndOverlay at 0x5F9120).
    ~List() {}

    iterator begin() {
        return iterator(mHead.mNext);
    }
    iterator end() {
        return iterator(&mHead);
    }

    bool empty() const {
        return mHead.mNext == &mHead;
    }
    unsigned long size() const {
        unsigned long count = 0;
        for (const Node* node = mHead.mNext; node != &mHead; node = node->mNext) {
            ++count;
        }
        return count;
    }
    T& front() {
        return *Access::FromNode(mHead.mNext);
    }
    T& back() {
        return *Access::FromNode(mHead.mPrev);
    }
    // The neighbours of an element, or null at the ends.
    T* next(T& item) {
        Node* node = Access::ToNode(item).mNext;
        return node != &mHead ? Access::FromNode(node) : nullptr;
    }
    T* prev(T& item) {
        Node* node = Access::ToNode(item).mPrev;
        return node != &mHead ? Access::FromNode(node) : nullptr;
    }

    void push_back(T& item) {
        InsertBefore(Access::ToNode(item), mHead);
    }
    void remove(T& item) {
        Access::ToNode(item).Remove();
    }
    // Moves the elements of `other` to the end of the list.
    void splice(List& other) {
        Node* first = other.mHead.mNext;
        if (first == &other.mHead || first == &mHead) {
            return;
        }
        Node* last = other.mHead.mPrev;
        other.mHead.mNext = &other.mHead;
        other.mHead.mPrev = &other.mHead;
        first->mPrev = mHead.mPrev;
        mHead.mPrev->mNext = first;
        last->mNext = &mHead;
        mHead.mPrev = last;
    }

    // Sorts the list with the comparison, as the map's SortInternal does.
    template <class Cmp>
    void sort(Cmp cmp) {
        SortInternal(cmp, &mHead, &mHead, static_cast<long>(size()));
    }

    // Quicksorts the `count` elements after `before` and before `end`
    // around their middle element. The map's signature takes the
    // comparison by reference; it is empty and passed by value here.
    template <class Cmp>
    static void SortInternal(Cmp cmp, Node* before, Node* end, long count) {
        while (count >= 2) {
            Node* node = before->mNext;
            unsigned long left = static_cast<unsigned long>(count) >> 1;
            Node* pivot = node;
            for (unsigned long i = 0; i < left; ++i) {
                pivot = pivot->mNext;
            }
            T& pivotItem = *Access::FromNode(pivot);
            Node* last = end->mPrev;
            while (node != pivot) {
                Node* next = node->mNext;
                if (cmp(pivotItem, *Access::FromNode(node))) {
                    --left;
                    node->Remove();
                    InsertBefore(*node, *end);
                }
                node = next;
            }
            Node* moved = last->mNext;
            for (node = pivot->mNext; node != moved;) {
                Node* next = node->mNext;
                if (cmp(*Access::FromNode(node), pivotItem)) {
                    ++left;
                    node->Remove();
                    InsertBefore(*node, *pivot);
                }
                node = next;
            }
            SortInternal(cmp, before, pivot, static_cast<long>(left));
            count = count - 1 - static_cast<long>(left);
            before = pivot;
        }
    }

private:
    static void InsertBefore(Node& node, Node& position) {
        node.mPrev = position.mPrev;
        node.mNext = &position;
        position.mPrev->mNext = &node;
        position.mPrev = &node;
    }

    Node mHead;
};

}  // namespace LinkedList
