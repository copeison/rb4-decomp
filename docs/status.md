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
- [x] Reconstruct top-level default render-resource initialization ordering.
- [x] Reconstruct fallback directional-light creation and activation.
- [x] Recover default texture lookup and shadow-offset accessors.
- [x] Reconstruct default render-resource polling and teardown.
- [x] Recover default render-resource construction and its `100.0` light scale.
- [x] Reconstruct the render-system poll, begin, end, and skipped-frame control flow.
- [x] Reconstruct pre-frame screenshot capture and its resolution table.
- [x] Recover renderer draw-mode and buffer debug-view tables and setters.
- [x] Recover and name the 24-command renderer debug console registry.
- [x] Reconstruct renderer settings defaults, config keys, and capability gates.
- [x] Recover the Low, Medium, and High renderer quality-level mapping.
- [x] Recover runtime resolution parsing and screenshot-mode labels.
- [x] Distinguish the configured vsync mode from the runtime enable flag.
- [x] Reconstruct base render-system construction and destruction ordering.
- [x] Recover the common render-system singleton and shared epoch accessor.
- [x] Reconstruct render-system runtime initialization and shutdown ordering.
- [x] Reconstruct Orbis render-system allocation and object lifetime.
- [x] Recover the separate Orbis render-system singleton lifecycle.
- [x] Reconstruct Orbis video-output and submit-thread startup.
- [x] Reconstruct Orbis video-output and submit-thread shutdown.
- [x] Reconstruct Orbis GPU idle waits and deferred-allocation retirement.
- [x] Reconstruct the Orbis frame-submit handshake.
- [x] Complete the Orbis deferred GPU-allocation queue lifecycle.
- [x] Reconstruct the guarded game-system shutdown callback.
- [x] Reconstruct the matching game-system renderer startup sequence.
- [x] Recover renderer platform and graphics-API identity mappings.
- [x] Reconstruct platform capability and resolution-list initialization.
- [x] Recover the Orbis GPU fence factory and backing allocation.
- [x] Recover the seven-format Orbis mesh factory and format-name map.
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
- [x] Reconstruct the common Orbis mesh draw and instancing path.
- [x] Reconstruct Orbis back-buffer allocation and video-output registration.
- [x] Reconstruct the common render-target lifecycle, 32-byte base layout, and
  active-state header fields used for dimensions and debug modes.
- [x] Reconstruct Orbis graphics/compute render-context allocation.
- [x] Reconstruct Orbis graphics/compute frame submission and reset.
- [x] Reconstruct Orbis render-target, blend, and default pipeline state.
- [x] Recover Orbis depth, stencil, raster, and color-write state setters.
- [x] Recover Orbis sampler construction and shader unbinding.
- [x] Reconstruct shared Orbis texture and sampler stage binding.
- [x] Recover masked Orbis shader-resource clearing.
- [x] Reconstruct accelerated and raster Orbis depth-stencil clears.
- [x] Reconstruct format-specific Orbis transient vertex drawing.
- [x] Recover the Orbis GPU fence destruction and deferred release paths.
- [x] Recover Orbis GPU fence sequencing, signaling, and command-buffer waits.
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
- [x] Reconstruct the common render compute-buffer factory and 80-byte base layout.
- [x] Reconstruct the Orbis particle-buffer allocation, upload, draw, and destruction paths.
- [x] Reconstruct the common render particle-buffer factory and 64-byte base layout.
- [x] Reconstruct the Orbis occlusion-query lifecycle and conditional-rendering commands.
- [x] Reconstruct the common render occlusion-query factory and 64-byte base layout.
- [x] Recover the Orbis vertex, geometry, pixel, and compute shader factory.
- [x] Reconstruct Orbis compute and pixel shader backends.
- [x] Reconstruct Orbis geometry and vertex shader backends.
- [x] Reconstruct the common render-shader lifecycle and layout.
- [x] Reconstruct the common render-texture lifecycle and 168-byte base layout.
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
