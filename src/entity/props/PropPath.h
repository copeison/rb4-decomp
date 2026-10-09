#pragma once

#include <cstddef>

#include "utl/text/Symbol.h"

class DataArray;

// A path to a property: up to twelve nodes, each a member name or an array
// index (entity/PropPath.o). Only the parts the reconstructed code uses are
// declared; the object is not reconstructed.
class PropPath {
public:
    // One node of the path. The map names the type; the field names are
    // not in the reference map.
    struct Node {
        // How mValue is read. Names not in the reference map.
        enum Type : unsigned char {
            kNodeSymbol = 0,
            kNodeIndex = 1,
            // An unused node; every node of a new path has it.
            kNodeNone = 2,
        };

        // Leaves the value unset, as the inlined constructors do.
        Node() : mType(kNodeNone) {}
        // The binary's copies of the assignments (0x17E850, 0x17E860) are
        // called on temporaries and return them.
        Node& operator=(Symbol name);          // 0x17E850
        Node& operator=(unsigned long index);  // 0x17E860

        // The interned characters of a member name. Name not in the
        // reference map.
        const char* NameStr() const {
            return reinterpret_cast<const char*>(mValue);
        }

        Type mType;
        // The member name's characters or the index.
        unsigned long mValue;
    };

    // The maximum number of nodes. Name not in the reference map.
    static constexpr unsigned long kMaxNodes = 12;

    // An empty path. Inlined by every user.
    PropPath() : mSize(0) {}

    // Sets the path from the message's nodes from `start` on: symbols name
    // members and integers index arrays.
    void FromDataArray(const DataArray* array, unsigned long start);  // 0x17EF60
    // Writes the path as "(name : index ...)" into the buffer.
    void ToString(char* buffer, unsigned long size) const;  // 0x17EBA0

    // The node at the index; a negative index counts from the end. Inlined
    // into PropRegistry::FindProp (0x180B40). Name not in the reference map.
    const Node& operator[](int index) const {
        return mNodes[index < 0 ? static_cast<long>(mSize) + index : index];
    }

    // Adds and removes the last node, as the map's PropPath::ScopedPusher
    // does around each visited property. Inlined by the visitors. Names not
    // in the reference map.
    void PushNode(const Node& node) {
        mNodes[mSize++] = node;
    }
    void PopNode() {
        --mSize;
        mNodes[mSize].mType = Node::kNodeNone;
    }

    Node mNodes[kMaxNodes];
    unsigned long mSize;
};

static_assert(sizeof(PropPath::Node) == 16);
static_assert(offsetof(PropPath, mSize) == 192);
static_assert(sizeof(PropPath) == 200);
