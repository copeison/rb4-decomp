# Scene drawer

The scene drawer draws a scene entity: it keeps the draw instances of the
scene's components, culls them for each camera into per-bucket lists,
sorts them and flushes them through the passes. Its sources are in
`src/render/drawing`.

| Object | Range | Vtable | Size |
| --- | --- | ---: | ---: |
| `render/RndSceneDrawer.o` | `0x419170`-`0x4349FF` | `0x19007A8` (10 slots) | 13008 |
| `render/RndSceneDrawParams.o` | `0x434A00`-`0x434B7F` | | 208 |
| `render/RndBasicCuller.o` | `0x459C20`-`0x45EB90` | `0x1903740` (10 slots) | 344 |
| `render/RndScenePartialFramerateData.o` | `0x6D18A0`-`0x6D190F` | | 80 |

In this build `RndSceneDrawParams.o` follows `RndSceneDrawer.o`; the
older map has it before. Every object ends with the same three-int static
initializer (`{-1, 8, 4}`; `0x4349E0` here, `0x434B60` for the
parameters), which comes from a shared header and is not modelled.

## Types

- `RndSceneDrawParams` (208 bytes, constructor `0x434A00`) is what a
  caller asks for: up to two buffer collections (a stereo pair when the
  target modes are the left and right eye), the context, the clear color,
  the camera and its entity, the projection rectangle, the show/hide
  flags, the mode switches (depth only, draw to textures, wireframe only,
  skip), the shading mode and the shadow, atmosphere and post-processing
  switches, and whether the scene is output to the back buffer.
- `RndSceneDrawTarget` (48 bytes) is what a draw produced for one
  collection: its index, the ping-pong pair of light-accumulation indices
  the post-processing swaps, the result texture of a partial-framerate
  copy, the depth target index of the transparent passes and the
  volumetric scattering's resolution. `Draw` returns one per collection
  and takes the previous ones back.
- `RndSceneInternalContext` (28384 bytes) is the working state of one
  draw: the finalized parameters, a `CameraData` (9368 bytes) per
  collection with its camera context, its cull analysis and the drawer
  bucket sets it culls into, the shared cull camera, the clear color and
  the scene's singleton components (scene, light manager, sky, fog or
  volumetric scattering, the two post-processing chains, antialiasing).
- `RndDrawInstance` (232 bytes) is one drawable: drawable, instance
  buffer, material runtime data, render state, show/hide and state flags,
  bounds, sort keys, `RndInstanceData`, two clip planes and the sort
  distance. Each of the low 22 state-flag bits puts it in a draw bucket.
- `PodVector<T>` is the memcpy-grown vector the drawer and the culler use
  ("PodVector buffer"); `Free()` stands for its destructor because
  `FixedVector` needs trivially destructible elements.
- `RndCuller` is the pure interface (typeinfo only) the drawer culls
  through, with `RndShowHideContext` and `RndCullerParams` (no culling,
  state-flag mask, extra clip planes).

## RndSceneDrawer

The drawer is a `PollDepBase` job (slots 3-5: `ThreadPoll`, `PostPoll`,
`GetPollName` "Scene Drawer %d"). Draw instance components register with
the drawer their scene has (`FindForEntity`, `0x419260`: the scene
component's or the `RndEntity` component's drawer on the root object, up
through the instancing entities). Registrations made while the scene
enters are queued on a list under the drawer's lock with their instance
counts per level; `EnterCoda` and the post-poll hand them to the culler
at once (`_ProcessNewlyRegistered`) and reserve the buckets for the
culler's largest bucket sizes. `EnterPrelude` and `ExitPrelude` drop the
queue and clear the culler. `ResetRegistrations` frees every bucket.

The members are the culler, the scene constant buffer, the dynamic GPU
data queue, the occlusion query manager, three bucket sets of 22
`PodVector<RndDrawInstance>` and their sortable `RndDrawInstance*` lists
(the first and second camera, and the shadow casters), the drawer's own
camera context (`SetSceneCamera`), the partial-framerate cull results,
the job graph, the threaded draw's context and twelve fences for the
async-compute passes.

`Draw` (`0x41AA10`) finalizes the parameters
(`_ExtractSingletonsAndFinalizeParams`, `0x41B060`), uploads the shader
graph globals and draws each collection at the full framerate
(`_DrawFullFramerate`: the depth-only, texture and wireframe modes, or
`_DrawInterval0`, `_DrawInterval1`, the output conversion and the buffer
inspection) or at a partial framerate (`_DrawPartialFramerate`: the first
frame of an interval draws interval 0 and stores the cull results and the
lit scene, the next draws interval 1 on a copy of it). A stereo pair with
the stereo optimizations is drawn by `_DrawStereo`, which culls the lights
and draws the shadow maps once. `StartDrawJobs` and `FinishDrawJobs` run
the same passes as a job graph.

Interval 0 culls (`_Cull`), analyses the buckets (`_AnalyzeCullResults`:
which need the light culling, the scene texture levels, the linear depth),
submits the occlusion queries and draws the "draw post-cull" passes (scene
texture capture, depth clear, scene mask, Z and normals prepasses, the
deferred lit, emissive and unlit buckets 4-6, linear depth), culls the
lights and draws the shadow maps, then the "draw post-light-cull" passes
(sky texture, deferred buckets 7-9, decals 10-12, ambient occlusion and the
volumetric scattering on the compute pipe). Interval 1 accumulates the
deferred lighting, applies the fog or the volumetric scattering, and
draws the forward buckets 13-14, the occlusion queries, the transparent
buckets 15-20 with the mask buffer and the two post-processing chains
between them, the antialiasing, the shading-mode display and the overlays
(21).

Each pass sorts its bucket (`StateCompare`, `DepthOnlyStateCompare` or
`DistanceCompareBackToFront`) and flushes it through `_FlushBucket<N>`,
which batches up to 256 consecutive instances with the same state
(one with the `disable_batching` variable set). The nine instantiations
(`0x4268D0`-`0x42F3F0`) differ only in their `_FlushBatch<N>`, which sets
the stencil, the blend state or the forward environment and atmosphere
shading of its passes; the map's values of `N` are not recovered, so the
instantiations are numbered in the binary's order (`FlushType`).

## RndBasicCuller

`RndBasicCuller` is the only `RndCuller`. It owns the draw instances of
every registered `RndDrawInstanceCom`, with one `LodInstances` pool (112
bytes) per scene level: an `eastl::vector<RndDrawInstance>`, a free list
of deregistered instances sorted by address, and overflow blocks for
registrations that do not fit, since growing the pool would move
instances the components point at. Registration (`0x45A490`) takes a run
of freed instances (`_AllocFromFreeList`, `0x45AA60`), the pool's spare
capacity, or a new block; deregistration (`0x45ABF0`) shrinks the pool,
hides the instances on the free list, or deletes the block.
`PrepareToGrowBy` reserves 1.5 times the pending counts; `Poll` is empty.

`Cull` (`0x45AE90`) stays on the calling thread when culling is off or the
thread already polls for a manager (`_CullSingleThreaded`, `0x45B6C0`,
visiting each enabled level with `_VisitLodInstances<0/1>`); otherwise
`_CullMultiThreaded` (`0x45B080`) runs two passes of 48 static
`RndBasicCullerJob`s (`ThreadedJob`, vtable `0x19037A0`, 12 slots) in the
"Culler" poll group: a cull pass over ranges of at least 100 instances
and a copy pass into the drawer's buckets with the sort distances.
`GetMaxBucketSizes` counts level 0's pool per bucket bit and `TrimPools`
releases the jobs' lists, keeping nine tenths of their peak as the next
reservation.

## RndScenePartialFramerateData

The data a partial-framerate scene's first frame of an interval leaves for
the next: the frame, interval and drawer it was drawn by, the drawer's
deregistration and the light manager's removal counts, the parameters the
finalization keeps across the interval, the cull analysis and the light
manager's update flag. The drawer's `mStoredCullResults`
(`RndSceneCullResults`) keeps the buckets and the culled lights.

## RndEntityCom

`RndEntityCom` (`render/RndEntityCom.o`, `0x6CA2D0`-`0x6CAE7B`, vtable
`0x1938278`, 41 slots, 72 bytes; class id "RndEntity") derives from
`RndDrawableEntityCom`. Its `RuntimeData` holds the entity's poll group
and poll groups, the scene drawer and whether the component owns it.
`ObtainSceneDrawer` borrows the drawer of the nearest entity up the tree
or creates an owned one; the destructor deletes an owned drawer.
`_Init` and `_Imprint` stay declared.

## Not reconstructed

- The job graph (`RndSceneDrawJobs`, `0x430610`-`0x4349DF`): only the
  constructor, destructor, `Start` and `Finish` the drawer calls are
  declared.
- `_InitLightAccum` (`0x426160`), `_GenerateLinearDepth` (`0x4234C0`),
  `_DrawMaskBuffer` (`0x424E10`), `_CaptureSceneTex` (`0x420FF0`),
  `_DrawSceneMask` (`0x421AE0`), `_DrawToTextures` (`0x41E420`) and
  `_DrawWireframe` (`0x41ECC0`) are declared only.
- Compiler-generated code in the object: the implicit
  `RndSceneInternalContext` and `RndCameraContext` copies (`0x41D710`,
  `0x41DA90`, `0x4275F0`, `0x427AF0`), the light-list destructor
  (`0x4274E0`) and the EASTL sort instantiations (`0x428A90`,
  `0x42B310`, `0x42FF90` and their helpers).

## Weak evidence

- `RndSceneDrawJobs`, `StartDrawJobs`, `FinishDrawJobs`, `ExitPrelude`,
  `ResetRegistrations`, `_ReserveDrawInstances`, `_DrawStereo`,
  `_DrawPostCull`, `_DrawPostLightCull`, `_DrawDepthOnly`,
  `_DrawToTextures`, `_SetDepthOnlyTargets`, `_BuildBatchContext` and the
  `FlushType` names.
- `RndSceneDrawParams::mDrawScene`, `mTargetMode`, `mDrawSceneMask`;
  `RndSceneDrawTarget::mDepthTargetIndex` and `mResolved`;
  `RndDrawInstance::mUsesSceneTex` and `mUsesSceneDepth`.
