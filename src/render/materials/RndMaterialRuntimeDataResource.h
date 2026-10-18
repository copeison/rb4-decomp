#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

class RndMaterialRuntimeData;

// The resource that holds a material's runtime data in its entity, at the
// path "mat_rt:<shader graph>:<layer>_<serial>" with the extension
// "materialruntime" (render/RndMaterialRuntimeDataResource.o, 0x4F9990 to
// 0x4F9D50; vtable 0x19113B0, 13 slots; 56 bytes). It is never loaded or
// saved: the material makes it around data it already built.
class RndMaterialRuntimeDataResource : public Resource {
public:
    // The class symbol, kept in a guarded static (0x1A8AD98) that is built
    // on first use. Inlined into its users.
    static Symbol Id() {
        static Symbol sId;
        if (sId == Symbol()) {
            sId = Symbol("RndMaterialRuntimeDataResource");
        }
        return sId;
    }

    // Registers the class: its metadata and its factory. Emitted with the
    // renderer's type registration at 0x4AFFA0.
    static void Init() {
        _Init(sMetaData);
        const Symbol id = Id();
        Resource::sFactory[id] = &_Create;
        sMetaData.Init(id, Symbol("Resource"), true);
    }
    // The factory. Emitted at 0x4DC8A0.
    static Resource* _Create() {
        return new RndMaterialRuntimeDataResource;
    }
    // Names the type's extension ("materialruntime").
    static void _Init(ResourceMetaData& metaData);  // 0x4F9AE0

    // Leaves the data unset.
    RndMaterialRuntimeDataResource();  // 0x4F9990
    explicit RndMaterialRuntimeDataResource(
        RndMaterialRuntimeData* data);  // 0x4F9A00

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x4F9C20
    Symbol GetId() const override;                   // slot 1: 0x4F9C30
    bool IsA(Symbol type) const override;            // slot 2: 0x4F9CD0
    // Slots 4-5: nothing is read or written.
    bool Load(BinStream& stream, bool cached) override;  // slot 4: 0x4F9BF0
    void Save(BinStream& stream, bool cached) override;  // slot 5: 0x4F9C00
    bool Fail() const override;                          // slot 6: 0x4F9C10
    // Hands the data to RndMaterialRuntimeData::Delete.
    // Slots 10-11: 0x4F9A70, 0x4F9AA0.
    ~RndMaterialRuntimeDataResource() override;

    static ResourceMetaData sMetaData;  // 0x1A8C0A0

    // The held data; null when the resource failed. Name not in the
    // reference map.
    RndMaterialRuntimeData* mData;
};

static_assert(offsetof(RndMaterialRuntimeDataResource, mData) == 48);
static_assert(sizeof(RndMaterialRuntimeDataResource) == 56);
