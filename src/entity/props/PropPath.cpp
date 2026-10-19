// entity/PropPath.o (part): the comparisons.
#include "entity/props/PropPath.h"

// Reconstructed from eboot.elf at 0x17EA80.
bool PropPath::operator==(const PropPath& other) const {
    if (mSize != other.mSize) {
        return false;
    }
    for (unsigned long i = 0; i < mSize; ++i) {
        const Node& node = mNodes[i];
        const Node& otherNode = other.mNodes[i];
        if (node.mType != otherNode.mType) {
            return false;
        }
        if (node.mType != Node::kNodeNone && node.mValue != otherNode.mValue) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x17EB40.
bool PropPath::operator<(const PropPath& other) const {
    if (mSize != other.mSize) {
        return mSize < other.mSize;
    }
    for (unsigned long i = 0; i < mSize; ++i) {
        const Node& node = mNodes[i];
        const Node& otherNode = other.mNodes[i];
        if (node.mType != otherNode.mType) {
            return node.mType < otherNode.mType;
        }
        if (node.mType != Node::kNodeNone && node.mValue != otherNode.mValue) {
            return node.mValue < otherNode.mValue;
        }
    }
    return false;
}
