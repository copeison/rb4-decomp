#pragma once

#include <cstddef>

#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class ResourcePath;

// Type information shared by every resource of one class
// (entity/ResourceMetaData.o, 0x1AF110-0x1AF940). Each resource class fills
// its metadata and registers it with Init, which also maps the class's file
// extensions to its id. Field names are not in the reference map.
class ResourceMetaData {
public:
    ResourceMetaData();  // 0x1AF130

    // Sets the id and the parent class's metadata, registers the
    // extensions unless mTypeFlags[4] is set, and adds the metadata to the
    // list. The flag is not read.
    void Init(Symbol id, Symbol parent, bool unknown);  // 0x1AF490

    static eastl::vector<ResourceMetaData*>& GetAllResourceMetaData();  // 0x1AF1B0
    // The registered metadata with the id, or null. The release build does
    // not read `fail`.
    static ResourceMetaData* GetMetaData(Symbol id, bool fail);  // 0x1AF1C0
    // The id registered for the extension, or the empty symbol.
    static Symbol GetIdFromExt(Symbol ext);  // 0x1AF200
    // The id registered for the path's extension, or the empty symbol.
    static Symbol GetIdFromPath(const ResourcePath& path);  // 0x1AF280
    // Whether the class registered for the path's extension is the class
    // with the id or derives from it.
    static bool IsA(const ResourcePath& path, Symbol id);  // 0x1AF350

    // The file extensions of the type, such as "mp3" and "wav" for
    // FmodAudioStreamResource; Init registers each in the extension map.
    eastl::vector<Symbol> mExtensions;
    // The type's category, such as "FMod Banks"; empty by default.
    Symbol mCategory;
    // Four flags the constructor sets (+40 to +43) and three it clears (+44
    // to +46). Resource::Init clears mTypeFlags[0]. Init skips the extension
    // registration when mTypeFlags[4] is set; Resource::LoadFile builds no
    // cache for a type with mTypeFlags[5]; GetCachedResourcePath adds the
    // platform suffix to the cached name unless mTypeFlags[6] is set. The
    // readers of the first four are not identified.
    bool mTypeFlags[7];
    // Zero by default; both FMOD types set it to one. Its reader is not
    // identified, so the name is weakly supported.
    int mTypeOption;
    // Returns the cached files' platform suffix; the constructor stores
    // 0x1AF1A0, which returns PlatformSymbol(7).
    Symbol (*mPlatformSymbolFunc)();
    Symbol mId;
    // Set when Init finishes.
    bool mInitialized;
    ResourceMetaData* mParent;

    // Every initialized metadata, in registration order.
    static eastl::vector<ResourceMetaData*> sResourceMetaDataList;  // 0x19E4658
    // Each registered extension's resource id.
    static eastl::map<Symbol, Symbol> sExtToId;  // 0x19E4678
};

static_assert(offsetof(ResourceMetaData, mCategory) == 32);
static_assert(offsetof(ResourceMetaData, mTypeFlags) == 40);
static_assert(offsetof(ResourceMetaData, mTypeOption) == 48);
static_assert(offsetof(ResourceMetaData, mPlatformSymbolFunc) == 56);
static_assert(offsetof(ResourceMetaData, mId) == 64);
static_assert(offsetof(ResourceMetaData, mInitialized) == 72);
static_assert(offsetof(ResourceMetaData, mParent) == 80);
static_assert(sizeof(ResourceMetaData) == 88);
