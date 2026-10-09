#include "entity/resources/ResourceMetaData.h"

#include "entity/resources/Resource.h"
#include "os/memory/MemMgr.h"
#include "os/platform/PlatformMgr.h"
#include "utl/files/FileUtl.h"

// The globals at 0x19E4658 and 0x19E4678.
eastl::vector<ResourceMetaData*> ResourceMetaData::sResourceMetaDataList;
eastl::map<Symbol, Symbol> ResourceMetaData::sExtToId;

namespace {

// Reconstructed from eboot.elf at 0x1AF1A0. The default platform suffix of
// cached files. Name not in the reference map.
Symbol DefaultPlatformSymbol() {
    return PlatformSymbol(kPlatformPS4);
}

// The heap the registrations allocate from, found on first use; Init keeps
// one lookup for each of its two allocations. Name not in the reference map.
long MetaDataHeap() {
    return MemFindHeap("metadata");
}

}  // namespace

// Reconstructed from eboot.elf at 0x1AF130.
ResourceMetaData::ResourceMetaData()
    : mCategory(),
      mTypeFlags{true, true, true, true, false, false, false},
      mTypeOption(0),
      mPlatformSymbolFunc(DefaultPlatformSymbol),
      mId(),
      mInitialized(false) {}

// Reconstructed from eboot.elf at 0x1AF1B0.
eastl::vector<ResourceMetaData*>& ResourceMetaData::GetAllResourceMetaData() {
    return sResourceMetaDataList;
}

// Reconstructed from eboot.elf at 0x1AF1C0.
ResourceMetaData* ResourceMetaData::GetMetaData(Symbol id, bool fail) {
    static_cast<void>(fail);
    for (ResourceMetaData* metaData : sResourceMetaDataList) {
        if (metaData->mId == id) {
            return metaData;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x1AF200.
Symbol ResourceMetaData::GetIdFromExt(Symbol ext) {
    const auto it = sExtToId.find(ext);
    return it != sExtToId.end() ? it->second : Symbol();
}

// Reconstructed from eboot.elf at 0x1AF280.
Symbol ResourceMetaData::GetIdFromPath(const ResourcePath& path) {
    return GetIdFromExt(Symbol(FileGetExt(path.Str(), false)));
}

// Reconstructed from eboot.elf at 0x1AF350.
bool ResourceMetaData::IsA(const ResourcePath& path, Symbol id) {
    const ResourceMetaData* metaData = GetMetaData(GetIdFromPath(path), false);
    if (metaData == nullptr) {
        return false;
    }
    if (metaData->mId == id) {
        return true;
    }
    for (metaData = metaData->mParent; metaData != nullptr; metaData = metaData->mParent) {
        if (metaData->mId == id) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x1AF490. Every extension is also listed
// in the base class's metadata.
void ResourceMetaData::Init(Symbol id, Symbol parent, bool unknown) {
    static_cast<void>(unknown);
    mId = id;
    mParent = parent == Symbol() ? nullptr : GetMetaData(parent, false);
    if (!mTypeFlags[4]) {
        static long sExtensionHeap = MetaDataHeap();
        MemPushHeap(sExtensionHeap);
        for (const Symbol& ext : mExtensions) {
            sExtToId[ext] = id;
            Resource::sMetaData.mExtensions.push_back(ext);
        }
        MemPopHeap();
    }
    static long sListHeap = MetaDataHeap();
    MemPushHeap(sListHeap);
    sResourceMetaDataList.push_back(this);
    MemPopHeap();
    mInitialized = true;
}
