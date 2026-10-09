#pragma once

#include <atomic>
#include <cstddef>

#include "os/files/File.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class BinStream;
class TextStream;

// Type information shared by every resource of one class, constructed at
// 0x1AF130 and filled by Init (0x1AF490, the map's Init(Symbol, Symbol,
// bool)). Field names are not in the reference map.
class ResourceMetaData {
public:
    // The file extensions of the type, such as "mp3" and "wav" for
    // FmodAudioStreamResource; Init registers each in the extension map.
    eastl::vector<Symbol> mExtensions;
    // An empty Symbol, four flags the constructor sets, three it clears and
    // an int. Init skips the extension registration when the flag at +44 is
    // set; Resource's companion-file check (0x1AD310) reads the one at +45.
    unsigned char mTypeSettings[24];
    // Returns the platform folder symbol; the constructor stores 0x1AF1A0,
    // which returns PlatformSymbol(7).
    Symbol (*mPlatformSymbolFunc)();
    Symbol mId;
    // Set when Init finishes.
    bool mInitialized;
    ResourceMetaData* mParent;
};

static_assert(offsetof(ResourceMetaData, mTypeSettings) == 32);
static_assert(offsetof(ResourceMetaData, mPlatformSymbolFunc) == 56);
static_assert(offsetof(ResourceMetaData, mId) == 64);
static_assert(offsetof(ResourceMetaData, mInitialized) == 72);
static_assert(offsetof(ResourceMetaData, mParent) == 80);

// Path of a loadable resource. The map places it with Resource in the entity
// module, which has not been reconstructed; only its storage is declared.
class ResourcePath {
public:
    ResourcePath() : mPath("") {}
    // Resolves the engine path. Inlined by its users, for example
    // RndDefaults::_LoadLighting at 0x6BEF7C.
    explicit ResourcePath(const char* path) : mPath("") {
        FileResolvePath(mPath, path);
    }

    Symbol mPath;  // Name not in the reference map.
};

template <class T>
class ResourcePtr;

// Reference-counted engine resource. The map places it in the entity module;
// until that module is reconstructed, only the members the FMOD resources use
// are declared here. The vtable has 13 slots.
class Resource {
public:
    // Inlined into each resource constructor, for example at 0x271660. The
    // engine path is resolved from the empty string.
    Resource()
        : mRefs(0), mNoCompanionFile(false), mFileChangedOnDisk(false), mFileTime(), mLoadTime(0) {
        FileResolvePath(mPath.mPath, "");
    }

    virtual ResourceMetaData* GetMetaData() const = 0;  // slot 0
    virtual Symbol GetId() const = 0;                   // slot 1
    virtual bool IsA(Symbol type) const = 0;            // slot 2
    virtual void LoadFile();                            // slot 3
    virtual bool Load(BinStream& stream, bool unknown);  // slot 4
    virtual void Save(BinStream& stream, bool unknown);  // slot 5
    virtual bool Fail() const;                          // slot 6
    // Slot 7 at 0x5C270: false here. When a resource has no generated
    // companion file, the loader at 0x1AD310 builds one only for types that
    // support it; two types override this to return !mNoCompanionFile.
    virtual bool SupportsCompanionFile() const;
    // Slots 8-9 at 0x5CF90 and 0x5CFA0 are never overridden or called in
    // this build. The map's vtable has twelve slots; slots 8-9 are matched
    // to its two CSV statistics members, a guess resting on their being
    // the remaining const members. Slot 8 returns zero.
    virtual int PrintCsvStatsHeader(TextStream& stream) const;
    virtual void PrintCsvStats(TextStream& stream) const;
    virtual ~Resource();                                // slots 10-11: 0x1ADD70
    // Slot 12 at 0x5D030, added after the map's build: false here. GetOrLoad
    // reloads a loaded resource when mFileChangedOnDisk is set or this
    // returns true. Name not in the reference map.
    virtual bool NeedsReload();

    void AddRef();      // 0x1ADEB0
    void ReleaseRef();  // 0x1ADEF0
    // Binds the resource to its path. At 0x1ACD40. Name not in the
    // reference map.
    void SetFile(ResourcePath path, bool unknown);
    // Looks up a loaded resource. At 0x1AB8E0.
    static Resource* Get(ResourcePath path);
    // Returns the resource at the path, loading it as the class with the id
    // when it is not loaded. The map's signature is
    // GetOrLoad(ResourcePath, Symbol); this build adds the flag.
    static ResourcePtr<Resource> GetOrLoad(
        ResourcePath path,
        Symbol type,
        bool unknown);  // 0x1ABA50
    // The typed form, instantiated where it is used: RndSceneResource's at
    // 0x6C0160. The map's signature is GetOrLoad<T>(ResourcePath); this
    // build adds the flag.
    template <class T>
    static ResourcePtr<T> GetOrLoad(ResourcePath path, bool unknown);

    // Field names are not in the reference map.
    ResourcePath mPath;
    std::atomic<int> mRefs;
    // Read by the SupportsCompanionFile overrides; no writer is identified,
    // so the name is weakly supported.
    bool mNoCompanionFile;
    // Set by the file watcher (0x1ACCEE) for a resource whose file changed;
    // GetOrLoad reloads the resource and clears it.
    bool mFileChangedOnDisk;
    // The file's modification time when it was loaded, as returned by
    // 0x378AA0; the loader compares it with the companion file's.
    unsigned long mFileTime[2];
    // How long LoadFile took, measured around it at 0x1AC4F4.
    int mLoadTime;
};

static_assert(offsetof(Resource, mPath) == 8);
static_assert(offsetof(Resource, mRefs) == 16);
static_assert(offsetof(Resource, mFileChangedOnDisk) == 21);
static_assert(offsetof(Resource, mFileTime) == 24);
static_assert(offsetof(Resource, mLoadTime) == 40);
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
    // Takes a reference to the new resource before releasing the old one,
    // as RndDefaults::Init does at 0x6BDCEC.
    ResourcePtr& operator=(T* resource) {
        if (resource != nullptr) {
            resource->AddRef();
        }
        if (mResource != nullptr) {
            mResource->ReleaseRef();
        }
        mResource = resource;
        return *this;
    }
    // Takes over the other reference, as RndDefaults::_LoadLighting does at
    // 0x6BEF90.
    ResourcePtr& operator=(ResourcePtr&& other) {
        T* const resource = other.mResource;
        other.mResource = nullptr;
        if (mResource != nullptr) {
            mResource->ReleaseRef();
        }
        mResource = resource;
        return *this;
    }

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

// The class's T::Id() is evaluated before the lookup; the binary repeats
// the evaluation for an assertion compiled out of this build.
template <class T>
ResourcePtr<T> Resource::GetOrLoad(ResourcePath path, bool unknown) {
    ResourcePtr<T> resource;
    resource = static_cast<T*>(GetOrLoad(path, T::Id(), unknown).Get());
    return resource;
}
