#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "render/drawing/RndDrawInstanceCom.h"
#include "render/meshes/RndMeshResource.h"
#include "utl/text/Symbol.h"

class RndMesh;
class RndSkeletonPoseCom;

// The component that draws a triangle mesh (render/RndMeshCom.o, 0x5C3020
// to 0x5CE6AF). Its class id is "Mesh". The mesh is a file resource, an
// inline resource in the entity, or a mesh the component or someone else
// owns; with the unique usage the component draws its own copy. It hands
// one draw instance per scene level to the scene drawer, follows a skinned
// mesh's root bone with the draw node's sphere, and queues the mesh's GPU
// refresh with the drawer. The vtable at 0x19288B0 has 46 slots. The object
// is 256 bytes. Field names are not in the reference map; the properties are
// named after the registry (0x5C3660).
class RndMeshCom : public RndDrawInstanceCom {
public:
    // "mesh_resource_type". The map names the type; the enumerator names
    // follow the registry's labels.
    enum ResourceType : int {
        // "Mesh is a file-based resource".
        kResourceFile = 0,
        // "Mesh is an inline resource".
        kResourceInline = 1,
        // "Mesh is not a resource (I manage its lifetime)".
        kResourceUnique = 2,
        // "Mesh is not a resource (someone external manages its lifetime)".
        kResourceUniqueExternal = 3,
    };

    // "mesh_resource_usage". Name not in the reference map; the enumerator
    // names follow the registry's labels.
    enum ResourceUsage : int {
        // "Shares the same mesh geometry with other instances".
        kUsageShare = 0,
        // "Makes a full copy of the mesh geometry".
        kUsageUnique = 1,
    };

    RndMeshCom();  // 0x5C3020
    // Copies the mesh properties for an imprint; the resource, the mesh and
    // the other run-time state start afresh. Inlined into _Imprint
    // (0x5CA6A0). Not in the reference map.
    RndMeshCom(const RndMeshCom& other)
        : RndDrawInstanceCom(other),
          mMeshResourcePath(other.mMeshResourcePath),
          mResourceType(other.mResourceType),
          mResourceUsage(other.mResourceUsage),
          mDriven(other.mDriven),
          mStaticBoundingSphere(other.mStaticBoundingSphere),
          mMeshResource(),
          mMesh(nullptr),
          mSkeleton(nullptr),
          mSkinned(false),
          mPendingMeshSync(0),
          mPendingUsage(-1) {}
    // Slots 0-1: 0x5C30B0, 0x5C3120. An owned mesh is deleted.
    ~RndMeshCom() override;

    Symbol GetId() const override;             // slot 4: 0x5CA620
    Symbol GetClassName() const override;      // slot 5: 0x5CA630
    int CurrentRev() const override;           // slot 7: 0x5CA640
    bool IsA(Symbol type) const override;      // slot 8: 0x5CA660
    Component* AsComponent() override;         // slot 9: 0x5CA690
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x5CA6A0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x5CA870
    ComMetaData& _GetMetaData() override;       // slot 23: 0x5CA880
    // Slot 29 at 0x5C9BB0: loads a file or inline mesh and finds the root
    // object's skeleton. The map's _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;
    // Slot 33 at 0x5C9E00.
    void _Poll() override;
    // Slot 38 at 0x5CA150: applies a changed usage, reloads a changed mesh
    // resource and finds the skeleton again before the poll.
    void _EditPoll() override;
    // Slot 42 at 0x5CA260: the renderer's lit default material.
    RndMaterialCom* _GetDefaultMaterial() const override;
    // Slots 43-44 at 0x5CA280 and 0x5CA2D0. The map's signatures start with
    // an ObjPtr const&.
    void _InitDrawInstancesImpl(VectorAdapter<RndDrawInstance>& instances) override;
    void _SyncDrawInstancesImpl(VectorAdapter<RndDrawInstance>& instances) override;

    // Changes the resource type, releasing an owned mesh. An inline mesh
    // gets the path "mesh_inline_<layer>_<serial>.<ext>" and is reloaded
    // when the entity already holds it. The map's signature starts with an
    // ObjPtr const&.
    void SetResourceType(ResourceType type);  // 0x5C31B0
    // Makes the mesh the entity's inline mesh resource for this object.
    // The map's signature starts with an ObjPtr const&.
    void SetInlineMesh(RndMesh* mesh);  // 0x5C34C0
    // Draws a mesh that is not a resource; the callers pass
    // kResourceUnique. The map's signature is SetUniqueMesh(ObjPtr const&,
    // RndMesh*).
    void SetUniqueMesh(RndMesh* mesh, ResourceType type);  // 0x5C3630
    // Replaces the drawn mesh, deleting an owned one, and takes its sphere
    // and whether it is skinned. The map's signature starts with an ObjPtr
    // const&.
    void _SetMesh(RndMesh* mesh);  // 0x5C33A0
    // Sets the draw node's sphere to the mesh's, moved by the root bone's
    // render transform. The map's signature starts with an ObjPtr const&.
    void _PollSkinnedSphere();  // 0x5CA020

    // The class factory. Emitted with the renderer's component
    // registration at 0x405690.
    static Component* _Create();
    // Registers the properties, the mesh utilities and the class
    // description (100 KB). Not reconstructed: the property metadata's
    // attributes and actions are written through helpers that are not
    // modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x5C3660

    static Symbol sId;          // 0x1AA5DE0, "Mesh"
    // The class symbol GameObject::CreateComponent takes; also "Mesh". Name
    // not in the reference map.
    static Symbol sClassName;   // 0x1AA5DE8
    static PropRegistry sPropRegistry;  // 0x1AA5DF0
    static ComMetaData sMetaData;       // 0x1AA5E90
    // The sub-object class of a mesh inside another resource, "mesh".
    static Symbol kSubObjectResource;  // 0x1AA6038

    // "mesh_resource_path".
    ResourcePath mMeshResourcePath;
    // "mesh_resource_type".
    ResourceType mResourceType;
    // "mesh_resource_usage".
    ResourceUsage mResourceUsage;
    // "driven" and "static_bounding_sphere": the latter keeps a skinned
    // mesh's sphere from following the root bone.
    bool mDriven;
    bool mStaticBoundingSphere;
    // The file or inline mesh resource.
    ResourcePtr<RndMeshResource> mMeshResource;
    // The drawn mesh.
    RndMesh* mMesh;
    // The root object's skeleton, found when the resources load and in the
    // edit poll.
    RndSkeletonPoseCom* mSkeleton;
    // Whether the mesh's vertex type has bone weights and indices.
    bool mSkinned;
    // Sync flags ORed into the mesh's RndMesh::mPendingSync by the next
    // poll, which queues the mesh with the scene drawer; bit 0 also takes
    // its sphere. No writer was found in this object, so the name is weak.
    unsigned int mPendingMeshSync;
    // A "mesh_resource_usage" set in the editor (0x5CA9E0), applied by the
    // edit poll; -1 when none.
    int mPendingUsage;
};

static_assert(offsetof(RndMeshCom, mMeshResourcePath) == 192);
static_assert(offsetof(RndMeshCom, mResourceType) == 200);
static_assert(offsetof(RndMeshCom, mResourceUsage) == 204);
static_assert(offsetof(RndMeshCom, mDriven) == 208);
static_assert(offsetof(RndMeshCom, mStaticBoundingSphere) == 209);
static_assert(offsetof(RndMeshCom, mMeshResource) == 216);
static_assert(offsetof(RndMeshCom, mMesh) == 224);
static_assert(offsetof(RndMeshCom, mSkeleton) == 232);
static_assert(offsetof(RndMeshCom, mSkinned) == 240);
static_assert(offsetof(RndMeshCom, mPendingMeshSync) == 244);
static_assert(offsetof(RndMeshCom, mPendingUsage) == 248);
static_assert(sizeof(RndMeshCom) == 256);
