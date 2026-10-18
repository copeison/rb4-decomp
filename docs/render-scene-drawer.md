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

## The other passes

- `_InitLightAccum` (`0x426160`, "Init Light Accum") clears the light
  accumulation of a cleared draw, to the scene's clear color when it
  has one, and unless that color is black fills the pixels of the
  deferred lit geometry (stencil test 4) with a quad when bucket 4 has
  instances.
- `_GenerateLinearDepth` (`0x4234C0`, "Analyze Depth") linearizes the
  depth: with the tiled lighting on compute slot 0 between fences 5
  and 6, followed by the light tiles' depth ranges
  (`RndCShaderLinearizeDepth`, `RndCShaderCalcDepthRange`); otherwise
  with a quad (`RndShaderLinearizeDepth`).
- `_CaptureSceneTex` (`0x420FF0`, "Capture SceneTex") copies the source
  light buffer into the destination one, which becomes the target's
  result, and for blurred scene reads downsamples and blurs it into
  `mBlurredLightAccum` once or twice ("Blur SceneTex").
- `_DrawSceneMask` (`0x421AE0`, "Process Scene Mask") inlines the map's
  `_RefineSceneMask` ("Refine Scene Mask") and writes stencil bit 0x20
  from the refined tiled mask ("Stencil Scene Mask"), through the tiled
  mask mesh while `scene_mask_stencil_mesh` is set.
- `_DrawMaskBuffer` (`0x424E10`, "Mask Buffer") draws bucket 18 in solid
  white into the scene mask when the scene's mask buffer is enabled,
  optionally followed by the signed-distance classification and
  distance passes, and sets `RndSceneDrawTarget::mResolved`.
- `_DrawToTextures` (`0x41E420`) draws the impostor maps into the four
  target textures; `_DrawWireframe` (`0x41ECC0`) draws the wireframe
  overlay into the back buffer.
- `_DrawDebugCallbacks` (`0x41FC20`) and `_DrawStereoPostCull`
  (`0x420730`) are empty; only the job graph calls them.

## RndSceneDrawJobs

The multithreaded draw's job graph (1168 bytes, `0x430610`-`0x4343DF`
and the jobs' members up to `0x4349DF`), built once by the drawer and
hooked into the caller's graph by `Start` (`0x4317E0`); `Finish`
(`0x431970`) returns each collection's final target. The cull analysis
job works the target out for the collection's jobs with
`_PropagateDrawTarget` (`0x431AB0`), mirroring the changes of the
non-threaded passes. Every job is a `PollDepBase` with ten slots; the
bucket, dynamic GPU data and shadow map jobs derive from
`RndDeferredContextDrawJob` (`render/context/RndDeferredContextJobs.h`)
and draw into a deferred context that an `RndExecDeferredContextJob`
executes after them.

| Job | Report name | Vtable | Size | Constructor | Slot 3 |
| --- | --- | ---: | ---: | ---: | ---: |
| `RndSceneDrawStartJob` | draw start | `0x1900878` | 152 | `0x432440` | `0x432470` |
| `RndSceneDrawInitContextJob` | draw init immed context | `0x19008D8` | 160 | `0x432480` | `0x4324C0` |
| `RndSceneDrawEndJob` | (default) | `0x1900938` | 160 | `0x432560` | `0x4325B0` |
| `RndSceneDrawCullJob` | draw cull | `0x1900998` | 168 | `0x4325D0` | `0x432630` |
| `RndSceneDrawCullAnalysisJob` | draw cull analysis | `0x19009F8` | 216 | `0x432740` | `0x4327E0` |
| `RndSceneDrawPostCullJob` | draw post-cull | `0x1900A58` | 200 | `0x432870` | `0x4328F0` |
| `RndSceneDrawBucketJob` | draw bucket | `0x1900AB8` | 216 | `0x432B90` | `0x432C20` |
| `RndSceneDrawLightCullingJob` | draw light culling | `0x1900B18` | 168 | `0x432EE0` | `0x432F40` |
| `RndSceneDrawTiledLightCullingJob` | draw tiled light culling | `0x1900B78` | 152 | `0x4330A0` | `0x4330E0` |
| `RndSceneDrawPostLightCullJob` | draw post-light-cull | `0x1900BD8` | 208 | `0x433360` | `0x433470` |
| `RndSceneDrawDeferredLightJob` | draw deferred light | `0x1900C38` | 208 | `0x4336B0` | `0x4337C0` |
| `RndSceneDrawSubmitOccQueriesJob` | draw submit occ queries | `0x1900C98` | 160 | `0x4339C0` | `0x433A10` |
| `RndSceneDrawCaptureSceneTexJob` | draw capture scenetex | `0x1900CF8` | 208 | `0x433AB0` | `0x433B40` |
| `RndSceneDrawPostProcJob` | draw postproc | `0x1900D58` | 208 | `0x433BE0` | `0x433C70` |
| `RndSceneDrawSyncJob` | draw sync dynamic gpu data | `0x1900DB8` | 192 | `0x433D10` | `0x433D80` |
| `RndSceneDrawShadowMapJob` | draw shadowmap (exec N) | `0x1900E18` | 1280 | `0x433E50` | `0x434000` |
| `RndSceneDrawEndCollectionJob` | draw end | `0x1900E78` | 208 | `0x4341B0` | `0x4342C0` |

The graph: the start job, then each collection's cull and cull
analysis and one dynamic GPU data job per deferred context; the
immediate context's init after those. The light culling waits for
every analysis and hands shadow light *i* to shadow map job *i*
(`RndConfig::mMaxShadowContribBuffers` jobs). Each collection's bucket
jobs (none for buckets 2, 3 and 21) execute in bucket order after its
post-cull job, with the immediate-context jobs between them: the tiled
light culling (linear depth and tiled lights) after bucket 6, the
post-light-cull job before bucket 9, the deferred lighting between 12
and 13, the occlusion queries between 14 and 15, the level 2 scene
texture capture between 15 and 17 and the two post-processing chains
around bucket 19; the end collection job and the graph's end job
close it. The jobs that draw on deferred contexts are handed to the
caller (`mJobs`).

## Not reconstructed

- Compiler-generated code in the object: the implicit
  `RndSceneInternalContext` and `RndCameraContext` copies (`0x41D710`,
  `0x41DA90`, `0x4275F0`, `0x427AF0`), the light-list destructor
  (`0x4274E0`), the EASTL sort instantiations (`0x428A90`,
  `0x42B310`, `0x42FF90` and their helpers) and the job graph's EASTL
  vector growth and range insert (`0x431F00`, `0x432000`, `0x432100`,
  `0x432210`).
- `RndDynamicGpuDataMgr::SplitQueue` (`0x6C9FE0`, in
  `render/RndDynamicGpuDataMgr.o`) and `RndContext::SetWireframeColor`
  (`0x6BD6F0`) are declared only.

## Weak evidence

- `RndSceneDrawJobs`, `StartDrawJobs`, `FinishDrawJobs`, `ExitPrelude`,
  `ResetRegistrations`, `_ReserveDrawInstances`, `_DrawStereo`,
  `_DrawPostCull`, `_DrawPostLightCull`, `_DrawDepthOnly`,
  `_DrawToTextures`, `_SetDepthOnlyTargets`, `_BuildBatchContext` and the
  `FlushType` names.
- Every job class name, `_PropagateDrawTarget`, `_DrawStereoPostCull`
  and `_DrawDebugCallbacks` (the map's empty-in-release candidate), and
  `RndDynamicGpuDataMgr::SplitQueue`.
- `RndSceneDrawParams::mDrawScene`, `mTargetMode` (the job graph passes
  it where `Draw` passes the context's scene draw index), `mDrawSceneMask`;
  `RndSceneDrawTarget::mDepthTargetIndex` and `mResolved`;
  `RndDrawInstance::mUsesSceneTex` and `mUsesSceneDepth`.
