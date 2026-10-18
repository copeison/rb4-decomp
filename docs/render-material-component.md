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
| `DynamicComBase` (inferred) | `0x18EA760` (47 slots) | `Component` plus six pure slots; code at `0x1D0B30`-`0x1D17BF`, in an object the map does not name |
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

## Runtime data

`RndMaterialRuntimeData` (`render/RndMaterialRuntimeData.o`,
`0x4F71E0`-`0x4F998F`, 264 bytes) holds the following:

- the graph resource and its root `RndShaderGraph`;
- the cached root flags;
- the material's render state;
- one usage-hint mask per quality level;
- the "Material" constant buffer;
- the exposed textures.

Its functions work as follows:

- `_CreateCBuffer` (`0x4F7470`) lays the buffer out in temporary memory
  with `RndShaderGraph::FillCBufferConfig` (`0x4FF110`). It creates the
  buffer only when the layout has constants.
- `IsValid` (`0x4F7720`) measures the layout again with a
  validation-only `RndShaderCBufferConfig`
  (`MarkForMaterialValidation`, `0x63A1A0`). The data is still valid
  when the size and the exposed-texture count match.
- `GetSceneTexUsage`, `GetSceneTexCaptureUsage`, `GetSceneDepthUsage` and
  `UsesLinearDepth` (`0x4F7800`-`0x4F7860`) read the graph's `+232`-`+244`.
- `SyncCBufferExposedProps` (`0x4F78F0`) walks the graph's
  `mExposedPropLayout` (`+312`). It copies bools as words, copies the
  float vectors, and transposes the 3x3, 3x4 and 4x4 matrices into
  register rows. It skips the integer types.
- `InitUsageHints` (`0x4F8D90`) picks one usage group per quality level.
  The choice depends on whether the graph is emissive or lit, its alpha
  cut and forward lighting (`RndShaderGraph::IsFwdLit`, per quality), its
  scene-texture level and linear depth, the bucket, the blend mode, the
  forced-opaque flag and the depth prepass. Scene-mask materials add
  bit 3.
- `_ClearExposedTextures` (`0x4F8770`) empties the texture vectors.

`RndMaterialRuntimeDataResource` (`render/RndMaterialRuntimeDataResource.o`,
`0x4F9990`-`0x4F9D50`, vtable `0x19113B0`, `sMetaData` at `0x1A8C0A0`) has
its own source file. Its extension is "materialruntime". It never loads
or saves, and it fails without data. Its destructor passes the data to
`RndMaterialRuntimeData::Delete`. The registration (`0x4AFFA0`) and the
factory (`0x4DC8A0`) are inline in the header.

`_Imprint` (`0x4F4A80`) inlines the copy constructors of `DynamicComBase`,
`DynamicComResource` and the map's `DynamicCom(DynamicCom const&)`: they
copy `prop_storage` through `PropArrayBase::_Copy`, `prop_crc`, the loaded
flag, the file, `com_order_crc` and `reloads_self`, and start the saved
properties and both resource pointers afresh. The material copies its
properties and resets its runtime state; the properties go through
`DynamicComBase::_ImprintProps` (`0x1D1540`).

## Not reconstructed

- `InitPropRegistry` and `_InitMetaData`, because the property metadata is
  not modelled.
- The property change functors (`0x4F4EE0`-`0x4F4F40`) and the prop-handler
  `std::function` stubs (`0x4F52C0`-`0x4F5CA0`).
- `DynamicCom::_Init` (`0x4F4F70`, `0x4F5150`) and the
  `DynamicPropRegistry` builder (`0x4F60C0`, `0x4F6190`).
- In `DynamicComBase`: `_PostLoad`, `_Save`, `_InitializeDynamicProps` and
  the storage slots.
- In `RndMaterialRuntimeData`: `LoadExposedTextures` (`0x4F7C60`) and
  its per-texture loader (`0x4F82D0`). They need the entity property read
  at `0x18B390`, which `src/entity/props/PropUtl.h` does not declare, and
  a 2D-array texture resource type. The texture-resource `GetOrLoad`
  instantiations at `0x4F8FD0`-`0x4F97EE` are also missing.

## Weak evidence

- The names `DynamicComBase`, `DynamicComResource` and
  `DynamicPropRegistry`.
- The names of slots 41-43, 45 and 46.
- `mKeepBlendMode`, `mPlaymodeSink` and `mRecordModeSink`.
- `NeedsUniqueMaterial`.
- `RndShaderGraph`'s `Allows*` and blend-factor queries, and `mShaders`.
- `PrecacheMaterialShaders`. It shares `0x3DEC10` with
  `RndDevice::SetConsoleState`, a forwarder to slot 12.
- The `RndShaderGraph` names `IsFwdLit`, `IsEmissive` and `UsesAlphaCut`
  (`0x4FD7C0`, `0x4FD990`, `0x4FDAB0`) are matched to the map's order of
  root queries. The meanings of the usage-hint bits are not recovered.
