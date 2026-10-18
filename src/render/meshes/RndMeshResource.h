#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

class RndMesh;

// A mesh file resource (render/RndMeshResource.o, 0x5D1A00 to 0x5D4800;
// vtable 0x1929C08, 13 slots; 64 bytes). It holds the mesh read from a
// "HXMESH" file and the build options its companion file sets.
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

    // Registers the class: its metadata and its factory. Emitted with the
    // renderer's type registration at 0x3EB400.
    static void Init() {
        _Init(sMetaData);
        const Symbol id = Id();
        Resource::sFactory[id] = &_Create;
        sMetaData.Init(id, Symbol("Resource"), true);
    }
    // The factory. Emitted at 0x403F50.
    static Resource* _Create() {
        return new RndMeshResource;
    }
    // Names the type's extension ("mesh") and category ("Meshes").
    static void _Init(ResourceMetaData& metaData);  // 0x5D1B80

    RndMeshResource();  // 0x5D1A00
    // Wraps a mesh made in place, for RndMeshCom::SetInlineMesh (0x5C34C0).
    // The build options are left unset.
    explicit RndMeshResource(RndMesh* mesh);  // 0x5D1A80

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x5D46A0
    Symbol GetId() const override;                   // slot 1: 0x5D46B0
    bool IsA(Symbol type) const override;            // slot 2: 0x5D4750
    // Slots 4-5. Load reads a "HXMESH" file (its cached form when `cached`
    // is set, the inline text form for an inline resource) through the
    // reader at 0x5D1D50 and syncs the mesh; Save writes one. Not
    // reconstructed: the vertex layouts' stream readers and writers
    // (0x443600-0x444899) are not modelled.
    bool Load(BinStream& stream, bool cached) override;  // slot 4: 0x5D1CC0
    void Save(BinStream& stream, bool cached) override;  // slot 5: 0x5D2430
    bool Fail() const override;                          // slot 6: 0x5D1CA0
    // Only a resource with a file has a companion file.
    bool SupportsCompanionFile() const override;  // slot 7: 0x5D1CB0
    ~RndMeshResource() override;  // slots 10-11: 0x5D1AF0, 0x5D1B30

    // The mesh's flags. The setters at 0x5D2790-0x5D4150 (SetCompressVertices,
    // SetSmoothTangents, SetCalcUVScale, SetCalcCurvature, SetKeepMeshData,
    // SetVertexUsageFlags and SetFaceUsageFlags) rebuild the mesh through
    // 0x5D27F0 and mark the resource changed on disk; they are not
    // reconstructed.
    bool IsKeepMeshData() const;              // 0x5D40B0
    unsigned int GetVertexUsageFlags() const;  // 0x5D4100
    unsigned int GetFaceUsageFlags() const;    // 0x5D4140

    // The class's metadata; its last extension names the inline mesh files
    // RndMeshCom::SetResourceType makes (0x5C3236).
    static ResourceMetaData sMetaData;  // 0x1AA6910

    // Field names are not in the reference map.
    // The loaded mesh.
    RndMesh* mMesh;
    // The build options the companion file's "compress_vertices",
    // "smooth_tangents", "calc_uv_scale" and "calc_curvature" set (read at
    // 0x5D4180), and which the mesh file records. Only the vertex
    // compression is on by default.
    bool mCompressVertices;
    bool mSmoothTangents;
    bool mCalcUVScale;
    bool mCalcCurvature;
};

static_assert(offsetof(RndMeshResource, mMesh) == 48);
static_assert(offsetof(RndMeshResource, mCompressVertices) == 56);
static_assert(offsetof(RndMeshResource, mCalcCurvature) == 59);
static_assert(sizeof(RndMeshResource) == 64);
