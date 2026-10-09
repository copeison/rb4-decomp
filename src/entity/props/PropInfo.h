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
    unsigned int mUnknown4;
    PropertyType mType;
    unsigned long mCount;
    // Reference-counted through the map's PropInfo::PropMetadataPtr.
    PropMetadata* mMetadata;
};

static_assert(offsetof(PropInfo, mType) == 8);
static_assert(offsetof(PropInfo, mCount) == 16);
static_assert(offsetof(PropInfo, mMetadata) == 24);
static_assert(sizeof(PropInfo) == 32);
