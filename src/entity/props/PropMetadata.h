#pragma once

#include <cstddef>

#include "entity/props/PropInfo.h"
#include "entity/props/PropRegistry.h"

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
// type by Create. PropInfo::PropMetadataPtr counts its references. Only the
// destructor of its eight virtual methods is declared, and the members are
// not reconstructed; the bytes stand for them.
class PropMetadata {
public:
    // Slots 0-1: 0x12E610, 0x12ED50. PropInfo::PropMetadataPtr deletes the
    // metadata through slot 1.
    virtual ~PropMetadata();

    PropMetadata();  // 0x12DDB0

    // A new metadata of the class for the type, such as BoolMetadata.
    static PropMetadata* Create(PropertyType type);  // 0x12ED70

    // Field names are not in the reference map.
    // The attribute entries that the constructor sets up: 144-byte records
    // of help text, allowed values and the like.
    unsigned char mAttributes[1320];
    // Five flags the constructor sets to 0, 1, 1, 0 and 0. PropInfo::IsSaved
    // reads mFlags[1]; the others' readers are not identified.
    bool mFlags[5];
    // The last attribute record (from +1344) and the members up to the
    // reference count; not decoded.
    unsigned char mTrailingAttributes[187];
    // The references PropInfo::PropMetadataPtr holds. Name not in the
    // reference map.
    long mRefCount;
};

static_assert(offsetof(PropMetadata, mFlags) == 1328);
static_assert(offsetof(PropMetadata, mRefCount) == 1520);
static_assert(sizeof(PropMetadata) == 1528);

// The metadata of an array property (1904 bytes): the PropInfo of its
// elements, which PropRegistry::RegisterPropArrayItem fills, and the array's
// own attributes. Field names are not in the reference map.
class ArrayMetadata : public PropMetadata {
public:
    ArrayMetadata();  // 0x1415F0

    PropInfo mItemInfo;
    // The array attributes the constructor sets up; not decoded.
    unsigned char mArrayAttributes[344];
};

static_assert(offsetof(ArrayMetadata, mItemInfo) == 1528);
static_assert(sizeof(ArrayMetadata) == 1904);

// The metadata of a struct property: the registry of its members, whose
// dynamic size and alignment PropRegistry::AmendDynamicStructSize reads.
// The map has GroupMetadata; that this is the class is inferred from the
// struct's place in the registry, so the evidence is weak. Field names are
// not in the reference map.
class GroupMetadata : public PropMetadata {
public:
    // The group attributes; not decoded.
    unsigned char mGroupAttributes[168];
    PropRegistry mRegistry;
};

static_assert(offsetof(GroupMetadata, mRegistry) == 1696);

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
