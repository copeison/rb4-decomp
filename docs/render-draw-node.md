# Render draw node

`RndDrawNodeCom` (`render/RndDrawNodeCom.o`, `0x6C3D40`-`0x6C944F`) is the
component that places an object in the scene's draw hierarchy. Its class id
is `"DrawNode"`. The source is in `src/render/scene/RndDrawNodeCom.{h,cpp}`.

| Item | Address |
| --- | ---: |
| Vtable (41 slots) | `0x1937DD0` |
| `sId`, `sClassName` | `0x1AB05D8`, `0x1AB05E0` |
| `sPropRegistry`, `sMetaData` | `0x1AB05F0`, `0x1AB0690` |
| Static initializer | `0x6C9380` |
| `_Init` | `0x6C4060` |
| `Init` (in `RndInit.o`), `_Create` | `0x3EE650`, `0x404430` |

The object is 296 bytes. The static initializer also sets the shared
header's `-1, 8, 4` triple at `0x1AB05C8`; the `-1` is the null object id.

## Properties

| Offset | Property | Member |
| ---: | --- | --- |
| `0x18` | `local_sphere` | `mLocalSphere` |
| `0x28` | `sphere_radius_mult` | `mSphereRadiusMult` |
| `0x2C` | `show_hide_flags` (`showing` is bit 0 inverted) | `mShowHideFlags` |
| `0x30` | `inherit_from` (1 Transform Parent, 2 Entity Root) | `mInheritFrom` |
| `0x34` | `environment` | `mEnvironment` |
| `0x38` | `nested_light_environments` | `mNestedLightEnvironments` |
| `0x60` | `clip_plane_0`, `clip_plane_1` | `mClipPlanes` |

`RuntimeData` (`+0x68`, constructor `0x6C3E60`, 192 bytes) holds the
runtime properties `world_sphere`, `world_show_hide_flags`,
`environ_index`, `world_clip_plane_0/1` and `editor_show_hide_flags`, the
dirty flags, the inverse world transform (`+0xB4`) with its winding flag,
and the node pointers `_CacheNodePointers` fills: the TransCom, the
instance and instance-pool components, the entity root's draw node, the
parent and environment-parent draw nodes, the instanced entity's root draw
node and the parent object's instance pool.

## Behaviour

- Slot 29 (`_OnResourcesLoaded`, `0x6C63B0`) takes the scene's default
  environment for revision 0 data and caches the node pointers.
  `_FindParent` (`0x6C68C0`) walks the transform parents to the nearest draw
  node for `inherit_from` 1; the entity root inherits from the instancing
  object's draw node.
- `_Enter`, `_PollDrawNode` (`0x6C73A0`, run by slot 33 and `_EditPoll`) and
  `_EditEnter` share an inlined body: the entity root rebuilds its local
  sphere around its entity's (or the parent pool's) draw nodes when one
  moved; the world sphere follows the local one and the TransCom's world
  transform; the show/hide flags take the parent's bits `0xBBF` and the
  environment parent's `0x440`; each clip plane passes through its object,
  facing its z axis, or is the parent's; `environ_index` comes from the
  `RndLightEnvironCom` in a scene, or from the environment parent.
- `_Enter` marks the root to keep its sphere when an entity above polls
  only on screen (`EntityHeaderCom` polling mode 2); the edit mode always does.
- `_Exit` and `_EditExit` dirty the root's sphere for instance-level exits.
- `_EditPoll` re-registers nested lights (`_SyncNestedLightEnvirons`,
  `0x6C82F0`) after `nested_light_environments` changed.
- `_ContainsNestedLights` and `_GetNumNestedLights` recurse through the
  instanced entities for `RndLightCom` objects.

`_Imprint` (`0x6C8A70`) copies the properties and starts a fresh
`RuntimeData`. `_Init`, its property callbacks (`0x6C8CA0`-`0x6C8DBA`) and
its functor classes (vtables `0x1937F18`-`0x1938008`) stay with the
unreconstructed registry builder. `0x6C7370` is an unreferenced copy of
`_Exit`'s body.

The math helpers `Sphere::GrowToContain` (`0x117D500`) and
`MultiplyMinScale` (`0x117D7B0`, name not in the map) are declared in
`math/geometry/Sphere.h`.

## RndDrawNodeEditCom

`render/RndDrawNodeEditCom.o` (`0x6C9450`-`0x6C9B9A`, static initializer
`0x6C9AD0`) is reconstructed in `src/render/scene/RndDrawNodeEditCom.{h,cpp}`.
The class id is `"DrawNodeEditorData"` (sId `0x1AB0848`, sClassName
`0x1AB0850`, sPropRegistry `0x1AB0860`, sMetaData `0x1AB0900`); the vtable
at `0x1938068` has 59 slots and matches slot for slot. Its capability bits
are `0x42`; it shows only for a selected object while the selection
options' "show_bounding_spheres" is set, and draws the draw node's world
sphere as a yellow mesh at 0.3 alpha (`RndDrawUtl::DrawSphereMesh`,
`0x3E7D90`, declared). `_Init` makes it a TransEntityResource class that
requires and accompanies `RndDrawNodeCom`; `_Create` is `0x406050`.

### RndEditorDrawCom

The base of the editor companions (`render/RndEditorDrawCom.o`,
`0x461330`-`0x4616BE`, vtable `0x1903D98`, 48 bytes) is in
`src/render/editor/RndEditorDrawCom.{h,cpp}`. Its `RuntimeData`
(`0x461370`) holds capability bits and the link in the root's
`RndEditorDrawSetCom` list. `GetWorldSphere` (`0x461400`) combines the
class's own sphere (bit `0x20`, slot 45) with the draw node's (bit `0x40`).
`_Init` (`0x4614C0`) makes the class a debug class of drawable entities
that requires `RndEditorDrawSetCom`. Slots 41-58 are declared in vtable
order with the defaults the binary shares (`0x44C780`, `0x43D6D0`,
`0x3BC540`-`0x3BC620`). The map names slots 41, 42 and 45-58 except 51
and 52; the names of slots 43, 44, 51 and 52 rest only on their default
bodies (weak evidence), and `RndSubEditableId::sNone` (`0x1A86EB0`) is
declared but not defined.
