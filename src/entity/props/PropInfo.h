#pragma once

#include <cstddef>

class PropMetadata;

// The type of a registered property. The name is the map's; the
// enumerators are not in the reference map, and only those the
// reconstructed code uses are declared.
enum PropertyType : unsigned int {
    kPropertyBool = 9,
};

// One property of a PropRegistry (entity/PropInfo.o): where it lives in
// its component, its type and its metadata. Field names are not in the
// reference map.
class PropInfo {
public:
    // The byte offset in the component, or -1 for a property reached only
    // through its metadata's accessors.
    int mOffset;
    // Property flags, ~0 by default (PropInfo::PropInfo at 0x12DBE0).
    // RegisterDynamicProp (0x180980) stores its int argument here, and the
    // registry loader (0x181E90) passes the loaded value with 0x20 set.
    unsigned int mFlags;
    PropertyType mType;
    unsigned long mCount;
    // Reference-counted through the map's PropInfo::PropMetadataPtr.
    PropMetadata* mMetadata;
};

static_assert(offsetof(PropInfo, mFlags) == 4);
static_assert(offsetof(PropInfo, mType) == 8);
static_assert(offsetof(PropInfo, mCount) == 16);
static_assert(offsetof(PropInfo, mMetadata) == 24);
static_assert(sizeof(PropInfo) == 32);
