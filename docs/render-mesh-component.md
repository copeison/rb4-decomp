# Draw instance and mesh components

The components that hand drawables to the scene drawer derive from
`RndDrawInstanceCom`. `RndMeshCom` draws a triangle mesh. Their sources are
in `src/render/drawing/RndDrawInstanceCom.{h,cpp}` and
`src/render/meshes/RndMeshCom.{h,cpp}`.

| Object | Range | Vtable | Size |
| --- | --- | ---: | ---: |
| `render/RndDrawInstanceCom.o` | `0x6C1500`-`0x6C3D3F` | `0x1937BB0` (46 slots) | 192 |
| `render/RndMeshCom.o` | `0x5C3020`-`0x5CE6AF` | `0x19288B0` (46 slots) | 256 |

| Static | RndDrawInstanceCom | RndMeshCom |
| --- | ---: | ---: |
| `sId` / `sClassName` | `0x1AB0370` / `0x1AB0378` (`"DrawInstance"`) | `0x1AA5DE0` / `0x1AA5DE8` (`"Mesh"`) |
| `sPropRegistry` | `0x1AB0380` | `0x1AA5DF0` |
| `sMetaData` | `0x1AB0420` | `0x1AA5E90` |
| `_Init` | `0x6C17F0` | `0x5C3660` |

`RndMeshCom::kSubObjectResource` (`"mesh"`) is at `0x1AA6038`. The mesh
factory `_Create` is emitted with the renderer's registration at
`0x405690`.

## RndDrawInstanceCom

The properties are `billboarding`, `sort_by`, `sorting_hint`, `lods` (a
bit per scene level, all three by default), `extra_data` and
`extra_data_1`. `RuntimeData` (`+72`) holds:

- the registration flags;
- a sort key the instances take while `0x19B03C1` is set;
- one `VectorAdapter<RndDrawInstance>` per scene level (three with
  `RndConfig::mUseLod`, else one);
- the cached transform, draw node, material, material reference and
  default material;
- the link in the drawer's list (`+176`).

Slots 41-45 are the class's own:

- `_GetNumDrawInstancesImpl`: the `lods` bit.
- `_GetDefaultMaterial` (inferred): the unlit default, and the lit one for
  meshes.
- `_InitDrawInstancesImpl` and `_SyncDrawInstancesImpl`: empty here.
- `_DrawsGeometry` (inferred): false only for `RndDecalCom`.

The component registers with `RndSceneDrawer::FindForEntity`'s drawer when
it enters and any level has instances. It leaves the drawer when it exits,
unless the whole entity goes. It refreshes its instances every poll once a
subclass passed them to `InitDrawInstances`. `_SyncInstanceStateFlags`
(`0x6C3500`) resolves the material: the object's own, the referenced
object's (`RndMaterialReferenceCom`), or the default. It stores the
material's usage hints for the quality level, with the draw node's flag
`0x200` as `0x40000`. Deactivation hides the instances. Destroying the
component alone collapses the draw node's sphere onto the object's
position. The component follows `DrawNode`, `Material` and
`MaterialReference` in the component order.

## RndMeshCom

The properties are `mesh_resource_path`, `mesh_resource_type` (file,
inline, unique, unique external), `mesh_resource_usage` (share, unique
copy), `driven` and `static_bounding_sphere`. The registry also carries
the virtual and utility properties: the `make_*` builders, `combine`,
`bake_xfm` and `export_to_fbx`. The runtime members are:

- the mesh resource (`+216`) and the drawn mesh (`+224`);
- the root object's `RndSkeletonPoseCom` (`+232`);
- whether the mesh is skinned (`+240`);
- pending mesh sync flags (`+244`);
- a pending usage (`+248`).

- `_OnResourcesLoaded` (`0x5C9BB0`): an entity-inline mesh resource wins
  over `GetOrLoad`, and switches the type to inline. The unique usage
  draws `RndMeshUtl::Copy` of the mesh.
- `_SetMesh` (`0x5C33A0`) takes the mesh's sphere into the draw node. It
  decides skinning from the vertex layout's weight and bone attributes.
- `SetResourceType`, `SetInlineMesh` and `SetUniqueMesh` change where the
  mesh comes from. Inline meshes live in the entity as
  `"mesh_inline_<layer>_<serial>.<ext>"`.
- `_Poll` (`0x5C9E00`):
  1. A skinned mesh moves the draw node's sphere with the root bone
     (`_PollSkinnedSphere`, `0x5CA020`).
  2. It polls the instances.
  3. It ORs the pending flags into the mesh and queues the mesh with the
     drawer's `RndDynamicGpuDataMgr` (`RndDynamicGpuDataMgr::Enqueue`).
- `_InitDrawInstancesImpl` (`0x5CA280`) sets the mesh and the skeleton's
  bone buffer.
- `_SyncDrawInstancesImpl` (`0x5CA2D0`) fills each instance:
  - the world transform (transposed 3x4);
  - the draw node's inverse world rotation as the normal transform;
  - the draw node's sphere, clip planes, environment and show/hide flags;
  - the component's sort, billboard and extra data;
  - the material's runtime data (stamped with the frame count), cull mode
    and stencil flags.

`_Imprint` of both classes (`0x6C3790`, `0x5CA6A0`) is reconstructed. The
inlined copy constructors keep the draw-instance properties and the mesh's
path, resource type and usage, `driven` and `static_bounding_sphere`; the
draw-instance `RuntimeData`, the mesh resource, the mesh, the skeleton and
the pending sync and usage start afresh.

## Registration

`RndMeshCom::Init` (`0x3F7420`, placement factory `0x4056C0`) is
`InitComponentClass<RndMeshCom>`. `_Init` (`0x5C3660`) registers the mesh
properties, `mesh_resource_properties` (the resource's build options and
usage flags through `RndMeshResource`'s setters), read-only statistics
(`vertex_type`, `num_faces`, `num_vertices`, `vertices_size`, `root_bone`,
`root_bone_index`, `extents` through `RndMeshUtl::ComputeAABB`), and the
`utilities`: `bake_xfm`, `combine`, `make_triangle`, `make_quad`,
`make_triangle_fan`, `make_cube`, `make_box`, `make_sphere`,
`make_cylinder`, `make_capsule`, `make_cone`, `make_torus`,
`make_icosahedron`, `make_spline_fan` and `export_to_fbx` (an empty
action). Each utility edits a file-scope parameter block
(`0x1AA6040`-`0x1AA6408`, `gBakeXfmMode` at `0x19BDCC8`) through the
accessors at `0x5CA9E0`-`0x5CB7BF` and builds an inline mesh; it is
read-only while `driven`. Revision 1 turns saved mesh paths into
sub-object paths. The new builder parameter types (`CreateTriangleParams`,
constructor `0x5CA890`, `CreateConeParams`, `CreateTorusParams`,
`CreateIcosahedronParams`, `CreateSplineFanParams`) and the builders are
declared in `RndMeshUtl.h`; `Box`, `RndVertex::GetTypeName`/
`IsTypeCompressed`/`CanCompressType`, `RndSkeletonPoseCom::GetSkeleton`,
`CharSkeleton` (bone names only) and `RndSplineCom` (class id only) are
declared for it.

## Weak evidence

- `_DrawsGeometry`, `SetInstanceClipPlane` and `_ReRegisterWithSceneDrawer`.
- The sort-key global `gDrawInstanceSortKeys`.
- `mPendingMeshSync`.
- `RndMesh::mGeometryFrame` read as the root bone index.
- `RndSkeletonPoseCom::mRenderXfms`.
- The utility accessors' and parameter globals' names; `CreateSplineFan`
  and its parameters (not in the map); `Combine`'s last flag.
- `StructMetadata::mKind`, which `extents` and the vector structs set to 1.

`render/RndDynamicGpuDataMgr.o` is reconstructed in
`src/render/meshes/RndDynamicGpuDataMgr.cpp`: the constructor (`0x6C9D40`),
the destructor (`0x6C9DE0`, which detaches the objects still queued) and
`ProcessQueue` (`0x6C9EB0`). Under a "Sync Dynamic GPU Data" stat block,
`ProcessQueue` calls each queued object's `_SyncDynamicGpuDataImpl`,
detaches the object and empties the queue. The manager consists of a
`CritSec` and an `eastl::vector<RndDynamicGpuData*>` (48 bytes).

## RndMeshResource

`RndMeshResource` (`render/RndMeshResource.o`, `0x5D1A00`-`0x5D4800`,
vtable `0x1929C08`, 64 bytes, `sMetaData` at `0x1AA6910`) is in
`src/render/meshes/RndMeshResource.{h,cpp}`. The reconstructed parts are:

- both constructors, `0x5D1A00` and `0x5D1A80` (the second wraps an
  inline mesh);
- the destructor, which deletes the mesh;
- `_Init` (`0x5D1B80`), with extension "mesh" and category "Meshes";
- `Fail` and `SupportsCompanionFile` (`!mInlined`);
- `GetMetaData`, `GetId` and `IsA`;
- the getters `IsKeepMeshData`, `GetVertexUsageFlags` and
  `GetFaceUsageFlags`.

The registration (`0x3EB400`) and the factory (`0x403F50`) are inline in
the header. The build options (`+56`-`+59`) are "compress_vertices" (on by
default), "smooth_tangents", "calc_uv_scale" and "calc_curvature". The
companion-file reader (`0x5D4180`) reads them, along with "keep_mesh_data",
"vertex_usage_flags" and "face_usage_flags".

Not reconstructed:

- `Load` (`0x5D1CC0`), with its "HXMESH" reader (`0x5D1D50`), and `Save`
  (`0x5D2430`). They need the vertex layouts' stream readers and writers
  (`0x443600`-`0x444899`).
- The mesh rebuild (`0x5D27F0`).
- `AreVerticesCompressed` (`0x5D2770`).
- The option setters (`0x5D2790`-`0x5D4150`).
- The companion-file reader (`0x5D4180`).
