#pragma once

#include <cstddef>

#include "entity/core/EntityResource.h"
#include "utl/text/Symbol.h"

class BinStream;
class EditorCom;
class FusionPatchCom;

// A Fusion sampler patch (audio/FusionPatchResource.o, 0x5B590 to
// 0x5C282): an entity whose root object holds the FusionPatchCom. It loads
// from a ".sxt" file, from the ".fusion" text format or from its cache, and
// registers itself with FusionGeneratorManager under its file name. The
// vtable is at 0x18E2660; the object is 0xD0 bytes.
//
// The binary also overrides Resource's slots 8 and 9: slot 8 (0x5C280)
// returns true, and slot 9 (0x5B890) appends each keyzone's sample, after
// the sample's own slot-9 dependencies, to an
// eastl::vector<ResourcePtr<Resource>>. Resource.h still declares those
// slots as the map's PrintCsvStatsHeader and PrintCsvStats, so the
// overrides wait for its correction; until then the emitted vtable keeps
// Resource's 0x5CF90 and 0x5CFA0 in slots 8 and 9.
class FusionPatchResource : public EntityResource {
public:
    // The class id, created on first use and inlined into its users. The
    // local static is at 0x19C5EC0.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("FusionPatchResource");
        }
        return id;
    }

    // Registers the class as an entity resource. Inlined into
    // SoundManager::Init, at 0x1960 in SoundManager.o.
    static void Init() {
        _Init(sMetaData);
        const Symbol id = Id();
        Resource::sFactory[id] = &_Create;
        sMetaData.Init(id, Symbol("EntityResource"), true);
    }
    // The factory. Emitted in SoundManager.o at 0x95F0.
    static Resource* _Create() {
        return new FusionPatchResource;
    }

    FusionPatchResource();  // 0x5B590

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x5C180
    Symbol GetId() const override;                   // slot 1: 0x5C190
    bool IsA(Symbol type) const override;            // slot 2: 0x5C230
    // Slot 3 at 0x5B9D0. A ".sxt" or ".fusion" file is read directly into a
    // new entity; any other file loads through the cache.
    void LoadFile() override;
    // Slot 5 at 0x5BBB0. A ".fusion" patch is written in the text format,
    // except while components run regression tests.
    void Save(BinStream& stream, bool cached) override;
    // Slot 6 at 0x5C260.
    bool Fail() const override;
    // Slots 10-11: 0x5B5E0, 0x5B610. The patch leaves FusionGeneratorManager.
    ~FusionPatchResource() override;
    // Slot 12 at 0x5B770: true without a patch, or when a keyzone's sample
    // changed on disk or must itself be reloaded.
    bool NeedsReload() override;
    // Slot 13 at 0x5C060: names the root "fusion_patch" and gives it the
    // patch component; the root cannot change layer, be deleted or have its
    // properties changed in the editor.
    Entity* CreateEntity() override;
    // Slot 14 at 0x5BD20. A ".fusion" cache holds the text format, which is
    // read into the patch component.
    bool _LoadEntity(BinStream& stream, bool cached) override;

    // The patch component of the entity's root. The entity must exist. At
    // 0x5B830.
    FusionPatchCom* GetPatch() const;
    // The path's extension. No caller in this build. At 0x5BBA0.
    const char* _GetExt() const;
    // Loads the entity's resources. No caller in this build. At 0x5BF80.
    bool PrepareEmpty();
    // The root's editor component, or null. No caller in this build. At
    // 0x5BF90.
    EditorCom* GetEditorCom() const;
    // Whether the root's editor component records a change. It reads the
    // editor component's change state (+0x1C) through 0x114D50, which is
    // not modelled. No caller in this build. At 0x5BFF0. Not reconstructed.
    bool IsModified() const;

    // Fills the type metadata. At 0x5B650.
    static void _Init(ResourceMetaData& metaData);

    static ResourceMetaData sMetaData;  // 0x19C87D0

    // Field names are not in the reference map.
    // Whether the last load produced a patch; Fail returns its inverse.
    bool mLoaded;
    // The file name the patch is registered under, which play requests
    // name.
    Symbol mName;
};

static_assert(offsetof(FusionPatchResource, mLoaded) == 0xC1);
static_assert(offsetof(FusionPatchResource, mName) == 0xC8);
static_assert(sizeof(FusionPatchResource) == 0xD0);
