#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

class RndMesh;

// A mesh file resource (render/RndMeshResource.o). Its methods are not
// reconstructed; only the class symbol and the mesh RndParticleCom reads are
// declared.
class RndMeshResource : public Resource {
public:
    // The class symbol, "RndMeshResource", kept in a guarded static
    // (0x1A71950) that is built on first use. Inlined into its users, for
    // example Resource::GetOrLoad<RndMeshResource> at 0x4640B0.
    static Symbol Id() {
        static Symbol sId;
        if (sId == Symbol()) {
            sId = Symbol("RndMeshResource");
        }
        return sId;
    }

    // Wraps a mesh made in place, for RndMeshCom::SetInlineMesh (0x5C34C0).
    // The vtable is at 0x1929C08. Not reconstructed.
    explicit RndMeshResource(RndMesh* mesh);  // 0x5D1A80

    ResourceMetaData* GetMetaData() const override;      // slot 0: 0x5D46A0
    Symbol GetId() const override;                       // slot 1: 0x5D46B0
    bool IsA(Symbol type) const override;                // slot 2: 0x5D4750
    bool Load(BinStream& stream, bool cached) override;  // slot 4: 0x5D1CC0
    void Save(BinStream& stream, bool cached) override;  // slot 5: 0x5D2430
    bool Fail() const override;                          // slot 6: 0x5D1CA0
    bool SupportsCompanionFile() const override;         // slot 7: 0x5D1CB0
    ~RndMeshResource() override;                         // slots 10-11: 0x5D1AF0, 0x5D1B30

    // The class's metadata; its last extension names the inline mesh files
    // RndMeshCom::SetResourceType makes (0x5C3236).
    static ResourceMetaData sMetaData;  // 0x1AA6910

    // The loaded mesh. Name not in the reference map.
    RndMesh* mMesh;
};

static_assert(offsetof(RndMeshResource, mMesh) == 48);
