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
- [x] Reconstruct render-system runtime initialization and shutdown ordering.
- [x] Reconstruct Orbis render-system allocation and object lifetime.
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
