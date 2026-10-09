#include "entity/resources/Resource.h"

#include <string.h>

#include "entity/core/Component.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityConstants.h"
#include "entity/core/EntityResource.h"
#include "entity/resources/DataResource.h"
#include "os/debug/Debug.h"
#include "os/memory/MemMgr.h"
#include "os/platform/PlatformMgr.h"
#include "render/system/RndInit.h"
#include "utl/data/DataArray.h"
#include "utl/files/FileUtl.h"
#include "utl/options/Option.h"
#include "utl/streams/FileStream.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"

// The map's Resource::sPrecached and Resource::sPrecaching; see File.h.
unsigned char gFileArchiveMode = 0;  // 0x19E4558
bool gResourcePrecacheMode = false;  // 0x19E4559

// The globals at 0x19E44E0 through 0x19E45F8, in the binary's order.
ResourcePathRecorder* ResourcePathRecorder::sCurrent;
eastl::map<ResourcePath, Resource*> Resource::sResources;
eastl::map<Symbol, Resource::Factory> Resource::sFactory;
CritSec Resource::sResourcesCritSec;
CritSec Resource::sLoadCritSec;
ResourceMetaData Resource::sMetaData;
Resource* Resource::sDeleting;

namespace {

// The calling thread's load context. The map's TLSValue<Resource::LoadContext>
// gLoadContext; this build keeps it in the per-thread block at the offset
// recorded at 0x19B0328.
thread_local Resource::LoadContext gLoadContext;

// The folders read by Resource::InitNonCachedFolders. Name not in the
// reference map.
eastl::vector<Symbol> gNonCachedFolders;  // 0x19E45D8

// The cached data folders, relative to the file root. Names not in the
// reference map.
constexpr const char kBuildDataFolder[] = "../build/data";
constexpr const char kPrecachedDataFolder[] = "../build/data_precached";

// Reports a failed load against the component that is loading, when there is
// one. The message is formatted and then dropped: the failure report itself
// is compiled out of this build. Inlined into LoadFile. Name not in the
// reference map.
void ReportLoadFailure() {
    if (Component* component = Resource::GetLoadContext().mComponent) {
        static_cast<void>(component->MakeErrorName());
    }
}

// The registered factory for the class, or for the first registered class
// derived from it. Inlined into New and LoadUnique. Name not in the
// reference map.
Resource::Factory FindFactory(Symbol type) {
    const auto it = Resource::sFactory.find(type);
    if (it != Resource::sFactory.end()) {
        return it->second;
    }
    for (auto& entry : Resource::sFactory) {
        for (const ResourceMetaData* metaData = ResourceMetaData::GetMetaData(entry.first, false);
             metaData != nullptr;
             metaData = metaData->mParent) {
            if (metaData->mId == type) {
                return entry.second;
            }
        }
    }
    return nullptr;
}

}  // namespace

// Reconstructed from eboot.elf at 0x1AB390.
eastl::vector<Symbol>& Resource::GetNonCachedFolders() {
    return gNonCachedFolders;
}

// Reconstructed from eboot.elf at 0x1AB3A0.
void Resource::Init() {
    sMetaData.mTypeFlags[0] = false;
    sMetaData.Init(Id(), Symbol(""), false);
    gFileArchiveMode = OptionBool(gOptionArgs, "precached", false);
    gResourcePrecacheMode = OptionBool(gOptionArgs, "precache", false);
}

// Reconstructed from eboot.elf at 0x1AB4B0. Each folder gains a trailing
// slash.
void Resource::InitNonCachedFolders(DataArray* config) {
    const Symbol resource("resource");
    const Symbol nonCachedFolders("non_cached_folders");
    DataArray* const block = config->FindArray(resource, false);
    if (block == nullptr) {
        return;
    }
    DataArray* const folders = block->FindArray(nonCachedFolders, false);
    if (folders == nullptr) {
        return;
    }
    gNonCachedFolders.reserve(folders->mSize);
    for (int index = 1; index < folders->mSize; ++index) {
        StackString<512> folder(folders->Str(index));
        folder.append("/", 1);
        gNonCachedFolders.push_back(Symbol(folder.c_str()));
    }
}

// Reconstructed from eboot.elf at 0x1AB7F0.
Resource* Resource::New(Symbol type, bool fail) {
    static_cast<void>(fail);
    const Factory factory = FindFactory(type);
    return factory != nullptr ? factory() : nullptr;
}

// Reconstructed from eboot.elf at 0x1AB8E0.
ResourcePtr<Resource> Resource::Get(ResourcePath path) {
    ResourcePtr<Resource> resource;
    sResourcesCritSec.Enter();
    const auto it = sResources.find(path);
    if (it != sResources.end()) {
        resource = it->second;
    }
    sResourcesCritSec.Exit();
    return resource;
}

// Reconstructed from eboot.elf at 0x1ABA50. A loaded resource is returned
// unless its file changed or it asks to be reloaded. A reload that fails
// keeps the loaded resource; any other load result replaces the map entry.
ResourcePtr<Resource> Resource::GetOrLoad(ResourcePath path, Symbol type, bool ignoreExt) {
    if (path.mPath == Symbol()) {
        return ResourcePtr<Resource>();
    }
    if (ResourcePathRecorder::sCurrent != nullptr) {
        ResourcePathRecorder::sCurrent->Record(path);
    }
    ResourcePtr<Resource> resource = Get(path);
    if (resource && !resource->mFileChangedOnDisk && !resource->NeedsReload()) {
        return resource;
    }

    sLoadCritSec.Enter();
    resource = Get(path);
    if (resource && !resource->mFileChangedOnDisk && !resource->NeedsReload()) {
        sLoadCritSec.Exit();
        return resource;
    }
    ResourcePtr<Resource> loaded = LoadUnique(path, type, ignoreExt);
    ResourcePtr<Resource> result;
    if (!resource || (loaded && !loaded->Fail())) {
        sResourcesCritSec.Enter();
        sResources[path] = loaded.Get();
        sResourcesCritSec.Exit();
        result = static_cast<ResourcePtr<Resource>&&>(loaded);
    } else {
        resource->mFileChangedOnDisk = false;
        result = static_cast<ResourcePtr<Resource>&&>(resource);
    }
    sLoadCritSec.Exit();
    return result;
}

// Reconstructed from eboot.elf at 0x1AC010. A class picked by extension must
// be the requested class or derive from it; the base class itself is never
// created.
ResourcePtr<Resource> Resource::LoadUnique(ResourcePath path, Symbol type, bool ignoreExt) {
    if (path.mPath == Symbol()) {
        return ResourcePtr<Resource>();
    }
    sLoadCritSec.Enter();
    ResourcePtr<Resource> resource;
    const char* const ext = FileGetExt(path.Str(), false);
    if (!path.HasSubObject() && *ext != '\0' && !ignoreExt) {
        const Symbol extType = ResourceMetaData::GetIdFromExt(Symbol(ext));
        if (extType == Symbol()) {
            sLoadCritSec.Exit();
            return resource;
        }
        resource = New(extType, false);
        if (!resource->IsA(type)) {
            resource = nullptr;
            sLoadCritSec.Exit();
            return resource;
        }
    } else {
        if (ResourceMetaData::GetMetaData(type, false) == nullptr || type == Id()) {
            sLoadCritSec.Exit();
            return resource;
        }
        resource = New(type, false);
    }
    resource->_SetFileAndLoad(path);
    sLoadCritSec.Exit();
    return resource;
}

// Reconstructed from eboot.elf at 0x1AC450.
void Resource::_SetFileAndLoad(ResourcePath path) {
    SetFile(path, false);
    unsigned long bytesBefore;
    int allocs;
    int frees;
    MemGetStats(bytesBefore, allocs, frees);
    LoadFile();
    unsigned long bytesAfter;
    MemGetStats(bytesAfter, allocs, frees);
    mLoadSize = bytesAfter > bytesBefore ? static_cast<int>(bytesAfter - bytesBefore) : 0;
}

// Reconstructed from eboot.elf at 0x1AC550.
void Resource::ForceCacheLoadedResources() {
    sLoadCritSec.Enter();
    sResourcesCritSec.Enter();
    for (auto& entry : sResources) {
        LoadUnique(entry.first, Id(), false);
    }
    sResourcesCritSec.Exit();
    sLoadCritSec.Exit();
}

// Reconstructed from eboot.elf at 0x1AC710.
void Resource::GetLoadedResources(eastl::vector<ResourcePath>& paths) {
    sResourcesCritSec.Enter();
    paths.reserve(sResources.size());
    for (auto& entry : sResources) {
        if (entry.second != nullptr && !entry.second->Fail()) {
            paths.push_back(entry.first);
        }
    }
    sResourcesCritSec.Exit();
}

// Reconstructed from eboot.elf at 0x1AC8E0.
Resource::LoadContext Resource::GetLoadContext() {
    return gLoadContext;
}

// Reconstructed from eboot.elf at 0x1AC960.
void Resource::SetLoadContext(const LoadContext& context) {
    gLoadContext = context;
}

// Reconstructed from eboot.elf at 0x1AC9F0. Marks every loaded resource that
// depends on the changed file: one loaded from it, one whose sub-object
// lives in it, an entity resource with a layer in it, or a data resource
// that includes it. No caller remains in this build.
void Resource::FileChangedOnDisk(const String& file) {
    ResourcePath changed;
    changed = file.c_str();
    sResourcesCritSec.Enter();
    StackString<512> filePath;
    for (auto& entry : sResources) {
        Resource* const resource = entry.second;
        if (resource == nullptr) {
            continue;
        }
        bool depends = resource->mPath == changed;
        if (!depends && resource->mPath.GetFilepathAndSubObject(filePath, nullptr) != nullptr) {
            depends = filePath == changed.mPath;
        } else if (!depends) {
            if (resource->IsA(EntityResource::Id())
                && strcmp(FileGetExt(changed.Str(), false), kLayerExt) == 0) {
                const Entity* const entity = static_cast<EntityResource*>(resource)->mEntity;
                if (entity != nullptr) {
                    for (unsigned long layer = 0; layer < kMaxLayers; ++layer) {
                        if (layer != kMainLayerIndex && entity->LayerExists(layer)
                            && entity->GetLayerPath(layer) == changed) {
                            depends = true;
                            break;
                        }
                    }
                }
            } else if (resource->IsA(DataResource::Id())) {
                for (const ResourcePath& dependency : static_cast<DataResource*>(resource)->mFiles) {
                    if (dependency == changed) {
                        depends = true;
                        break;
                    }
                }
            }
        }
        if (depends) {
            resource->mFileChangedOnDisk = true;
        }
    }
    sResourcesCritSec.Exit();
}

// Reconstructed from eboot.elf at 0x1ACD40. The old path's assertion that
// no other resource holds the path is compiled out, leaving its lookup.
void Resource::SetFile(ResourcePath path, bool replace) {
    if (mPath == path && !replace) {
        return;
    }
    if (!replace) {
        sResourcesCritSec.Enter();
        sResourcesCritSec.Exit();
        mPath = path;
        return;
    }
    sResourcesCritSec.Enter();
    const auto old = sResources.find(mPath);
    if (old != sResources.end() && old->second == this) {
        sResources.erase(old);
    }
    sResources[path] = this;
    if (ResourcePathRecorder::sCurrent != nullptr) {
        ResourcePathRecorder::sCurrent->Record(path);
    }
    sResourcesCritSec.Exit();
    mPath = path;
}

// Reconstructed from eboot.elf at 0x1AD040. The file must be writable; the
// data is written to "<path>_temp", which then replaces it.
bool Resource::Save(const char* path) {
    if (Fail()) {
        return false;
    }
    if (mPath.mPath == Symbol()) {
        SetFile(ResourcePath(path), false);
    }
    bool failed;
    {
        FileStream test(path, kAppend, false);
        failed = test.Fail();
    }
    if (failed) {
        return false;
    }
    StackString<512> tempPath(path);
    tempPath += "_temp";
    {
        FileStream stream(tempPath.c_str(), kWrite, false);
        if (stream.Fail()) {
            return false;
        }
        Save(stream, false);
    }
    return FileRename(tempPath.c_str(), path, true);
}

// Reconstructed from eboot.elf at 0x1AD280.
void Resource::Load(const char* path) {
    sLoadCritSec.Enter();
    _SetFileAndLoad(ResourcePath(path));
    sLoadCritSec.Exit();
}

// Reconstructed from eboot.elf at 0x1AD310. The cached file is read when it
// is current, or in precached mode; otherwise the source file is loaded and
// the cache rewritten through "<cache>_temp". Types with mTypeFlags[5] and
// local files are never rebuilt.
void Resource::LoadFile() {
    StackString<512> cachedPath;
    bool rebuild;
    if (!CheckCache(mPath, "", cachedPath, rebuild, mFileTime)) {
        ReportLoadFailure();
        return;
    }

    bool load = true;
    bool readCache = gFileArchiveMode != 0;
    if (!rebuild) {
        // A companion file newer than the source forces a rebuild.
        bool companionNewer = false;
        if (SupportsCompanionFile() && gFileArchiveMode == 0) {
            StackString<512> companion(MakeResourceCompanionFilepath(mPath));
            const FileStat companionTime = FileTimestamp(companion.c_str());
            if (companionTime.mSeconds != 0 || companionTime.mFraction != 0) {
                companionNewer = mFileTime.mSeconds == companionTime.mSeconds
                    ? mFileTime.mFraction <= companionTime.mFraction
                    : mFileTime.mSeconds < companionTime.mSeconds;
            }
        }
        if (companionNewer) {
            rebuild = true;
        } else {
            readCache = true;
        }
    }

    if (readCache) {
        FileStream stream(cachedPath.c_str(), kRead, false);
        if (stream.Fail()) {
            rebuild = gFileArchiveMode == 0;
        } else if (stream.Size() == 0) {
            if (gFileArchiveMode == 0) {
                rebuild = true;
            }
        } else {
            rebuild = !Load(stream, true);
        }
        load = rebuild;
    }
    if (!load || gFileArchiveMode != 0) {
        return;
    }

    if (ResourceMetaData::GetMetaData(GetId(), true)->mTypeFlags[5]) {
        // The id was for the failure report, which is compiled out.
        static_cast<void>(GetId());
        return;
    }
    StackString<512> filePath;
    mPath.GetFilepathAndSubObject(filePath, nullptr);
    if (FileIsLocal(ResourcePath(filePath.c_str()).Str())) {
        return;
    }
    FileStream source(filePath.c_str(), kRead, false);
    if (source.Fail()) {
        ReportLoadFailure();
        return;
    }
    if (!Load(source, false)) {
        return;
    }
    StackString<512> tempPath(cachedPath.c_str());
    tempPath += "_temp";
    bool saved = false;
    {
        FileStream stream(tempPath.c_str(), kWrite, false);
        if (!stream.Fail()) {
            Save(stream, true);
            saved = true;
        }
    }
    if (saved) {
        FileRename(tempPath.c_str(), cachedPath.c_str(), true);
    }
    mFileTime = FileTimestamp(cachedPath.c_str());
}

// Reconstructed from eboot.elf at 0x1AD8B0. Local cached files are current
// when they exist; precached ones always are.
bool CheckCache(
    ResourcePath path,
    const char* ext,
    FixedString& cachedPath,
    bool& rebuild,
    FileStat& cachedTime) {
    FileStat sourceTime = {};
    if (!GetUncachedFileTimestamp(path, sourceTime)) {
        return false;
    }
    cachedPath = GetCachedResourcePath(path, ext, true);
    const bool local = FileIsLocal(ResourcePath(cachedPath.c_str()).Str());
    const bool noSource = sourceTime.mSeconds == 0 && sourceTime.mFraction == 0;
    if (noSource && !local && gFileArchiveMode == 0) {
        return false;
    }
    if (local) {
        if (FileExists(cachedPath.c_str(), kReadNoArk)) {
            rebuild = false;
            return true;
        }
        rebuild = !noSource;
        if (!rebuild) {
            return true;
        }
    } else {
        if (gFileArchiveMode != 0) {
            rebuild = false;
            return true;
        }
        cachedTime = FileTimestamp(cachedPath.c_str());
        if (cachedTime.mSeconds == 0 && cachedTime.mFraction == 0) {
            rebuild = true;
        } else {
            rebuild = cachedTime.mSeconds == sourceTime.mSeconds
                ? cachedTime.mFraction <= sourceTime.mFraction
                : cachedTime.mSeconds < sourceTime.mSeconds;
            if (!rebuild) {
                return true;
            }
        }
    }
    if (cachedTime.mSeconds == 0 && cachedTime.mFraction == 0) {
        char folder[512];
        FileGetPath(cachedPath.c_str(), folder);
        FileMkDirRecur(folder, "");
    }
    return true;
}

// Reconstructed from eboot.elf at 0x1ADAB0.
const char* MakeResourceCompanionFilepath(ResourcePath path) {
    StackString<512> filePath;
    const char* const subPath = path.GetFilepathAndSubObject(filePath, nullptr);
    if (subPath == nullptr) {
        FormatString format("%s.dta");
        format << filePath;
        return format.Str();
    }
    char folder[512];
    char base[512];
    const char* const dir = FileGetPath(filePath.c_str(), folder);
    const char* const name = FileGetBase(filePath.c_str(), base);
    const char* const ext = FileGetExt(filePath.c_str(), false);
    FormatString format("%s/%s__%s.%s.dta");
    format << dir << name << subPath << ext;
    return format.Str();
}

// Reconstructed from eboot.elf at 0x1ADD70.
Resource::~Resource() {
    if (mPath.mPath == Symbol()) {
        return;
    }
    sResourcesCritSec.Enter();
    const auto it = sResources.find(mPath);
    if (it != sResources.end() && it->second == this) {
        sResources.erase(it);
    }
    sResourcesCritSec.Exit();
}

// Reconstructed from eboot.elf at 0x1ADEB0.
void Resource::AddRef() {
    sResourcesCritSec.Enter();
    ++mRefs;
    sResourcesCritSec.Exit();
}

// Reconstructed from eboot.elf at 0x1ADEF0.
void Resource::ReleaseRef() {
    if (TheDebug.mExiting) {
        return;
    }
    sResourcesCritSec.Enter();
    if (mRefs.fetch_sub(1) == 1) {
        sDeleting = this;
        delete this;
        sDeleting = nullptr;
    }
    sResourcesCritSec.Exit();
}

// Reconstructed from eboot.elf at 0x1ADF70.
Symbol _GetPlatformSym() {
    return PlatformSymbol(kPlatformPS4);
}

// Reconstructed from eboot.elf at 0x1ADF80. Shader sources ("hlsl") gain the
// platform and graphics API.
const char* GetCachedResourcePath(ResourcePath path, const char* ext, bool platformSuffix) {
    StackString<512> filePath;
    const char* const subPath = path.GetFilepathAndSubObject(filePath, nullptr);
    if (filePath.c_str()[0] == '\0') {
        return Symbol().Str();
    }
    char localized[512];
    StackString<512> localizedPath(FileLocalize(filePath.c_str(), localized));
    filePath = localizedPath.c_str();
    char folder[512];
    FileGetPath(filePath.c_str(), folder);

    const char* const dataFolder = gResourcePrecacheMode ? kPrecachedDataFolder : kBuildDataFolder;
    const unsigned long dataFolderLength = gResourcePrecacheMode
        ? sizeof(kPrecachedDataFolder) - 1
        : sizeof(kBuildDataFolder) - 1;
    if (strncmp(folder, dataFolder, dataFolderLength) == 0) {
        return path.Str();
    }
    const char* relative = strncmp(folder, "../../", 6) == 0 ? folder + 6 : folder;
    const Symbol platform = PlatformSymbol(kPlatformPS4);
    const unsigned long platformLength = strlen(platform.Str());
    if (strncmp(relative, platform.Str(), platformLength) == 0 && relative[platformLength] == '/') {
        relative += platformLength + 1;
    }

    StackString<512> name(FileGetName(path.Str()));
    name.ReplaceAll(':', '+');
    name += ext;
    const Symbol type = ResourceMetaData::GetIdFromPath(ResourcePath(path.Str()));
    if (subPath != nullptr || type == Symbol()) {
        if (strcmp(FileGetExt(name.c_str(), false), "hlsl") == 0) {
            name += "_";
            name += platform.Str();
            name += GfxApiSymbol(Rnd::PlatformGfxApi()).Str();
        } else if (platformSuffix) {
            name += "_";
            name += platform.Str();
        }
    } else {
        const ResourceMetaData* const metaData = ResourceMetaData::GetMetaData(type, true);
        if (!metaData->mTypeFlags[6]) {
            name += "_";
            name += metaData->mPlatformSymbolFunc().Str();
        }
    }

    if (FileIsLocal(ResourcePath(relative).Str())) {
        FormatString format("%s/%s");
        format << relative << name;
        return format.Str();
    }
    if (gFileArchiveMode != 0) {
        FormatString format("%s/%s/%s");
        format << platform << relative << name;
        return format.Str();
    }
    FormatString format("%s/%s/%s/%s");
    format << dataFolder << platform << relative << name;
    return format.Str();
}

// Reconstructed from eboot.elf at 0x1AE4D0.
Symbol GfxApiSymbol(HxGfxApi api) {
    static Symbol sNames[kNumGfxApis] = {
        Symbol("null"),
        Symbol("dx11"),
        Symbol("ps4"),
        Symbol("mtl"),
        Symbol("vlk"),
        Symbol("nx"),
        Symbol("gles3"),
    };
    return api < kNumGfxApis ? sNames[api] : Symbol();
}

// Reconstructed from eboot.elf at 0x1AE5B0.
const char* GetUncachedResourcePath(ResourcePath path) {
    const char* result = path.Str();
    if (gFileArchiveMode == 0 || FileIsLocal(ResourcePath(result).Str())) {
        return result;
    }
    const Symbol platform = PlatformSymbol(kPlatformPS4);
    FormatString prefixFormat("%s/");
    prefixFormat << platform;
    StackString<64> prefix(prefixFormat.Str());
    if (strstr(result, prefix.c_str()) == nullptr) {
        FormatString format("%s/%s");
        format << platform << path.mPath;
        result = format.Str();
    }
    return result;
}

// Reconstructed from eboot.elf at 0x1AE770.
bool GetUncachedFileTimestamp(ResourcePath path, FileStat& stat) {
    StackString<512> filePath;
    path.GetFilepathAndSubObject(filePath, nullptr);
    if (filePath.c_str()[0] == '\0') {
        return false;
    }
    char localized[512];
    StackString<512> localizedPath(FileLocalize(filePath.c_str(), localized));
    filePath = localizedPath.c_str();
    stat = FileTimestamp(filePath.c_str());
    return true;
}

// Reconstructed from eboot.elf at 0x1AE910. The absolute-path check's
// result was for an assertion that is compiled out.
const char* CopyToCache(ResourcePath path) {
    static_cast<void>(FileIsAbsolute(path.Str()));
    StackString<512> cachedPath(GetCachedResourcePath(path, "", false));
    char folder[512];
    FileGetPath(cachedPath.c_str(), folder);
    FileMkDirRecur(folder, "");
    FileCopy(path.Str(), cachedPath.c_str(), true);
    FormatString format("%s");
    format << cachedPath;
    return format.Str();
}

// Reconstructed from eboot.elf at 0x1AEB10.
bool ResourceFileIsLocal(ResourcePath path) {
    static_cast<void>(path);
    return false;
}

// Reconstructed from eboot.elf at 0x1AEB20.
ResourcePathRecorder* ResourcePathRecorder::Begin() {
    if (sCurrent != nullptr) {
        return nullptr;
    }
    return new ResourcePathRecorder();
}

// Reconstructed from eboot.elf at 0x1AEB70.
void ResourcePathRecorder::End(ResourcePathRecorder* recorder) {
    delete recorder;
}

// Reconstructed from eboot.elf at 0x1AEBC0.
ResourcePathRecorder::ResourcePathRecorder() {
    sCurrent = this;
}

// Reconstructed from eboot.elf at 0x1AEC00.
ResourcePathRecorder::~ResourcePathRecorder() {
    sCurrent = nullptr;
}
