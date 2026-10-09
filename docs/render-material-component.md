# Material component

`RndMaterialCom` (`render/RndMaterialCom.o`, `0x4F1BD0`-`0x4F662F`) is the
material of an object. It has a shader graph, the graph's exposed
properties as dynamic properties, and the render state the scene drawer
reads. Its sources are `src/render/materials/RndMaterialCom.{h,cpp}`, with
the dynamic-component bases in `src/entity/core/DynamicCom.{h,cpp}` and the
GPU data in `src/render/materials/RndMaterialRuntimeData.{h,cpp}`.

## Hierarchy

| Class | Vtable | Notes |
| --- | --- | --- |
| `DynamicComBase` (inferred) | `0x18EA760` (47 slots) | `Component` plus six pure slots; code at `0x1D0B30`-`0x1D17BF` (`entity/InstanceCom.o`) |
| `DynamicComResource<T>` (inferred) | `0x1910E68` (1 slot) | polymorphic base at `+160`: `_ShouldLoadUniqueResource`, the resource and an inline resource |
| `DynamicCom<RndShaderGraphResource>` | `0x1910CC8` (47 slots) | the map's template; `mFile` (`+184`), `com_order_crc`, `reloads_self` |
| `RndMaterialCom` | `0x1910AF8` (48 slots) | secondary vtables `0x1910C88` (`+160`) and `0x1910CA0` (`RndDynamicGpuData`, `+200`) |

`DynamicComBase` keeps the dynamic properties in `prop_storage` (`+24`, a
`PropArray<unsigned char>`), their layout hash `prop_crc` (`+64`), and a
`MemStream` (`+72`, `utl/MemStream.o`, vtable `0x18EF308`). The stream
holds the properties while the resource reloads. Slots 41-46 are the
template's: the layout hash, whether a resource is loaded, sharing the
resource with an imprint, the class's base registry (`_GetBasePropRegistry`,
in the map), the resource's registry, and building that registry. The
resource builds the registry per component class in its
`DynamicPropRegistry<T>` (`+208` in `RndShaderGraphResource`, `Get` at
`0x4F60C0`). `RegisterBasePropRegistryInit` (`0x1D0B30`) records each class's
base-registry builder. Slot 47 of `RndMaterialCom` is
`_SyncDynamicGpuDataImpl` (thunk `0x4F46D0`).

## Statics

| Static | Address |
| --- | ---: |
| `sId`, `sClassName` (`"Material"`) | `0x1A8B920`, `0x1A8B928` |
| `sPropRegistry` | `0x1A8B930` |
| `sMetaData` | `0x1A8B9D0` |

The static initializer is at `0x4F6560`. `_Init` (`0x4F1BD0`) runs
`InitPropRegistry` (`0x4F1C10`), the description (`0x4F3010`) and
`RegisterBasePropRegistryInit`. The interface is `"MaterialProvider"`
(`GetMaterialProviderId`, `0x4F3240`).

## Layout

The properties (`+216`-`+253`) are `sharing_type`, `bucket`, `blend_mode`,
`blend_factor`, `cull_mode`, `receive_atmosphere`, `receive_decals`,
`depth_prepass`, `force_opaque`, `scene_mask` and `unique`, plus
`is_puppet` (`+259`). The runtime members are:

- the sharing and render-state dirty flags (`+256`, `+257`);
- the first-build flag (`+258`);
- the `mat_rt:` resource (`+264`);
- the `RndMaterialRuntimeData*` (`+272`);
- the GPU-refresh flag (`+280`);
- two `MsgSource::EventSinkElem` (`+288`, `+328`).

The object is 368 bytes.

## Behaviour

- Sharing: `SetSharingType` and `_SyncSharing` make the material unique
  for `kUnique`, or for `kAutomatic` when `NeedsUniqueMaterial`
  (`0x50F2D0`) says so. `_SetUnique` (`0x4F3900`) releases and rebuilds the
  runtime data.
- `_InitRuntimeData` (`0x4F3E50`): a shared material looks up
  `"mat_rt:<graph>:<layer>_<serial>"` among the entity's inline resources
  (`EntityResource::TryGetInline`). New data is inlined into the object's
  layer as an `RndMaterialRuntimeDataResource`, replacing a stale entry. A
  reused data set skips the texture load only on the first build.
- `_OnResourcesLoaded` (slot 29, `0x4F3AB0`) loads the graph
  (`DynamicCom::_LoadResource`, `0x4F5CB0`). It then replaces a blend mode
  the graph forbids and takes the graph's blend factor. It resets the
  bucket, cull mode and depth prepass when the graph's root node does not
  allow them. Finally it passes each quality level's usage hints to device
  slot 12 (`RndDevice::PrecacheMaterialShaders`, inferred).
- The poll (`0x4F43A0`) copies a dirty render state into the runtime data
  (`_SyncRenderState`, `0x4F4730`, which rebuilds the usage hints). It then
  queues the material's `RndDynamicGpuData` base with the scene drawer.
  `_SyncDynamicGpuDataImpl` then uploads the blend parameters, the exposed
  properties and the constant buffer.
- `Handle` answers `leaving_playmode` and `leaving_record_mode` by marking a
  shared material for a GPU refresh.
- `_PostCreate` copies the shader graph and blend mode of the object's draw
  instance's default material.

## Not reconstructed

- `InitPropRegistry` and `_InitMetaData`, because the property metadata is
  not modelled.
- `_Imprint` (`0x4F4A80`): its copy inlines the map's
  `DynamicCom(DynamicCom const&)`, which `src/entity` does not model. It
  copies `prop_storage` through `PropArrayBase::_Copy`, `prop_crc`, the
  loaded flag, the file, `com_order_crc` and `reloads_self`, starts the
  saved properties and both resource pointers afresh, then copies the
  material's properties and resets its runtime state (`mFirstRuntimeData`
  set, the subscriptions unlinked). The properties go through
  `DynamicComBase::_ImprintProps` (`0x1D1540`).
- The property change functors (`0x4F4EE0`-`0x4F4F40`) and the prop-handler
  `std::function` stubs (`0x4F52C0`-`0x4F5CA0`).
- `DynamicCom::_Init` (`0x4F4F70`, `0x4F5150`) and the
  `DynamicPropRegistry` builder (`0x4F60C0`, `0x4F6190`).
- In `DynamicComBase`: `_PostLoad`, `_Save`, `_InitializeDynamicProps` and
  the storage slots.
- In `RndMaterialRuntimeData`: everything but the constructor, `New`,
  `Delete`, the destructor, `_CacheGraphFlags` and the two constant-buffer
  syncs.

## Weak evidence

- The names `DynamicComBase`, `DynamicComResource` and
  `DynamicPropRegistry`.
- The names of slots 41-43, 45 and 46.
- `mKeepBlendMode`, `mPlaymodeSink` and `mRecordModeSink`.
- `NeedsUniqueMaterial`.
- `RndShaderGraph`'s `Allows*` and blend-factor queries, and `mShaders`.
- `PrecacheMaterialShaders`. It shares `0x3DEC10` with
  `RndDevice::SetConsoleState`, a forwarder to slot 12.
