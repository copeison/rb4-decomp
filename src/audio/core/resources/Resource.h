#pragma once

#include <atomic>
#include <cstddef>

#include "os/files/File.h"
#include "utl/text/Symbol.h"

class BinStream;

// Type information shared by every resource of one class. Only the fields
// read by the IsA overrides are declared. Field names are not in the
// reference map.
class ResourceMetaData {
public:
    unsigned char mUnknown0[64];
    Symbol mId;
    unsigned char mUnknown72[8];
    ResourceMetaData* mParent;
};

static_assert(offsetof(ResourceMetaData, mId) == 64);
static_assert(offsetof(ResourceMetaData, mParent) == 80);

// Path of a loadable resource. The map places it with Resource in the entity
// module, which has not been reconstructed; only its storage is declared.
class ResourcePath {
public:
    ResourcePath() : mPath("") {}

    Symbol mPath;  // Name not in the reference map.
};

// Reference-counted engine resource. The map places it in the entity module;
// until that module is reconstructed, only the members the FMOD resources use
// are declared here. The vtable has 13 slots.
class Resource {
public:
    // Inlined into each resource constructor, for example at 0x271660. The
    // engine path is resolved from the empty string.
    Resource() : mRefs(0), mUnknown20(0), mUnknown22(), mUnknown40(0) {
        FileResolvePath(mPath.mPath, "");
    }

    virtual ResourceMetaData* GetMetaData() const = 0;  // slot 0
    virtual Symbol GetId() const = 0;                   // slot 1
    virtual bool IsA(Symbol type) const = 0;            // slot 2
    virtual void LoadFile();                            // slot 3
    virtual bool Load(BinStream& stream, bool unknown);  // slot 4
    virtual void Save(BinStream& stream, bool unknown);  // slot 5
    virtual bool Fail() const;                          // slot 6
    virtual void Unknown7();                            // slot 7: 0x5C270. Name not in the reference map.
    virtual void Unknown8();                            // slot 8: 0x5CF90. Name not in the reference map.
    virtual void Unknown9();                            // slot 9: 0x5CFA0. Name not in the reference map.
    virtual ~Resource();                                // slots 10-11: 0x1ADD70
    virtual void Unknown12();                           // slot 12: 0x5D030. Name not in the reference map.

    void AddRef();      // 0x1ADEB0
    void ReleaseRef();  // 0x1ADEF0
    // Binds the resource to its path. At 0x1ACD40. Name not in the
    // reference map.
    void SetFile(ResourcePath path, bool unknown);
    // Looks up a loaded resource. At 0x1AB8E0.
    static Resource* Get(ResourcePath path);

    // Field names are not in the reference map.
    ResourcePath mPath;
    std::atomic<int> mRefs;
    unsigned short mUnknown20;
    unsigned char mUnknown22[18];
    int mUnknown40;
};

static_assert(offsetof(Resource, mPath) == 8);
static_assert(offsetof(Resource, mRefs) == 16);
static_assert(offsetof(Resource, mUnknown40) == 40);
static_assert(sizeof(Resource) == 48);

// Intrusive reference to a Resource.
template <class T>
class ResourcePtr {
public:
    ResourcePtr() : mResource(nullptr) {}
    explicit ResourcePtr(T* resource) : mResource(resource) {
        if (mResource != nullptr) {
            mResource->AddRef();
        }
    }
    ResourcePtr(const ResourcePtr& other) : ResourcePtr(other.mResource) {}
    ~ResourcePtr() {
        if (mResource != nullptr) {
            mResource->ReleaseRef();
        }
    }
    ResourcePtr& operator=(const ResourcePtr&) = delete;

    T* operator->() const {
        return mResource;
    }
    T* Get() const {
        return mResource;
    }
    explicit operator bool() const {
        return mResource != nullptr;
    }

    T* mResource;  // Name not in the reference map.
};
