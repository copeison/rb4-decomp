#pragma once

#include "utl/containers/Std.h"

namespace eastl {

// EASTL's red-black tree, in the subset reconstructed code uses. The
// non-template tree functions are EASTL library code in the binary.
enum RBTreeSide {
    kRBTreeSideLeft,
    kRBTreeSideRight,
};

struct rbtree_node_base {
    rbtree_node_base* mpNodeRight;
    rbtree_node_base* mpNodeLeft;
    rbtree_node_base* mpNodeParent;
    char mColor;
};

rbtree_node_base* RBTreeIncrement(const rbtree_node_base* node);  // 0x253100
rbtree_node_base* RBTreeDecrement(const rbtree_node_base* node);  // 0x253140
void RBTreeInsert(
    rbtree_node_base* node,
    rbtree_node_base* parent,
    rbtree_node_base* anchor,
    RBTreeSide side);  // 0x253270
void RBTreeErase(rbtree_node_base* node, rbtree_node_base* anchor);  // 0x2534A0

template <typename T>
struct less {
    bool operator()(const T& a, const T& b) const {
        return a < b;
    }
};

template <typename T1, typename T2>
struct pair {
    T1 first;
    T2 second;
};

// EASTL's map: the comparison object, the tree's anchor node (its right and
// left point at the largest and smallest nodes, its parent at the root),
// the size and the allocator. Nodes are 48 bytes for 8-byte keys and values.
template <
    typename Key,
    typename T,
    typename Compare = less<Key>,
    typename Allocator = HmxAllocator::allocator>
class map {
public:
    using value_type = pair<const Key, T>;

    struct node_type : rbtree_node_base {
        value_type mValue;
    };

    class iterator {
    public:
        explicit iterator(rbtree_node_base* node = nullptr) : mpNode(node) {}
        value_type& operator*() const {
            return static_cast<node_type*>(mpNode)->mValue;
        }
        value_type* operator->() const {
            return &static_cast<node_type*>(mpNode)->mValue;
        }
        iterator& operator++() {
            mpNode = RBTreeIncrement(mpNode);
            return *this;
        }
        iterator& operator--() {
            mpNode = RBTreeDecrement(mpNode);
            return *this;
        }
        bool operator==(const iterator& other) const {
            return mpNode == other.mpNode;
        }
        bool operator!=(const iterator& other) const {
            return mpNode != other.mpNode;
        }

        rbtree_node_base* mpNode;
    };

    map() : mCompare(), mnSize(0), mAllocator("EASTL map") {
        mAnchor.mpNodeRight = &mAnchor;
        mAnchor.mpNodeLeft = &mAnchor;
        mAnchor.mpNodeParent = nullptr;
        mAnchor.mColor = 0;
    }
    map(const map&) = delete;
    map& operator=(const map&) = delete;
    ~map() {
        DoNukeSubtree(static_cast<node_type*>(mAnchor.mpNodeParent));
    }

    unsigned long size() const {
        return mnSize;
    }
    iterator begin() {
        return iterator(mAnchor.mpNodeLeft);
    }
    iterator end() {
        return iterator(&mAnchor);
    }

    iterator lower_bound(const Key& key) {
        rbtree_node_base* current = mAnchor.mpNodeParent;
        rbtree_node_base* range = &mAnchor;
        while (current != nullptr) {
            if (!mCompare(static_cast<node_type*>(current)->mValue.first, key)) {
                range = current;
                current = current->mpNodeLeft;
            } else {
                current = current->mpNodeRight;
            }
        }
        return iterator(range);
    }

    iterator find(const Key& key) {
        const auto it = lower_bound(key);
        if (it != end() && !mCompare(key, it->first)) {
            return it;
        }
        return end();
    }

    T& operator[](const Key& key) {
        auto it = lower_bound(key);
        if (it == end() || mCompare(key, it->first)) {
            it = DoInsertKey(it, key);
        }
        return it->second;
    }

    iterator erase(iterator position) {
        iterator next(position);
        ++next;
        RBTreeErase(position.mpNode, &mAnchor);
        --mnSize;
        DoFreeNode(static_cast<node_type*>(position.mpNode));
        return next;
    }

    Compare mCompare;
    rbtree_node_base mAnchor;
    unsigned long mnSize;
    Allocator mAllocator;

private:
    node_type* DoCreateNode(const Key& key) {
        auto* node = static_cast<node_type*>(mAllocator.allocate(sizeof(node_type)));
        const_cast<Key&>(node->mValue.first) = key;
        node->mValue.second = T();
        return node;
    }
    void DoFreeNode(node_type* node) {
        mAllocator.deallocate(node, sizeof(node_type));
    }
    void DoNukeSubtree(node_type* node) {
        while (node != nullptr) {
            DoNukeSubtree(static_cast<node_type*>(node->mpNodeRight));
            auto* left = static_cast<node_type*>(node->mpNodeLeft);
            DoFreeNode(node);
            node = left;
        }
    }

    // Inserts the key with a position hint, as EASTL's
    // DoInsertKey(true_type, const_iterator, const key_type&).
    iterator DoInsertKey(iterator position, const Key& key) {
        if (position.mpNode != mAnchor.mpNodeRight && position.mpNode != &mAnchor) {
            iterator next(position);
            ++next;
            if (mCompare(position->first, key) && mCompare(key, next->first)) {
                if (position.mpNode->mpNodeRight != nullptr) {
                    return DoInsertKeyImpl(next.mpNode, true, key);
                }
                return DoInsertKeyImpl(position.mpNode, false, key);
            }
            return DoInsertKey(key);
        }
        if (mnSize != 0 && mCompare(static_cast<node_type*>(mAnchor.mpNodeRight)->mValue.first, key)) {
            return DoInsertKeyImpl(mAnchor.mpNodeRight, false, key);
        }
        return DoInsertKey(key);
    }

    // Inserts the key without a hint, returning the existing node when the
    // key is present.
    iterator DoInsertKey(const Key& key) {
        rbtree_node_base* current = mAnchor.mpNodeParent;
        rbtree_node_base* parent = &mAnchor;
        bool less = true;
        while (current != nullptr) {
            parent = current;
            less = mCompare(key, static_cast<node_type*>(current)->mValue.first);
            current = less ? current->mpNodeLeft : current->mpNodeRight;
        }
        iterator lower(parent);
        if (less) {
            if (lower.mpNode == mAnchor.mpNodeLeft) {
                return DoInsertKeyImpl(parent, false, key);
            }
            --lower;
        }
        if (mCompare(lower->first, key)) {
            return DoInsertKeyImpl(parent, false, key);
        }
        return lower;
    }

    iterator DoInsertKeyImpl(rbtree_node_base* parent, bool forceToLeft, const Key& key) {
        const auto side = forceToLeft || parent == &mAnchor ||
                mCompare(key, static_cast<node_type*>(parent)->mValue.first)
            ? kRBTreeSideLeft
            : kRBTreeSideRight;
        auto* node = DoCreateNode(key);
        RBTreeInsert(node, parent, &mAnchor, side);
        ++mnSize;
        return iterator(node);
    }
};

}  // namespace eastl
