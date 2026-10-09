#pragma once

#include <cstddef>

class PropMetadata;
class PropRegistry;

// The type of a registered property. The name is the map's; the
// enumerators are not in the reference map, and only those the
// reconstructed code uses are declared.
enum PropertyType : unsigned int {
    kPropertyBool = 9,
    // A property that only runs its metadata's action; RegisterAction
    // (0x180910) registers it without storage.
    kPropertyAction = 17,
    // Set on an array's type; the low byte is the element type.
    kPropertyArray = 0x100,
    // The type a new PropInfo has (0x12DBE0) and the type
    // Component::Insert passes to mean the element's own type.
    kPropertyNone = 0xFEFF,
};

// One property of a PropRegistry (entity/PropInfo.o, 0x12DBE0-0x12DD7F):
// where it lives in its component, its type and its metadata. Field names
// are not in the reference map.
class PropInfo {
public:
    // The reference-counted metadata pointer (the map's
    // PropInfo::PropMetadataPtr). The count is PropMetadata::mRefCount; the
    // last release deletes the metadata through its virtual destructor.
    class PropMetadataPtr {
    public:
        PropMetadataPtr() : mPtr(nullptr) {}
        // Inlined in this build as an assignment to the null pointer.
        explicit PropMetadataPtr(PropMetadata* metadata) : mPtr(nullptr) {
            *this = metadata;
        }
        PropMetadataPtr(const PropMetadataPtr& other);     // 0x12DCB0
        ~PropMetadataPtr();                                // 0x12DCD0
        PropMetadataPtr& operator=(PropMetadata* metadata);         // 0x12DC70
        PropMetadataPtr& operator=(const PropMetadataPtr& other);  // 0x12DD00

        PropMetadata* operator->() const {
            return mPtr;
        }
        PropMetadata* Get() const {
            return mPtr;
        }

        PropMetadata* mPtr;  // Name not in the reference map.
    };

    PropInfo();  // 0x12DBE0

    // The PropInfo of an array's elements, which the array's metadata
    // holds.
    const PropInfo& PropArrayItemInfo() const;  // 0x12DC20
    // Whether the property is stored in the component and its metadata
    // marks it for saving (the flag at PropMetadata+1329). Name not in the
    // reference map; the evidence is weak.
    bool IsSaved() const;  // 0x12DC00
    // The registry of a struct property's members, from the metadata's
    // slot 7; null for other types. The binary has a const and a non-const
    // copy (0x12DC30, 0x12DC40). Name not in the reference map. Not
    // reconstructed: PropMetadata's virtual methods are not modelled.
    const PropRegistry* GetStructRegistry() const;  // 0x12DC40

    // The address of the property in the component (or in the array element
    // or struct at `component`), or null. A property
    // with mDynamicOffset set lives in the dynamic storage whose pointer is
    // mDynamicOffset bytes past the component's vtable pointer. Inlined into
    // every property visitor, for example Component::Dump at 0xE8330. Name
    // not in the reference map.
    void* Storage(void* component) const {
        unsigned char* base = static_cast<unsigned char*>(component);
        if (mDynamicOffset >= 0) {
            base = *reinterpret_cast<unsigned char**>(base + mDynamicOffset + 8);
        }
        if (base == nullptr) {
            return nullptr;
        }
        return mOffset < 0 ? nullptr : base + mOffset;
    }

    // The byte offset in the component (or its dynamic storage), or -1 for
    // a property reached only through its metadata's accessors.
    int mOffset;
    // -1 for a property in the component itself. RegisterDynamicProp
    // (0x180980) stores its int argument here: the offset of the dynamic
    // storage pointer, counted from the component's first member.
    int mDynamicOffset;
    PropertyType mType;
    // The element count of a fixed array, or the storage size of a dynamic
    // property.
    unsigned long mCount;
    PropMetadataPtr mMetadata;
};

static_assert(sizeof(PropInfo::PropMetadataPtr) == 8);
static_assert(offsetof(PropInfo, mDynamicOffset) == 4);
static_assert(offsetof(PropInfo, mType) == 8);
static_assert(offsetof(PropInfo, mCount) == 16);
static_assert(offsetof(PropInfo, mMetadata) == 24);
static_assert(sizeof(PropInfo) == 32);
