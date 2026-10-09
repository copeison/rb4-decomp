#pragma once

#include <cstddef>

class PropInfo;

// What a property accessor receives. Name not in the reference map; only
// the value is declared.
struct PropAccessorArgs {
    unsigned char mUnknown0[40];
    // The new value, for a setter.
    const void* mValue;
};

static_assert(offsetof(PropAccessorArgs, mValue) == 40);

// A property's editing metadata (entity/PropMetadata.o), created for its
// type by PropMetadata::Create. Its vtable and members are not
// reconstructed; the bytes stand for them.
class PropMetadata {
public:
    unsigned char mUnknown0[1528];
};

// The metadata of a bool property (1712 bytes, vtable 0x18E7A00). A
// property registered without an offset is read and written through its
// accessors. Field names are not in the reference map.
class BoolMetadata : public PropMetadata {
public:
    using Setter = void (*)(const PropAccessorArgs& args);
    using Getter = bool (*)(const PropAccessorArgs& args);

    Setter mSetter;
    void* mUnknown1536;
    Getter mGetter;
    void* mUnknown1552;
};

static_assert(offsetof(BoolMetadata, mSetter) == 1528);
static_assert(offsetof(BoolMetadata, mGetter) == 1544);

// The property's metadata as its type's class; the value only selects the
// overload. The overloads share one body in this build.
BoolMetadata& TypeSpecificMetadata(PropInfo& info, const bool& type);  // 0x141C90
