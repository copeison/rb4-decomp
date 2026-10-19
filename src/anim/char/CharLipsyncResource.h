#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

class CharLipsync;

// A character's lip-sync data, extension "lipsync"
// (anim/CharLipsyncResource.o, 0x2F6700-0x2F716F; newer than the reference
// map, the class id gives the name): the CharLipsync built from the
// "<name>_lipsync.xml" file beside the resource's path. A cached file
// holds the data at revision 0; outside the archive it is used only while
// it is newer than the XML file. The vtable at 0x18F6EC0 has 13 slots. The
// object is 56 bytes.
class CharLipsyncResource : public Resource {
public:
    CharLipsyncResource();  // 0x2F6700
    // Slots 10-11: 0x2F6770, 0x2F67C0.
    ~CharLipsyncResource() override;

    ResourceMetaData* GetMetaData() const override;    // slot 0: 0x2F7040
    Symbol GetId() const override;                     // slot 1: 0x2F7050
    bool IsA(Symbol type) const override;              // slot 2: 0x2F70F0
    // Slot 4: the cached data, or the XML file's when the cache is stale or
    // the source is read.
    bool Load(BinStream& stream, bool cached) override;  // 0x2F6930
    // Slot 5: the revision, then the data in a cached file.
    void Save(BinStream& stream, bool cached) override;  // 0x2F6FD0
    // Slot 6: without data.
    bool Fail() const override;  // 0x2F7120

    // The XML file the data comes from: "<folder>/<base>_lipsync.xml" for
    // the resource's path. Inlined into Load. Name not in the reference
    // map.
    ResourcePath GetXmlPath() const;  // 0x2F6DD0

    // The class id, created on first use and inlined into its users. The
    // local static is at 0x19F7CC8.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("CharLipsyncResource");
        }
        return id;
    }

    // Registers the class as a resource. The binary emits it in
    // anim/AnimInit.o at 0x2AE2D0, which Anim::Init calls.
    static void Init();
    // The factory. Emitted in anim/AnimInit.o at 0x2B5ED0.
    static Resource* _Create();
    // Names the extension ("lipsync") and the category ("Animations").
    static void _Init(ResourceMetaData& metaData);  // 0x2F6810

    // Constructed by the static initializer at 0x2F7130, which first sets a
    // shared header's null object id to -1 (0x19F95B8).
    static ResourceMetaData sMetaData;  // 0x19F95C0

    // Field name not in the reference map.
    CharLipsync* mLipsync;
};

static_assert(offsetof(CharLipsyncResource, mLipsync) == 48);
static_assert(sizeof(CharLipsyncResource) == 56);
