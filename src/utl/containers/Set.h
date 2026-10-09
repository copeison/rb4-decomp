#pragma once

#include "utl/containers/Map.h"

namespace eastl {

// EASTL's set, in the subset the UTF8 line-break tables use: the comparison
// object, the tree's anchor node, the size and the allocator, with 40-byte
// nodes for 16-bit keys. The range constructor of eastl::set<unsigned short>
// is at 0x1184750 and its subtree release at 0x1186E10.
template <
    typename Key,
    typename Compare = less<Key>,
    typename Allocator = HmxAllocator::allocator>
class set {
public:
    struct node_type : rbtree_node_base {
        Key mValue;
    };

    template <typename InputIterator>
    set(InputIterator first, InputIterator last) : mCompare(), mnSize(0), mAllocator("EASTL set") {
        mAnchor.mpNodeRight = &mAnchor;
        mAnchor.mpNodeLeft = &mAnchor;
        mAnchor.mpNodeParent = nullptr;
        mAnchor.mColor = 0;
        for (; first != last; ++first) {
            DoInsertValue(*first);
        }
    }
    set(const set&) = delete;
    set& operator=(const set&) = delete;
    // The atexit thunk at 0x11848D0.
    ~set() {
        DoNukeSubtree(static_cast<node_type*>(mAnchor.mpNodeParent));
    }

    const rbtree_node_base* end() const {
        return &mAnchor;
    }
    const rbtree_node_base* find(const Key& key) const {
        const rbtree_node_base* current = mAnchor.mpNodeParent;
        const rbtree_node_base* range = &mAnchor;
        while (current != nullptr) {
            if (!mCompare(static_cast<const node_type*>(current)->mValue, key)) {
                range = current;
                current = current->mpNodeLeft;
            } else {
                current = current->mpNodeRight;
            }
        }
        if (range != &mAnchor && !mCompare(key, static_cast<const node_type*>(range)->mValue)) {
            return range;
        }
        return &mAnchor;
    }

    Compare mCompare;
    rbtree_node_base mAnchor;
    unsigned long mnSize;
    Allocator mAllocator;

private:
    void DoNukeSubtree(node_type* node) {
        while (node != nullptr) {
            DoNukeSubtree(static_cast<node_type*>(node->mpNodeRight));
            auto* left = static_cast<node_type*>(node->mpNodeLeft);
            mAllocator.deallocate(node, sizeof(node_type));
            node = left;
        }
    }

    // Inserts the value unless it is present, as EASTL's
    // DoInsertValue(true_type, const value_type&).
    void DoInsertValue(const Key& key) {
        rbtree_node_base* current = mAnchor.mpNodeParent;
        rbtree_node_base* parent = &mAnchor;
        bool less = true;
        while (current != nullptr) {
            parent = current;
            less = mCompare(key, static_cast<node_type*>(current)->mValue);
            current = less ? current->mpNodeLeft : current->mpNodeRight;
        }
        rbtree_node_base* lower = parent;
        if (less) {
            if (lower != mAnchor.mpNodeLeft) {
                lower = RBTreeDecrement(lower);
                if (!mCompare(static_cast<node_type*>(lower)->mValue, key)) {
                    return;
                }
            }
        } else if (!mCompare(static_cast<node_type*>(lower)->mValue, key)) {
            return;
        }
        const auto side = parent == &mAnchor || mCompare(key, static_cast<node_type*>(parent)->mValue)
            ? kRBTreeSideLeft
            : kRBTreeSideRight;
        auto* node = static_cast<node_type*>(mAllocator.allocate(sizeof(node_type)));
        node->mValue = key;
        RBTreeInsert(node, parent, &mAnchor, side);
        ++mnSize;
    }
};

}  // namespace eastl

static_assert(sizeof(eastl::set<unsigned short>) == 56);
static_assert(sizeof(eastl::set<unsigned short>::node_type) == 40);
