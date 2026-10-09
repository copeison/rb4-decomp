#pragma once

#include <cstddef>

class PropInfo;

// What a property accessor receives, built on the stack by the property
// setters such as the bool one at 0x1A6AA0. A getter receives the first four
// fields. Names not in the reference map.
struct PropAccessorArgs {
    // The setter's first two arguments, the map's ObjPtr const& and
    // Component& (weak: only the order is known).
    const void* mObject;
    void* mComponent;
    // The property's PropInfo.
    const PropInfo* mInfo;
    // The property's storage, or null for an accessor-only property.
    void* mStorage;
    // The value before the change, or null when the set is forced.
    const void* mOldValue;
    // The new value, for a setter.
    const void* mValue;
};

static_assert(offsetof(PropAccessorArgs, mValue) == 40);

// A property's editing metadata (entity/PropMetadata.o), created for its
// type by PropMetadata::Create. Its vtable and members are not
// reconstructed; the bytes stand for them.
class PropMetadata {
public:
    // The vtable and the attribute entries that the constructor (0x12DDB0)
    // sets up: 144-byte records of help text, allowed values and the like.
    // Name not in the reference map.
    unsigned char mAttributes[1528];
};

// The metadata of a bool property (1712 bytes, vtable 0x18E7A00). A
// property registered without an offset is read and written through its
// accessors. Field names are not in the reference map.
class BoolMetadata : public PropMetadata {
public:
    using Setter = void (*)(const PropAccessorArgs& args);
    using Getter = bool (*)(const PropAccessorArgs& args);

    Setter mSetter;
    // Called after a set that changed the value, with the old value, the
    // value read back and the requested value (0x1A6AA0).
    void (*mOnChanged)(const PropAccessorArgs& args);
    Getter mGetter;
    // Zeroed by PropMetadata::Create (0x12ED70); no reader was found.
    void* mReserved;
};

static_assert(offsetof(BoolMetadata, mSetter) == 1528);
static_assert(offsetof(BoolMetadata, mOnChanged) == 1536);
static_assert(offsetof(BoolMetadata, mGetter) == 1544);
static_assert(offsetof(BoolMetadata, mReserved) == 1552);

// The property's metadata as its type's class; the value only selects the
// overload. The overloads share one body in this build.
BoolMetadata& TypeSpecificMetadata(PropInfo& info, const bool& type);  // 0x141C90
