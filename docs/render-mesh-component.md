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

## Not reconstructed

- `_Init` of both classes (the mesh's is 100 KB of properties and utility
  actions).
- The mesh's property-handler stubs (`0x5CA9E0`-`0x5CE170`).
- The utility parameter blocks the mesh's static initializer builds
  (`0x1AA6048`-`0x1AA6407`).

## Weak evidence

- `_DrawsGeometry`, `SetInstanceClipPlane` and `_ReRegisterWithSceneDrawer`.
- The sort-key global `gDrawInstanceSortKeys`.
- `mPendingMeshSync`.
- `RndMesh::mGeometryFrame` read as the root bone index.
- `RndSkeletonPoseCom::mRenderXfms`.

`render/RndDynamicGpuDataMgr.o` is reconstructed in
`src/render/meshes/RndDynamicGpuDataMgr.cpp`: the constructor (`0x6C9D40`),
the destructor (`0x6C9DE0`, which detaches the objects still queued) and
`ProcessQueue` (`0x6C9EB0`). Under a "Sync Dynamic GPU Data" stat block,
`ProcessQueue` calls each queued object's `_SyncDynamicGpuDataImpl`,
detaches the object and empties the queue. The manager consists of a
`CritSec` and an `eastl::vector<RndDynamicGpuData*>` (48 bytes).
