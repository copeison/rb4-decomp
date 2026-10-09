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

    void push_back(T& item) {
        Node& node = Access::ToNode(item);
        node.mPrev = mHead.mPrev;
        node.mNext = &mHead;
        mHead.mPrev->mNext = &node;
        mHead.mPrev = &node;
    }
    void remove(T& item) {
        Access::ToNode(item).Remove();
    }

private:
    Node mHead;
};

}  // namespace LinkedList
