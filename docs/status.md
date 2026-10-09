# Decompilation status

## Milestones

- [x] Recover the ELF payload from `eboot.bin`.
- [x] Identify the executable format and compiler SDK baseline.
- [x] Prepare an IDA-compatible analysis copy without modifying program data.
- [x] Create the persistent IDA database and export the initial function map.
- [x] Recover and name the startup path.
- [x] Resolve Orbis imports against the PS4 SDK 5.008 stub libraries.
- [x] Classify major subsystems and source translation units.
- [x] Begin function-by-function C++ reconstruction.
- [x] Reconstruct the top-level game initialization sequence.
- [x] Reconstruct the main per-frame update and render schedule.
- [x] Recover the complete UI layout ID and primary asset-path map.
- [x] Resolve the private pad imports and reconstruct the special-controller
  calibration sample path.
- [x] Recover all FMOD 1.10.04 import names and apply them to IDA.
- [x] Reconstruct FMOD module setup and the primary audio initialization path.
- [x] Reconstruct PS4 FMOD module loading and thread-affinity assignment.
- [x] Reconstruct the default render-lighting scene and light-set switch.
- [x] Reconstruct the six default render materials and shader-graph bindings.
- [x] Reconstruct the default render-camera object and component attachment.
- [x] Reconstruct the 49 default fallback textures and family ordering.
- [x] Recover the exact default float colors, three-dimensional checker fill,
  common format conversion, typed descriptors, and direct texture factories.
- [x] Reconstruct top-level default render-resource initialization ordering.
- [x] Reconstruct fallback directional-light creation and activation.
- [x] Recover default texture lookup and shadow-offset accessors.
- [x] Replace default compute-buffer wrappers with their exact typed common
  descriptors and dynamic release path.
- [x] Reconstruct default render-resource polling and teardown.
- [x] Recover default render-resource construction, reverse vector teardown,
  retained scene cleanup, and its `100.0` light scale.
- [x] Reconstruct the render-system poll, begin, end, and skipped-frame control flow.
- [x] Reconstruct pre-frame screenshot capture and its resolution table.
- [x] Recover renderer draw-mode and buffer debug-view tables and setters.
- [x] Recover and name the 24-command renderer debug console registry.
- [x] Reconstruct the settings, HDR, screenshot, and screenshot-resolution
  debug command handlers directly.
- [x] Reconstruct shader-reload traversal, compiled-object release, and dirty
  publication through the typed resource manager.
- [x] Reconstruct renderer settings defaults, config keys, and capability gates.
- [x] Recover the exact 232-byte renderer-settings layout, allocation/release,
  and signed limit fields.
- [x] Replace settings platform-query adapters with direct slot-seven feature
  and advertised-resolution reads.
- [x] Reconstruct generic command-line switch-value lookup and remove the
  settings-specific resolution adapter.
- [x] Identify renderer tile dimensions and light-capacity settings.
- [x] Reconstruct shared tiled-light compute-buffer initialization.
- [x] Reconstruct shared tiled-light compute-buffer teardown.
- [x] Reconstruct per-target tiled-light and stereo compute-buffer allocation.
- [x] Inline the tiled-light interpolation descriptor and verified
  shadow-scratch reuse fallback.
- [x] Reconstruct per-target tiled-light resource teardown.
- [x] Reconstruct scene-mask render-target allocation and teardown.
- [x] Inline full-resolution, scratch, and tile scene-mask descriptors and
  owner registration.
- [x] Reconstruct tiled scene-mask targets and grid mesh.
- [x] Inline tiled scene-mask texture descriptors, primary-target flags, and
  owner registration.
- [x] Reconstruct per-scene linear and tiled depth targets.
- [x] Inline linear and tiled-depth creation-state and format descriptors.
- [x] Reconstruct the linear-depth compute shader's dispatch, orthographic
  permutation, tile definitions, resource bindings, and dimensions constant.
- [x] Reconstruct the tiled-depth-range compute shader's dispatch, tile-size
  definition, input/output bindings, and tile-count constant.
- [x] Reconstruct the per-scene ambient-occlusion target.
- [x] Inline the ambient-occlusion texture descriptor and primary-block
  registration path.
- [x] Reconstruct per-scene GBuffer targets.
- [x] Inline GBuffer color and normal texture descriptors and owner
  registration.
- [x] Reconstruct the per-scene depth/stencil target and attachment reuse.
- [x] Inline depth/stencil creation-state and 32/40-bit format descriptors.
- [x] Reconstruct the partial-frame light-accumulation target.
- [x] Reconstruct CMAA render targets and reuse behavior.
- [x] Replace the CMAA capability adapter with the exact platform-seven feature
  bit test.
- [x] Inline CMAA color, edge, and compressed-edge texture descriptors and
  owner registration.
- [x] Reconstruct all four CMAA compute-shader dispatches, exact constants,
  sampled inputs, and unordered-output bindings.
- [x] Reconstruct the SSAO compute-shader dispatch, texture/output bindings,
  constant layout, and tile-size shader definition.
- [x] Reconstruct primary and blurred light-accumulation targets.
- [x] Reconstruct the shared light-accumulation target factory.
- [x] Replace the shared light-accumulation creation adapter with direct
  descriptor assembly and recover texture address/filter default tables.
- [x] Reconstruct per-scene render-target resource-block initialization.
- [x] Recover the exact 80-byte partial-frame state initialization and replace
  its allocation and release adapters with typed ownership.
- [x] Reconstruct top-level render-target resource-owner initialization.
- [x] Reconstruct top-level render-target resource-owner teardown.
- [x] Recover the exact 1,552-byte render-target resource-owner and 216-byte
  per-scene block layouts, including inline storage and lifecycle defaults.
- [x] Replace opaque render-target owner offset accessors with typed field
  access throughout the reconstructed target creation and lifecycle paths.
- [x] Replace identified owner and per-scene target-slot adapters with direct
  typed fields and local enum-to-field mappings.
- [x] Correct owner and per-scene render-resource slots from the unrelated
  32-byte target wrapper to their shared render-texture base type.
- [x] Type render-texture attachment indices and counts and use them directly
  for light-accumulation and depth/stencil cursor advancement.
- [x] Reconstruct the render-target owner's concrete 2D and 2D-array texture
  creation virtuals and shared backend initialization dispatch.
- [x] Recover the concrete 1,552-byte render-target state constructor and
  deleting destructor, including its verified resource-owner prefix mapping.
- [x] Reconstruct the base and concrete render-target resource dispatch tables,
  including the virtual 2D source-type check and deleting destructor.
- [x] Inline verified render-target source binding, unidentified-slot teardown,
  release-state reset, and stereo-mode selection.
- [x] Recover inline scene-block resize semantics, registered-resource mode
  propagation, and the typed tiled-light block overlay.
- [x] Reconstruct render-target owner mode and partial-block controls.
- [x] Reconstruct the four-level sky render-target chain.
- [x] Replace the sky target creation adapter with direct descriptor assembly,
  data-format resolution, and common 2D texture creation.
- [x] Reconstruct the paired half-, quarter-, and eighth-size intermediate targets.
- [x] Replace scaled intermediate creation adapters with shared texture
  descriptor and data-format construction.
- [x] Reconstruct shadow-contribution, scratch, stencil, and soften-tile targets.
- [x] Inline shadow-contribution 2D-array, stencil, scratch, and soften-tile
  descriptors and owner registration.
- [x] Reconstruct mono and stereo volumetric-scattering texture chains.
- [x] Inline volumetric 3D descriptors, direct factory creation, reusable
  backend initialization, and accumulated-scattering voxel data.
- [x] Reconstruct the fallback light-probe accumulation target.
- [x] Inline the fallback light-probe accumulation descriptor, correct its
  tiled-lighting feature gate, and register the resulting texture directly.
- [x] Recover the Low, Medium, and High renderer quality-level mapping.
- [x] Recover runtime resolution parsing and screenshot-mode labels.
- [x] Distinguish the configured vsync mode from the runtime enable flag.
- [x] Reconstruct base render-system construction and destruction ordering.
- [x] Recover the common render-system singleton and shared epoch accessor.
- [x] Type deferred frame activation and reconstruct the common context
  activation wrapper.
- [x] Reconstruct render-system runtime initialization and shutdown ordering.
- [x] Reconstruct Orbis render-system allocation and object lifetime.
- [x] Recover the separate Orbis render-system singleton lifecycle.
- [x] Reconstruct Orbis video-output and submit-thread startup.
- [x] Reconstruct Orbis video-output and submit-thread shutdown.
- [x] Recover common back-buffer and render-context owner release during
  renderer shutdown.
- [x] Reconstruct the shared engine thread launch, trampoline, result capture,
  and join runtime used by the Orbis submit worker.
- [x] Recover the complete 136-byte shared engine thread wrapper, including
  worker registration, callback forwarding, six-processor default affinity,
  and the 128-KiB minimum stack policy.
- [x] Route the FMOD asynchronous file reader and recording worker through the
  shared engine thread wrapper and their exact affinity-record fields.
- [x] Reconstruct the shared CPU-mask builder and centralize the engine's
  six-processor default affinity policy.
- [x] Recover default engine-thread object initialization and cancellation,
  including the Orbis submit worker's initial priority and name.
- [x] Reconstruct Orbis submit-state defaults, recursive mutex lifetime, and
  the self-linked retired-allocation list construction and destruction.
- [x] Reconstruct the complete 17-entry Orbis render-system vtable and remove
  its final constructor adapter.
- [x] Replace the deferred GPU-allocation queue adapters with direct typed
  node allocation, linking, retirement, unlinking, and destruction.
- [x] Centralize shared render allocation APIs under `src/core/memory` and
  remove their duplicate subsystem adapter declarations.
- [x] Type the Orbis video handle, event queue, condition variables, recursive
  submission lock, submit token, and worker-running state.
- [x] Replace verified video, kernel event, Gnm event, splash-service, and
  thread-yield adapters with direct PS4 SDK calls.
- [x] Recover the submit-worker startup handshake, one-second event wait and
  filtering, and timeout submission recovery.
- [x] Recover flip-complete buffer identification and pending-presentation
  counter retirement.
- [x] Reconstruct end-of-pipe counter polling, forced-submit timing, runtime
  vsync application, flip submission, and buffer rotation.
- [x] Reconstruct Orbis GPU idle waits and deferred-allocation retirement.
- [x] Reconstruct the Orbis frame-submit handshake.
- [x] Complete the Orbis deferred GPU-allocation queue lifecycle.
- [x] Reconstruct the guarded game-system shutdown callback.
- [x] Reconstruct the matching game-system renderer startup sequence.
- [x] Recover renderer platform and graphics-API identity mappings.
- [x] Reconstruct platform capability and resolution-list initialization.
- [x] Recover the Orbis GPU fence factory and backing allocation.
- [x] Recover the seven-format Orbis mesh factory and format-name map.
- [x] Type and reconstruct the eight-byte Orbis resource factory, its
  registration, destructors, and complete 16-entry creation-method vtable.
- [x] Replace the common constant-buffer, shader, compute-buffer,
  particle-buffer, and occlusion-query factory adapters with typed vtable
  dispatch.
- [x] Recover the common 2D and 2D-array texture factory dispatch used by
  render-target resource creation.
- [x] Recover the common 3D texture factory dispatch used by volumetric
  scattering resources.
- [x] Type the remaining common 1D, cube, 1D-array, and cube-array texture
  factory dispatch slots.
- [x] Reconstruct common 1D, 1D-array, and cube-array descriptor resolution,
  factory creation, and reusable backend initialization.
- [x] Reconstruct common 2D and 2D-array descriptor creation paths for
  non-reusable textures.
- [x] Reconstruct common cube descriptor resolution, face preparation, factory
  creation, and conditional backend initialization.
- [x] Reconstruct common 3D descriptor resolution, factory creation, and
  reusable backend initialization.
- [x] Reconstruct the common 128-byte render-mesh base and lifetime.
- [x] Unify the render-system epoch used by common mesh updates, Orbis mesh
  draws, and resource synchronization.
- [x] Reconstruct the position-only mesh vertex storage and double-buffered
  GPU update path.
- [x] Reconstruct the color mesh vertex layout and double-buffered GPU update
  path.
- [x] Reconstruct the color-texture mesh vertex layout and GPU update path.
- [x] Reconstruct the 80-byte unskinned mesh vertex layout and GPU update path.
- [x] Reconstruct the 100-byte skinned mesh vertex layout and GPU update path.
- [x] Reconstruct the 52-byte compressed unskinned mesh and GPU update path.
- [x] Reconstruct the 64-byte compressed skinned mesh and GPU update path.
- [x] Reconstruct shared mesh and instance Gnm vertex-descriptor generation.
- [x] Recover all eight mesh-format strides and exact ten-slot attribute
  descriptor tables.
- [x] Reconstruct the Orbis default skinned vertex and identity-instance
  buffers and type their render-system storage.
- [x] Reconstruct the common Orbis mesh draw and instancing path.
- [x] Reconstruct Orbis back-buffer allocation and video-output registration.
- [x] Reconstruct the common render-target lifecycle, 32-byte base layout, and
  active-state header fields used for dimensions and debug modes.
- [x] Reconstruct Orbis graphics/compute render-context allocation.
- [x] Reconstruct Orbis graphics/compute frame submission and reset.
- [x] Recover the common frame-owner collection, active owner, and active Orbis
  frame indices used by render-target selection.
- [x] Type the render system's complete 40-byte frame-owner array and its
  reverse shutdown ownership path.
- [x] Centralize the verified 312-byte render-system core prefix shared by
  frame activation, epoch tracking, settings, factory, and owner lifetime.
- [x] Embed the shared core prefix in the Orbis runtime layout and use its
  common context and back-buffer slots directly.
- [x] Reconstruct Orbis render-target, blend, and default pipeline state.
- [x] Recover Orbis depth, stencil, raster, and color-write state setters.
- [x] Recover Orbis sampler construction and shader unbinding.
- [x] Reconstruct shared Orbis texture and sampler stage binding.
- [x] Recover masked Orbis shader-resource clearing.
- [x] Reconstruct accelerated and raster Orbis depth-stencil clears.
- [x] Reconstruct format-specific Orbis transient vertex drawing.
- [x] Type the 168-byte Orbis transient vertex buffer and recover its
  allocation, append, reset, fallback-stream, and identity-instance binding.
- [x] Recover the Orbis GPU fence destruction and deferred release paths.
- [x] Recover Orbis GPU fence sequencing, signaling, and command-buffer waits.
- [x] Replace Orbis GPU fence lifecycle and allocation adapters with
  source-owned dispatch and named storage.
- [x] Recover cross-queue Orbis resource signaling and grouped waits.
- [x] Reconstruct Orbis transition, UAV, and split resource barriers.
- [x] Recover Orbis compute dispatch and graphics/compute debug markers.
- [x] Recover the Orbis GPU-stat timestamp lifecycle and clock conversion.
- [x] Recover the Orbis compute-buffer counter copy command.
- [x] Reconstruct the Orbis constant-buffer allocation, updates, binding, and destruction paths.
- [x] Reconstruct the common render constant-buffer factory and 64-byte base layout.
- [x] Recover the Orbis 1D texture factory and platform constructor.
- [x] Reconstruct Orbis 1D texture storage, uploads, and destruction.
- [x] Reconstruct Orbis 1D texture shader-stage binding.
- [x] Recover the common 1D texture and exact Orbis subclass layouts.
- [x] Recover the Orbis 2D texture factory and platform constructor.
- [x] Reconstruct Orbis 2D texture storage, views, updates, and destruction.
- [x] Reconstruct Orbis 2D texture shader-stage binding.
- [x] Recover the common 2D texture and exact Orbis subclass layouts.
- [x] Recover the Orbis 3D texture factory and platform constructor.
- [x] Reconstruct Orbis 3D texture storage, uploads, and destruction.
- [x] Reconstruct Orbis 3D texture shader-stage binding.
- [x] Recover the common 3D texture and exact Orbis subclass layouts.
- [x] Recover the Orbis cube texture factory and platform constructor.
- [x] Reconstruct Orbis cube texture storage, views, and destruction.
- [x] Reconstruct Orbis cube texture shader-stage binding.
- [x] Recover the common cube texture and exact Orbis subclass layouts.
- [x] Recover the Orbis 1D texture-array factory and platform constructor.
- [x] Reconstruct Orbis 1D texture-array storage, uploads, and destruction.
- [x] Reconstruct Orbis 1D texture-array shader-stage binding.
- [x] Recover the common 1D texture-array and exact Orbis subclass layouts.
- [x] Recover the Orbis 2D texture-array factory and platform constructor.
- [x] Reconstruct Orbis 2D texture-array storage, views, and destruction.
- [x] Reconstruct Orbis 2D texture-array shader-stage binding.
- [x] Recover the common 2D texture-array and exact Orbis subclass layouts.
- [x] Recover the Orbis cube texture-array factory and platform constructor.
- [x] Reconstruct Orbis cube texture-array storage, uploads, and destruction.
- [x] Reconstruct Orbis cube texture-array shader-stage binding.
- [x] Recover the common cube texture-array and exact Orbis subclass layouts.
- [x] Recover the Orbis compute-buffer factory and repair adjacent function boundaries.
- [x] Reconstruct Orbis compute-buffer storage, updates, binding, and destruction.
- [x] Recover typed active-bank descriptor and allocation selection for Orbis
  compute buffers.
- [x] Reconstruct the common render compute-buffer factory and 80-byte base layout.
- [x] Correct the compute-buffer descriptor's stride/count field order.
- [x] Reconstruct the Orbis particle-buffer allocation, upload, draw, and destruction paths.
- [x] Reconstruct the common render particle-buffer factory and 64-byte base layout.
- [x] Reconstruct the Orbis occlusion-query lifecycle and conditional-rendering commands.
- [x] Reconstruct the common render occlusion-query factory and 64-byte base layout.
- [x] Recover the Orbis vertex, geometry, pixel, and compute shader factory.
- [x] Reconstruct Orbis compute and pixel shader backends.
- [x] Reconstruct Orbis geometry and vertex shader backends.
- [x] Reconstruct the common render-shader lifecycle and layout.
- [x] Reconstruct the common render-texture lifecycle and 168-byte base layout.
- [x] Type the shared render-texture usage field and color/depth backend
  dispatch.
- [x] Recover the complete 80-byte mip-chain descriptor and correct the 1D,
  2D, and 3D texture descriptor layouts to 224 bytes.
- [x] Recover the exact typed 144-byte common texture-descriptor state and use
  it across every reconstructed texture family.
- [x] Replace derived texture descriptor byte arrays and offset writes with
  typed format, extent, source, and array-count fields.
- [x] Type the base texture's mirrored creation and resolved descriptor state.
- [x] Centralize typed mip-chain state and array lifecycle boundaries across
  every common texture dimension.
- [x] Centralize the verified 44-byte texture creation state within the common
  descriptor model.
- [x] Recover the typed 312-byte render-system core prefix, including lock,
  active-frame, epoch, timing, settings, and factory state.
- [x] Reconstruct common render-system core-prefix construction, inline owner
  list setup, dynamic-array teardown, and recursive mutex lifetime.
- [x] Type the 128-byte platform configuration, its defaults, resolution-vector
  lifetime, fixed array accessor, and platform-seven boot predicate.
- [x] Reconstruct all fixed platform capability profiles and both capability
  mask words without an adapter.
- [x] Reconstruct common primary and auxiliary frame preparation, attachment,
  submission control flow, timing, and epoch advancement.
- [x] Recover the 128-byte common GPU-stat block, typed frame query ID, direct
  query-end dispatch, context scope pop, and history-ring transition.
- [x] Reconstruct GPU-stat block construction, statistic ownership teardown,
  pointer-array release, and recursive mutex lifetime.
- [x] Reconstruct GPU-stat root and hardware-counter initialization, pointer
  array growth, counter metadata publication, and final sorting.
- [x] Replace the GPU root-statistic sorting adapter with the verified key at
  statistic offset `0x28`.
- [x] Reconstruct GPU-stat frame begin, nested context scopes, query-ID history
  growth, and direct render-context begin dispatch.
- [x] Reconstruct sorted GPU-statistic lookup and creation, root association,
  child-list growth, and hardware-counter metadata propagation.
- [x] Type GPU root-statistic child/history ownership and reconstruct its
  reverse array and embedded-name teardown.
- [x] Reconstruct GPU query resolution, two-slot result accumulation,
  50-frame smoothing, total remainder, and root aggregation.
- [x] Reconstruct primary-context submission-resource collection and direct
  render-context vtable dispatch.
- [x] Recover the two-slot audio-analysis texture owner and reconstruct its
  per-frame width validation and rebuild decision.
- [x] Recover the global Bink render manager and reconstruct its four-slot
  pending-video conversion loop.
- [x] Reconstruct render phase callback dispatch and partial-framerate epoch
  phase publication.
- [x] Recover the render runtime's resource-manager, lighting, backend,
  primitive-mesh, audio-analysis, and GPU-stat initialization/teardown order.
- [x] Identify the 344-byte render-system resource as the FogDeferred shader
  and reconstruct its allocation, dispatch, registration, and slot lifetime.
- [x] Recover the 712-byte resource-manager construction layout, fixed
  registries, handle sentinels, pointer-array owner, and list teardown.
- [x] Type the resource-manager runtime ownership region and reconstruct full
  shutdown of its array owners, specialized state, and 36 dynamic resources.
- [x] Recover the specialized resource-manager state's six reverse-destroyed
  arrays of 40-byte name records.
- [x] Reconstruct resource-manager finalization and its four-layer,
  128-sample function-table texture.
- [x] Recover the four clamped transfer curves used to populate the renderer's
  function-table texture.
- [x] Reconstruct primary-shader finalization, virtual mode selection, and
  startup-option gating.
- [x] Reconstruct lazy primary-shader support allocation, parameter-registry
  defaults, and `HX_NUM_RT_SLICES` binding setup.
- [x] Correct the primary-shader support objects to the typed constant block
  and 864-byte backend binding state, including owned-array teardown.
- [x] Correct the primary-shader's 32-byte support array to the shader constant
  registry and route construction and teardown through its typed lifecycle.
- [x] Reconstruct FogDeferred's color-space permutation, falloff constant, and
  sky, linear-depth, and function-table texture bindings.
- [x] Separate primary-shader resources into their own domain and reconstruct
  common construction, source-owned abstract dispatch, support-object teardown,
  list unlinking, and compiled-array destruction.
- [x] Type the shared shader-parameter binding and registry layouts and
  reconstruct the resource manager's four built-in permutation bindings.
- [x] Reconstruct shader-parameter range packing, bit-mask generation,
  record-array growth, and caller binding publication.
- [x] Recover the resource manager's 304-byte shader-constant state, including
  the named block owners, uniform handles, transient blocks, and registry.
- [x] Reconstruct shader constant-block construction, member-array growth,
  scalar/array/sliced registration, and the manager's built-in block layout.
- [x] Reconstruct Metal/HLSL constant-block source emission, registry source
  emission, and the resource manager's FNV-1a shader-source hash.
- [x] Reconstruct the 129-record shader constant registry, including all 13
  source comment groups and 116 named integer definitions.
- [x] Reconstruct the shared 16-byte render resource name, including its
  capacity-prefixed owned string, empty representation, growth, and teardown.
- [x] Reconstruct top-level resource-manager initialization, name its 35 shader
  slots, and recover the async-compute-gated allocation sequence.
- [x] Reconstruct primary-shader registration and late-registration finalize
  behavior.
- [x] Reconstruct post-base defaults for the FXAA, DOF sprite, display shading
  mode, sphere-map, linear-depth, scene-mask, and test-pattern shaders.
- [x] Reconstruct compact compute-shader construction for blur classification,
  depth range, DOF disc blur, SSAO, CMAA, and signed-distance passes.
- [x] Reconstruct both signed-distance compute-shader dispatches, sampled
  inputs, unordered outputs, and distance/tile constant layouts.
- [x] Reconstruct the blur-classification compute-shader dispatch, texture
  bindings, unordered classification output, and tile constant layout.
- [x] Reconstruct the DOF disc-blur compute-shader dispatch, sampled scene
  inputs, texture and structured outputs, constants, and tile definitions.
- [x] Reconstruct the volumetric-scattering density, accumulation, and deferred
  compute shaders, including permutations, typed light buffers, and constants.
- [x] Reconstruct the render-test compute shader and retire the remaining
  generic compute-dispatch adapter from the built-in shader constructors.
- [x] Reconstruct the FXAA graphics-shader dispatch, sampled source texture,
  reciprocal-dimensions constant, and constant-block extent.
- [x] Reconstruct the DOF sprite graphics shader, bokeh texture binding, and
  typed `CSBokehSprite` vertex-stage buffer input.
- [x] Reconstruct the downsample graphics shader, all color/bloom permutations,
  texel-offset constant, and pixel-stage texture binding metadata.
- [x] Reconstruct the graphics linear-depth shader dispatch and sampled depth
  texture binding beside its compute-path counterpart.
- [x] Reconstruct the refine and stencil scene-mask graphics shaders, including
  stage-specific mask inputs, tile counts, and the vertex-only variant.
- [x] Reconstruct the shading-mode, sphere-map, and cube-texture display
  shaders with their debug constants, permutations, and sampled resources.
- [x] Reconstruct the error and basic fallback shaders, their geometry/shading
  permutations, texture modes, constants, and sampled resources.
- [x] Reconstruct the bloom graphics shader, half-size and hue-preservation
  permutations, three sampled bloom inputs, and bloom/overbright constants.
- [x] Reconstruct the blur graphics shader, sample-count/type/direction and
  mask/classification permutations, weighted-offset array, and four textures.
- [x] Reconstruct the output-conversion graphics shader, HMD-mask, BT.2020,
  and perceptual-quantizer permutations, minimum intensity, and both textures.
- [x] Reconstruct the Bink conversion graphics shader, alpha-plane
  permutation, four YCrCbA plane textures, and scale/offset constants.
- [x] Reconstruct the test-pattern and render-test-simple graphics shaders,
  retiring the last built-in dispatch adapters and the shared adapter file.
- [x] Reconstruct the `BinStream` endian/cipher read and seek primitives,
  the Park-Miller stream cipher, and the compiled-shader cache object loader.
- [x] Reconstruct the engine file wrappers, the complete `BinStream` base
  dispatch and endian/cipher I/O, and the 592-byte `FileStream`.
- [x] Reconstruct the compiled-shader cache validators: backend-state
  declaration hashing, source-file hashing, and global define matching.
- [x] Extend every source-owned primary-shader dispatch to the binary's 11
  slots, with permutation validators and fallback/slice/geometry overrides.
- [x] Reconstruct stage permutation enumeration and the primary-shader
  layout hash used to validate compiled-shader caches.
- [x] Reconstruct primary-shader backend initialization, compiled-cache
  validation and loading, and its recursive critical section.
- [x] Reconstruct the primary-shader permutation bind, compiled-object
  stage binding, context stage unbinding, and the error-shader fallback bind.
- [x] Reconstruct the bloom draw function with shared pass-draw helpers for
  pixel texture binding, per-draw constant buffers, and key packing.
- [x] Reconstruct the output-conversion draw function and the shared
  sRGB-to-linear color conversion.
- [x] Reconstruct the downsample draw function, including its refined
  hardware-reciprocal texel offset and HDR10 color-space selection.
- [x] Reconstruct the single-texture linearize-depth, refine-scene-mask,
  and display-sphere-map draw functions.
- [x] Reconstruct the test-pattern and render-test-simple draw functions.
- [x] Reconstruct the DOF sprite draw and generalize draw-time binding to
  per-stage context slot limits and compute-buffer stage binds.
- [x] Adopt the original names and module layout from the reference map, and
  convert the engine foundation (`Rand2`, `Hmx::Color`, `BinStream`,
  `FileStream`, `File`, `String`, `Symbol`, `Thread`, `MemMgr`, options) to
  real classes under `src/math`, `src/utl`, and `src/os`.
- [x] Convert the render resource objects to original classes: the
  `RndShaderResource` base, `RndComputeBuffer`, `RndShaderCBuffer`,
  `RndParticleBuffer`, `RndOcclusionQuery`, `RndShaderProgram`, `RndFactory`,
  and their PS4 subclasses, including `PS4Fence`.
- [x] Convert textures to original classes: `RndPixelFormat`, `RndPixelData`
  (with its vtable and pool allocation), `RndPixelDataCube`, `RndTextureBase`,
  the seven texture kinds, their PS4 subclasses, and `PS4RenderUtl`'s
  stage selects; introduce a shared `eastl::vector`. Fix `SyncStatic`'s
  keep-pixel-data test and remove a border-color parameter the binary never
  had.
- [x] Convert meshes to original classes: `RndDrawable`, `RndDynamicGpuData`,
  `RndMesh`, and the `RndMeshTyped`/`PS4MeshTyped` templates, which replace
  seven copied per-layout files. Forward the update context instead of null.
- [x] Convert the render context to original classes: `RndContext` (constructor,
  `Init`, `Terminate`, and the 34-slot vtable) and `PS4Context`, whose slot
  bodies move from free functions into methods. Merge the submission-resource
  record into `RndResourceBarrier`.
- [x] Reconstruct all built-in graphics-shader constructors, including their
  shared permutation bindings and verified trailing handle state.
- [x] Reconstruct the seven remaining compute-shader constructors, completing
  source ownership of all 35 built-in resource-manager shader constructors.
- [x] Recover the 304-byte lighting-resource state, constructor defaults,
  fixed owners, pointer arrays, runtime shutdown, and destructor.
- [x] Recover the 40-byte inline primitive-mesh set and its box/cylinder
  ownership lifecycle.
- [x] Reconstruct the render-system deferred-release queue, direct deleting
  dispatch, growth, drain, and shutdown behavior.
- [x] Reconstruct deferred-release queue construction, capacity teardown,
  recursive mutex lifetime, and adjacent frame-phase initialization.
- [x] Reconstruct the four built-in render constant buffers, their fixed CPU
  values, backend uploads, ownership, and shutdown release.
- [x] Reconstruct common texture-descriptor defaults and source-data detection.
- [x] Reconstruct the shared texture creation-profile merge, resolved-field and
  sampler defaults, flag propagation, and special usage-class behavior.
- [x] Reconstruct typed descriptor construction for all seven common texture
  dimensions and use those constructors in default-resource creation.
- [x] Reconstruct the common 80-byte mip-chain vector construction, doubling
  growth, deep-copy append, validation, and ownership teardown.
- [x] Type and reconstruct recursive mip-level ownership, deep source-pixel
  copies, auxiliary release, and common descriptor/state teardown.
- [x] Reconstruct six-face cube state ownership and cube-array reserve, deep
  append, validation, capacity growth, and teardown.
- [x] Reconstruct 1D/2D texture-array dimensional and cross-layer mip
  validation, including the original 2D array size limit.
- [x] Recover the texture data-format bit widths used by mip source allocation,
  including storage reuse and optional source copying.
- [x] Reconstruct the full 85-ID data-format descriptor table and its 31-entry
  compact variant bit-width mapping.
- [x] Reconstruct exact data-format lookup for fixed descriptor tuples and all
  compact platform-variant layout families.
- [x] Reconstruct platform-aware data-format resolution, supported-format mask
  checks, channel fallbacks, and resource-class-eight conversion behavior.
- [x] Reconstruct fixed-format float-image conversion, including all channel
  orders, normalized and floating component widths, half conversion, and sRGB
  encoding.
- [x] Reconstruct shared spot-shadow depth-array rebuilding, including active
  configuration selection, all seven resolutions, per-layer mip descriptors,
  replacement ownership, and direct tiled-light owner slots.
- [x] Recover the shared deleting-dispatch slot for textures, compute buffers,
  meshes, and render targets and remove their dynamic-release adapters.
- [x] Reconstruct common compute-buffer base dispatch, staging ownership,
  backend initialization, and deleting storage release.
- [x] Reconstruct clear/copy buffer compute-shader dispatch, numeric/texture
  permutations, typed buffer bindings, texture bindings, and clear constant.
- [x] Reconstruct common constant-buffer base dispatch and deleting storage
  release.
- [x] Reconstruct common particle-buffer base dispatch and deleting storage
  release.
- [x] Reconstruct common occlusion-query base dispatch, intrusive unlinking,
  and deleting storage release.
- [x] Reconstruct common render-target base dispatch, owned-state teardown,
  and deleting storage release.
- [x] Reconstruct common render-shader base dispatch, backend initialization,
  release routing, and deleting storage release.
- [x] Reconstruct common render-texture base dispatch, precache-mode backend
  suppression, and deleting storage release.
- [x] Reconstruct the common 1D-texture dispatch, mip/source queries,
  creation-state replacement, source-data release, and deleting lifecycle.
- [x] Reconstruct the common 3D-texture dispatch, mip/source queries,
  creation-state replacement, source-data release, and deleting lifecycle.
- [x] Reconstruct the common 2D-texture dispatch, mip/source queries,
  linked-resource resolution, source-data release, and deleting lifecycle.
- [x] Reconstruct the common 1D-array texture dispatch, per-layer mip/source
  queries, source-data release, and deleting lifecycle.
- [x] Reconstruct the common 2D-array texture dispatch, per-layer mip/source
  queries, source-data release, validation, and deleting lifecycle.
- [x] Reconstruct the common cube-texture dispatch, six-face source queries,
  source-data release, validation, and deleting lifecycle.
- [x] Reconstruct the common cube-array texture dispatch, cube/face source
  queries, source-data release, validation, and deleting lifecycle.
- [x] Reconstruct common render-mesh dispatch, update-link registration,
  triangle storage, position-vertex access, and deleting lifecycle.
- [x] Replace all seven Orbis mesh format adapters with source-owned dispatch,
  CPU vertex storage, double-buffered GPU storage, and index conversion.
- [x] Reconstruct the synchronous and asynchronous FMOD file I/O bridge.
- [x] Reconstruct the engine-to-FMOD listener transform bridge.
- [x] Reconstruct the `HMX.BufferedOutput` plugin and custom-output startup path.
- [x] Reconstruct the FMOD pre/post-mix callback and timing lifecycle.
- [x] Reconstruct the 128-sample audio output listener dispatch.
- [x] Reconstruct external FMOD Studio system attachment and detachment.
- [x] Reconstruct deferred FMOD channel and DSP release processing.
- [x] Recover the engine-to-FMOD speaker configuration mapping.
- [x] Reconstruct audio timing snapshots and per-source aggregation.
- [x] Reconstruct `AudioClipFmod` channel and event teardown.
- [x] Reconstruct `AudioClipFmod` playback setup and routing.
- [x] Reconstruct `AudioClipFmod` Studio DSP binding.
- [x] Reconstruct `AudioClipFmod` runtime playback controls.
- [x] Reconstruct the `FmodAudioBusGeneratorManager` pool and handle format.
- [x] Reconstruct `FmodAudioBusGenerator` sound creation and Studio-bus routing.
- [x] Reconstruct the `FmodAudioStreamGeneratorManager` pool and handle format.
- [x] Reconstruct the core `FmodAudioStreamGenerator` playback lifecycle.
- [x] Identify the `FmodBufferedStreamGenerator` pool and control surface.
- [x] Reconstruct the normal buffered-stream PCM render path.
- [x] Reconstruct the `FmodDialogGeneratorManager` pool and resource routing.
- [x] Reconstruct the `FmodDialogGenerator` Studio-event lifecycle.
- [x] Reconstruct the generic `FmodStudioSoundGeneratorManager` pool.
- [x] Reconstruct `FmodAudioStreamResource` loading and path registration.
- [x] Reconstruct the `FMODSoundToPCMCallback` decode bridge.
- [x] Reconstruct `FModBankResource` loading and bank enumeration.
- [x] Reconstruct FMOD audio-input bus routing and record-driver discovery.
- [x] Reconstruct the FMOD-backed recording audio render target.
- [x] Identify the synchronized six-sample interpolation kernel.
- [x] Reconstruct the `PS4TextureCube` color and depth syncs and destructor.
- [x] Type the PS4 textures as `sce::Gnm::Texture` and reconstruct the 1D,
  3D, 1D-array and cube-array static syncs with `PS4RenderUtl::GetDataFormat`
  and `GetSurfaceType`.
- [x] Reconstruct the `PS4OcclusionQuery` query and predication commands.
- [x] Reconstruct `PS4ComputeBuffer::_SyncStaticImpl` and its stage selects.
- [x] Reconstruct the four `PS4ShaderProgram*` classes with
  `RndShaderCompilerBlob` and the Gnmx shader parser and fetch-shader
  generators; model `PS4Context`'s 18 `ComputeContext`s; reconstruct
  `PS4ShaderCBuffer::_SelectImpl`, `PS4Context::SetCbEnabled` and
  `RndContext::_ReselectGlobalCBuffers`.
- [x] Move the last `rb4` code into the global namespace: SDK
  `sce::Gnm::RenderTarget`, `DepthRenderTarget`, `DataFormat` and `SizeAlign`
  replace the Orbis stand-ins, and the blend-mode enum takes the map's name
  `RndBlendMode`.
- [x] Reconstruct `PS4ParticleBuffer::_DrawBatchImpl` and the particle buffer
  creation (`MemFindHeap`, `MemPushHeap`, `MemPopHeap`).
- [x] Use the SDK's `sce::Gnm` and `sce::Gnmx::GfxContext` types in the PS4
  backend, and reconstruct `PS4Context::SetupDraw` and
  `PS4RenderUtl::GetPrimitiveType`.
- [x] Pass the screenshot render callback as `std::function<void()>`.
- [x] Name the main loop's calls from the map: `SystemInit`, `SystemPoll`,
  `InitRBLayoutDefines`, `StagePresence::InitEnumMacros`, `SoundManager::Poll`,
  `RBProfileMgr::Poll`, and `UIMgr::Poll`, `GetCurrentLayout` and `Draw`.
- [x] Convert the remaining render helpers: `Vector2i` extents, `PlatformMgr`
  and `HxPlatform`, the `RndPixelFormat` data-format table,
  `RndPrimitiveMeshes`, `RndAudioTextures`, `BinkRenderMgr`, and the tiled
  light buffers in `RndLightMgrCom`.
- [x] Convert the render debug and shader helpers: `RndBufferInspection`,
  `ToString(RndUserShadingMode)`, `RndShaderIncludeChecksums`, `RndShaderUtl`,
  `RndShaderResource::Select`, and `RndShaderDrawUtl` for the inlined draw
  helpers that the map does not name.
- [x] Move the game, input and UI code into their original modules: `App` and
  `main` in `src/rockband/app/Main.cpp`, `StagePresence` in `src/rb_game`,
  `PembrokeGuitarController` in `src/os/joypads`, and `UILayoutId` in
  `src/ui/layout`.
- [x] Move the remaining `src/render/core` and `src/render/resources` files
  into the `src/render` domain folders.
- [x] Fold `src/render/platform/orbis` into the PS4 classes: the context and
  synchronization code into `PS4Context`, the video output into `PS4Device`
  and `PS4Window`, and the mesh, buffer, and shader helpers into
  `PS4MeshTyped`, `PS4RenderUtl`, `PS4TransientBuffer`, and the new
  `RndVertexInterpreter`.
- [x] Convert the render settings to `RndConfig` and the platform
  configurations to `RndCapabilities`.
- [x] Convert the lighting resources to `RndLightGlobals`, the debug commands
  to `RndCommands`, the render startup to `Rnd::Init` and `Rnd::Terminate`,
  and the Orbis transient vertex buffer to `PS4TransientBuffer`, with
  `PS4RenderStateUtl::InitSampler`.
- [x] Merge the default textures, camera, materials, and lighting into
  `RndDefaults` (`src/render/defaults`).
- [x] Convert the GPU statistics block to `RndGpuStatsMgr`.
- [x] Convert the resource manager to `RndShaderMgr` with flat, typed members.
- [x] Convert the shader system to `RndShader` and `RndShaderCompute`, with
  `RndShaderFixedDefines`, `RndShaderDefines`, `RndShaderDefinesGroup`,
  `RndShaderCBufferConfig`, `RndShaderResourceConfig`, and
  `RndShaderCollection`, and all 35 built-in shaders as subclasses (see
  [render-shader-classes.md](render-shader-classes.md)).
- [x] Convert the render targets to `RndBufferCollection` and
  `RndBufferCollection2D`, merging the 17 per-buffer files into the map's
  object and restoring the missing buffer registrations (see
  [render-buffer-collection.md](render-buffer-collection.md)).
- [x] Convert the render windows to `RndWindow`, `RndBufferedWindow`, and
  `PS4Window` with their seven-slot vtables (see
  [render-window.md](render-window.md)).
- [x] Convert the render device to `RndDevice` and `PS4Device` with all 17
  virtual slots, a fully asserted layout, `Rnd::PlatformCreateDevice`,
  `CritSec`, `ScopedCritSec`, and `Condition`; recover the deferred-context,
  console-state, and material pending-free entry points (see
  [render-device.md](render-device.md)).
- [x] Convert the audio and microphone subsystems to the reference map's
  classes (`FModSystem`, the FMOD generators and managers, `FmodFileWrapper`,
  `MicHwManager_FMOD`, `Mic_FMOD`, the DSP plug-ins) and move `Vector3`,
  `Hmx::Matrix3`, and `Transform` into `src/math`.
- [x] Establish a compatible PS4 object-build and structural-comparison loop.
- [ ] Link a complete reconstructed executable after recovering the remaining
  engine adapters and external FMOD libraries.

## Validation constraint

The game targets the PS4 SDK 5.000 generation. The local 5.008 installed files
contain headers and stub libraries but no Sony compiler or linker, and the
matching toolchain is unavailable. Static reconstruction, IDA naming, import
resolution, and documentation continue independently. The installed SDK 5.500
toolchain now compiles every reconstructed translation unit and generates an
object archive, hash manifest, unresolved-symbol report, and optional assembly
listings. Its code generation is not expected to match the original build byte
for byte.

## Naming conventions

- Confirmed names use their original spelling when recovered from RTTI,
  assertions, logging strings, imports, or source paths.
- Inferred names use a descriptive `subsystem_action` form until stronger
  evidence becomes available.
- Unidentified functions keep IDA's address-based name.
- Every reconstructed function records its source address in a nearby comment.

## Evidence policy

Function names and types should be tied to at least one concrete signal:
callers/callees, referenced strings, RTTI, vtable position, imported APIs,
structure offsets, or behavior visible in pseudocode. Guesses are marked as
such rather than silently promoted to confirmed names.
