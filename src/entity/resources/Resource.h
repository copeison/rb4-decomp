#pragma once

#include <atomic>
#include <cstddef>

#include "entity/resources/ResourceMetaData.h"
#include "os/files/File.h"
#include "os/threading/CritSec.h"
#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class BinStream;
class Component;
class DataArray;
class FixedString;
class String;
class TextStream;

// Path of a loadable resource: a symbol of the path relative to the file
// root, optionally followed by "::<sub-object class>::<sub-object path>".
// The map has its methods in entity/Resource.o; this build places them after
// ResourceMetaData.o (0x1AF950-0x1AFFAB).
class ResourcePath {
public:
    ResourcePath() {}
    // Inlined by its users, for example RndDefaults::_LoadLighting at
    // 0x6BEF7C.
    explicit ResourcePath(const char* path) : mPath("") {
        *this = path;
    }

    // Normalizes the path and makes it relative to the file root. The
    // sub-object part is cut off while the path is resolved and put back
    // afterwards.
    ResourcePath& operator=(const char* path);  // 0x1AF950

    const char* Str() const {
        return mPath.Str();
    }
    bool operator==(const ResourcePath& other) const {
        return mPath == other.mPath;
    }
    bool operator!=(const ResourcePath& other) const {
        return mPath != other.mPath;
    }
    // Orders paths by their symbols, as the resource map does.
    bool operator<(const ResourcePath& other) const {
        return mPath < other.mPath;
    }

    // Builds "<file>::<sub-object class>::<sub-object path>", or the file
    // alone when there is no sub-object path, and resolves it.
    void SetFilepathAndSubObject(const char* file, Symbol subClass, const char* subPath);  // 0x1AFAA0
    // Copies the file part into `file` and returns the sub-object path, or
    // null when the path has no sub-object. `subClass`, when given, gets the
    // sub-object class.
    const char* GetFilepathAndSubObject(FixedString& file, Symbol* subClass) const;  // 0x1AFC70
    bool HasSubObject() const;  // 0x1AFDC0
    // The absolute path under the file root.
    const char* GetFullPath() const;  // 0x1AFDE0
    // Replaces the sub-object class of a path that has one.
    void FixSubObjectPath(Symbol subClass);  // 0x1AFF10

    Symbol mPath;  // Name not in the reference map.
};

static_assert(sizeof(ResourcePath) == 8);

template <class T>
class ResourcePtr;

// Reference-counted engine resource (entity/Resource.o, 0x1AB350-0x1AF10A).
// Every resource with a path is in the loaded-resource map, which a
// critical section guards together with the reference counts. The vtable
// is at 0x18E8A18 and has 13 slots.
class Resource {
public:
    // The component whose resources the calling thread is loading;
    // LoadFile reports a failed load against it. The map has the type and
    // the thread-local TLSValue<Resource::LoadContext> gLoadContext; the
    // field name is not in the map.
    struct LoadContext {
        Component* mComponent;
    };

    // Creates a resource of a registered class.
    using Factory = Resource* (*)();  // Name not in the reference map.

    // Inlined into each resource constructor, for example at 0x271660. The
    // path is resolved from the empty string.
    Resource()
        : mRefs(0), mInlined(false), mFileChangedOnDisk(false), mFileTime(), mLoadSize(0) {
        mPath = "";
    }

    virtual ResourceMetaData* GetMetaData() const = 0;  // slot 0
    virtual Symbol GetId() const = 0;                   // slot 1
    virtual bool IsA(Symbol type) const = 0;            // slot 2
    // Slot 3. Loads the resource from its cached file, rebuilding the cache
    // from the source file when it is stale or missing.
    virtual void LoadFile();                            // 0x1AD310
    // Slots 4-6. Load reads the file: `cached` is set for a cached file
    // and clear for the source. Save writes the cached file.
    virtual bool Load(BinStream& stream, bool cached) = 0;  // slot 4
    virtual void Save(BinStream& stream, bool cached) = 0;  // slot 5
    virtual bool Fail() const = 0;                          // slot 6
    // Slot 7 at 0x5C270: false here. When a resource has no cached companion
    // file, LoadFile compares the companion's time only for types that
    // support it; two types override this to return !mInlined.
    virtual bool SupportsCompanionFile() const {
        return false;
    }
    // Slots 8-9 at 0x5CF90 and 0x5CFA0 are never overridden or called in
    // this build; the map has them in Resource.o. Slot 8 returns zero.
    virtual int PrintCsvStatsHeader(TextStream& stream) const {
        static_cast<void>(stream);
        return 0;
    }
    virtual void PrintCsvStats(TextStream& stream) const {
        static_cast<void>(stream);
    }
    // Slots 10-11: 0x1ADD70, 0x1ADE90. Removes the resource from the map.
    virtual ~Resource();
    // Slot 12 at 0x5D030, added after the map's build: false here. GetOrLoad
    // reloads a loaded resource when mFileChangedOnDisk is set or this
    // returns true. Name not in the reference map.
    virtual bool NeedsReload() {
        return false;
    }

    // Registers the base class's metadata and reads the precache options.
    static void Init();  // 0x1AB3A0
    // Reads the "non_cached_folders" list of the config's "resource" block.
    // Called by SystemInit. Name not in the reference map.
    static void InitNonCachedFolders(DataArray* config);  // 0x1AB4B0
    // The folders read by InitNonCachedFolders, each with a trailing
    // slash. No caller remains in this build. Name not in the reference
    // map.
    static eastl::vector<Symbol>& GetNonCachedFolders();  // 0x1AB390

    // The class id, created on first use and inlined into its users. The
    // local static is at 0x19C7488.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("Resource");
        }
        return id;
    }

    // Creates a resource of the class, or of the first registered class
    // derived from it; null when there is none. The release build does not
    // read `fail`.
    static Resource* New(Symbol type, bool fail);  // 0x1AB7F0
    // Looks up a loaded resource.
    static ResourcePtr<Resource> Get(ResourcePath path);  // 0x1AB8E0
    // Returns the resource at the path, loading it as the class with the id
    // when it is not loaded or must be reloaded. The map's signature is
    // GetOrLoad(ResourcePath, Symbol); this build adds the flag, which
    // LoadUnique uses.
    static ResourcePtr<Resource> GetOrLoad(ResourcePath path, Symbol type, bool ignoreExt);  // 0x1ABA50
    // The typed form, instantiated where it is used: RndSceneResource's at
    // 0x6C0160. The map's signature is GetOrLoad<T>(ResourcePath); this
    // build adds the flag.
    template <class T>
    static ResourcePtr<T> GetOrLoad(ResourcePath path, bool ignoreExt);
    // Creates and loads a new resource at the path, outside the map. The
    // class comes from the path's extension unless the path has a
    // sub-object or no extension, or `ignoreExt` is set; then the class is
    // the one with the id. The map's signature is
    // LoadUnique(ResourcePath, Symbol); this build adds the flag.
    static ResourcePtr<Resource> LoadUnique(ResourcePath path, Symbol type, bool ignoreExt);  // 0x1AC010
    // Loads every loaded resource again as a new copy, which writes its
    // cached file.
    static void ForceCacheLoadedResources();  // 0x1AC550
    // Appends the paths of the loaded resources that did not fail. No
    // caller remains in this build. Name not in the reference map.
    static void GetLoadedResources(eastl::vector<ResourcePath>& paths);  // 0x1AC710
    static LoadContext GetLoadContext();  // 0x1AC8E0
    // Marks the loaded resources that depend on the changed file for
    // reloading.
    static void FileChangedOnDisk(const String& file);  // 0x1AC9F0
    static void SetLoadContext(const LoadContext& context);  // 0x1AC960

    void AddRef();      // 0x1ADEB0
    // Deletes the resource with its last reference. Nothing is released
    // once the program is exiting.
    void ReleaseRef();  // 0x1ADEF0
    // Binds the resource to the path in the map. With `replace`, the map
    // entry of the old path is removed when it names this resource.
    void SetFile(ResourcePath path, bool replace);  // 0x1ACD40
    // Writes the cached file through a temporary file. The path becomes the
    // resource's when it has none.
    bool Save(const char* path);  // 0x1AD040
    // Sets the path and loads the file under the load lock.
    void Load(const char* path);  // 0x1AD280

    // The base class's metadata.
    static ResourceMetaData sMetaData;  // 0x19E4580
    // Each registered class's factory, by class id.
    static eastl::map<Symbol, Factory> sFactory;  // 0x19E4520
    // The loaded resources by path.
    static eastl::map<ResourcePath, Resource*> sResources;  // 0x19E44E8
    // Guards sResources and the reference counts. Name not in the
    // reference map.
    static CritSec sResourcesCritSec;  // 0x19E4560
    // Held while resources load. Name not in the reference map.
    static CritSec sLoadCritSec;  // 0x19E4570
    // The resource being deleted by its last ReleaseRef. Name not in the
    // reference map.
    static Resource* sDeleting;  // 0x19E45F8

    // Field names are not in the reference map.
    ResourcePath mPath;
    std::atomic<int> mRefs;
    // Set while the resource is inlined into an entity resource's layer;
    // EntityResource clears it when it releases its inlined resources
    // (0xFDB10), and Entity::MakeErrorName (0xF0DD0) then names the owner.
    // The two SupportsCompanionFile overrides return its inverse.
    bool mInlined;
    // Set by the file watcher (0x1ACCEE) for a resource whose file changed;
    // GetOrLoad reloads the resource and clears it.
    bool mFileChangedOnDisk;
    // The source file's time when the resource was loaded, which LoadFile
    // compares with the cached file's.
    FileStat mFileTime;
    // The bytes allocated while LoadFile ran, from MemGetStats.
    int mLoadSize;

private:
    // Sets the path and loads the file, measuring the memory it takes.
    void _SetFileAndLoad(ResourcePath path);  // 0x1AC450
};

static_assert(offsetof(Resource, mPath) == 8);
static_assert(offsetof(Resource, mRefs) == 16);
static_assert(offsetof(Resource, mFileChangedOnDisk) == 21);
static_assert(offsetof(Resource, mFileTime) == 24);
static_assert(offsetof(Resource, mLoadSize) == 40);
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
    ResourcePtr(ResourcePtr&& other) : mResource(other.mResource) {
        other.mResource = nullptr;
    }
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
    operator T*() const {
        return mResource;
    }

    T* mResource;  // Name not in the reference map.
};

// The class's T::Id() is evaluated before the lookup; the binary repeats
// the evaluation for an assertion compiled out of this build.
template <class T>
ResourcePtr<T> Resource::GetOrLoad(ResourcePath path, bool ignoreExt) {
    ResourcePtr<T> resource;
    resource = static_cast<T*>(GetOrLoad(path, T::Id(), ignoreExt).Get());
    return resource;
}

// Records every path GetOrLoad and SetFile see while it exists. Nothing in
// this build creates one, so the names are weakly supported. Names not in
// the reference map.
class ResourcePathRecorder {
public:
    // Becomes the current recorder.
    ResourcePathRecorder();   // 0x1AEBC0
    ~ResourcePathRecorder();  // 0x1AEC00

    // Creates the current recorder, or returns null when one exists.
    static ResourcePathRecorder* Begin();  // 0x1AEB20
    // Deletes the recorder, which stops recording.
    static void End(ResourcePathRecorder* recorder);  // 0x1AEB70

    // Appends the path unless it is already recorded. Inlined into
    // GetOrLoad and SetFile.
    void Record(ResourcePath path) {
        for (const ResourcePath& recorded : mPaths) {
            if (recorded == path) {
                return;
            }
        }
        mPaths.push_back(path);
    }

    eastl::vector<ResourcePath> mPaths;

    static ResourcePathRecorder* sCurrent;  // 0x19E44E0
};

static_assert(sizeof(ResourcePathRecorder) == 32);

// The path a resource's file is read from. In precached mode, a path that is
// not local gains the platform folder ("%s/%s" with PlatformSymbol(7))
// unless it already contains it.
const char* GetUncachedResourcePath(ResourcePath path);  // 0x1AE5B0
// The source file's time. Returns false for an empty path. The map's
// signature is GetUncachedFileTimestamp(ResourcePath, long&).
bool GetUncachedFileTimestamp(ResourcePath path, FileStat& stat);  // 0x1AE770
// The cached file of a source path: its name with ':' made '+', the
// extension and a platform suffix, under the build data folder. Paths
// already under the data folder are their own cache. The map's signature is
// GetCachedResourcePath(ResourcePath, char const*); this build adds
// `platformSuffix`, which adds the platform to names of unregistered types.
const char* GetCachedResourcePath(ResourcePath path, const char* ext, bool platformSuffix);  // 0x1ADF80
// The cached file of the source path into `cachedPath`, whether it must be
// rebuilt because it is missing or not newer than the source, and its time.
// Returns false when the source is missing. Creates the cached file's
// folder when the cache does not exist. The map's signature ends in
// unsigned long& rather than FileStat&.
bool CheckCache(
    ResourcePath path,
    const char* ext,
    FixedString& cachedPath,
    bool& rebuild,
    FileStat& cachedTime);  // 0x1AD8B0
// The companion file of a resource: "<path>.dta", or
// "<dir>/<base>__<sub-object path>.<ext>.dta" for a sub-object.
const char* MakeResourceCompanionFilepath(ResourcePath path);  // 0x1ADAB0
// Copies the source file to its cached path and returns that path. No
// caller remains in this build; the match rests on its place after
// GetUncachedFileTimestamp, as in the map.
const char* CopyToCache(ResourcePath path);  // 0x1AE910
// False in this build. No caller remains; the match rests on its size and
// place, so it is weakly supported.
bool ResourceFileIsLocal(ResourcePath path);  // 0x1AEB10
// The platform folder symbol. The map has it in Resource.o; this build's
// copy is not called.
Symbol _GetPlatformSym();  // 0x1ADF70
