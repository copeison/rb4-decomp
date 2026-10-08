# Source reconstruction

Files under `src/` are cleaned C++ reconstructions. Generated Hex-Rays output
stays under `analysis/exports/` and is used as evidence rather than copied into
the source tree.

| Address | Reconstructed symbol | Source | Status |
| --- | --- | --- | --- |
| `0xA0` | `game_initialize` | `src/game/initialize.cpp` | Complete top-level initialization order and arguments recovered; two option-field meanings remain unknown. |
| `0x190` | `game_run_frame` | `src/game/frame.cpp` | Update order and render/skip/exit control flow recovered; several owning class names remain unknown. |
| `0x3C0` | `game_main` | `src/game/main.cpp` | Control flow recovered; dependent functions are still being reconstructed. |
| `0x402C30`, `0x402D30` | game-system renderer lifecycle | `src/game/systems.cpp` | Orbis renderer creation, platform warmup, shared and default resource initialization, nine dependent initializers, cleanup registration, guarded shutdown, virtual deletion, and singleton clearing recovered. |
| `0x997590` | `stage_presence_id_to_symbol` | `src/game/stage_presence.cpp` | All 22 enum values and their interned symbol strings recovered. |
| `0xBB06A0`, `0xBB5D40` | `ui_layout_id_to_symbol`, layout asset map | `src/ui/ui_layout_id.cpp` | All 108 ordered IDs and their 92 direct layout paths recovered. |
| `0x252BC0` | `command_line_mark_switches_handled` | `src/core/command_line.cpp` | Complete behavior and observed container layout reconstructed. |
| `0x8D28A0`, `0x8D2EA0`, `0x8D3000`, `0x8D3060` | special pad reader and calibration sample path | `src/input/special_pad_reader.cpp` | Hardware probing, sensor-mode report, rolling sample collection, and transfer recovered. |
| `0x2773C0`, `0x278270` | `fmod_audio_initialize`, custom DSP registration | `src/audio/fmod_audio_system.cpp` | FMOD 1.10.04 startup, advanced settings, file callbacks, driver format, DSP buffers, and all 12 custom plugins recovered. |
| `0x258C60`, `0x2590B0`, `0x261F60` | PS4 FMOD module and thread-affinity setup | `src/audio/fmod_orbis_platform.cpp` | PRX loading, engine affinity-group lookup, CPU-mask construction, and the 11-entry private FMOD affinity table recovered. |
| `0x6BEF40`-`0x6C0160` | default render lighting and fallback | `src/render/default_lighting.cpp` | `RndSceneResource` loading, authored light/probe discovery, object-ID lists, mode switching, scale-dependent probe/spot setup, and fallback directional-light creation recovered. |
| `0x6BEC50` | default render materials | `src/render/default_materials.cpp` | Six named material objects, shader-graph paths, unique sharing, and explicit additive/source blend modes recovered. |
| `0x6BEBC0` | default render camera | `src/render/default_camera.cpp` | Creation of `default_cam` and attachment of its `RndCameraCom` recovered. |
| `0x6BDE60`, `0x6C00F0` | default fallback textures | `src/render/default_textures.cpp` | Seven named color/error families, exact extents, all seven texture shapes, and indexed lookup recovered. |
| `0x6BDCA0`, `0x6BF860`, `0x6BFA00` | default render-resource lifecycle | `src/render/default_render_resources.cpp` | Feature gate, initialization order, per-frame scene polling, and complete texture/compute/scene teardown recovered. |
| `0x3DD410`, `0x3DD790` | render-system construction and destruction | `src/render/render_system_lifecycle.cpp` | Core synchronization, 13 platform slots, default and backend resources, singleton publication, settings ownership, and reverse teardown recovered. |
| `0x1AE4D0`, `0x363030`, `0x4414A0`, `0x8D5DE0` | renderer platform and API identities | `src/render/render_platform.cpp` | Thirteen platform slots, seven exact API names, configured platform-to-API lookup, and the fixed Orbis API recovered. |
| `0x3641B0`, `0x6B9940`, `0x6B99B0` | renderer platform configuration | `src/render/render_platform_config.cpp` | Supported-platform selection, fixed capability profiles, validated resolution lists, 1,920 × 1,080 fallback, and sorting recovered. |
| `0x3DDAE0`-`0x3DDFE4` | render-system runtime lifecycle | `src/render/render_system_runtime.cpp` | Backend startup, four built-in buffer mappings, frame-owner initialization, runtime epoch tracking, deferred-release drain, and shutdown ordering recovered. |
| `0x8D5DF0`, `0x8D77F0`, `0x8D79B0` | Orbis render-system object lifetime | `src/render/orbis_render_system.cpp` | 4,352-byte allocation, base construction, video and worker state, recursive submission locks, deferred-command list, platform singleton, and reverse destruction recovered. |
| `0x8D7340`-`0x8D80BB` | Orbis video output and submit worker | `src/render/orbis_video_output.cpp` | Video port, EOP queue, flip events, default vertex buffers, submit thread, fence retirement, runtime vsync application, shutdown, and deleting destructor recovered. |
| `0x8D8100`-`0x8D857A` | Orbis GPU synchronization | `src/render/orbis_gpu_sync.cpp` | Ten-counter idle barrier, active-frame flushing, and the complete mutex-protected deferred-allocation queue lifecycle recovered. |
| `0x8D8300` | Orbis frame submission | `src/render/orbis_frame_submit.cpp` | Submit-token condition wait, active-command flush, primary frame-owner submission, and collected-object finalization recovered. |
| `0x8D85C0`, `0x8E1570` | Orbis GPU fence | `src/render/orbis_fence.cpp` | 24-byte object factory and zeroed four-byte `PS4Fence` GPU allocation recovered. |
| `0x27A1C0`-`0x27A850` | FMOD file callbacks and asynchronous reader | `src/audio/fmod_file_io.cpp` | Open, close, read, seek, priority queue, worker, cancellation, and shutdown behavior recovered. |
| `0x262300`, `0x27ACB0` | listener update and engine-to-FMOD transform conversion | `src/audio/fmod_listener.cpp` | Primary listener gating, 48-byte transform layout, handedness conversion, and zero velocity recovered. |
| `0x2763F0`-`0x276520`, `0x2786D0` | `HMX.BufferedOutput` callbacks and custom-output initialization | `src/audio/fmod_buffered_output.cpp`, `src/audio/fmod_audio_system.cpp` | Output descriptor, 128 virtual drivers, format negotiation, update dispatch, and update-driven FMOD flags recovered. |
| `0x2781C0`, `0x2783E0`, `0x278880`-`0x2789CD` | mix-buffer dispatch and FMOD pre/post-mix callback | `src/audio/fmod_mix_callback.cpp` | Semaphore ownership, mix sequence, source reset, observer dispatch, and cumulative/rolling timing recovered. |
| `0x1127880` | output-block listener dispatch | `src/audio/audio_output_dispatcher.cpp` | Pending-list promotion, two-phase listener calls, atomic guards, and 128-sample subdivision recovered. |
| `0x277840`, `0x277A80` | attach or detach an external FMOD Studio system | `src/audio/fmod_audio_system.cpp` | User-data binding, format discovery, DSP registration, callback enablement, and synchronized detach recovered. |
| `0xD3BA0`, `0xD3BC0` | global audio mix-format accessors | `src/audio/audio_mix_format.cpp` | Sample rate, reciprocal, buffer cadence, and milliseconds-per-buffer calculations recovered. |
| `0x1128380`, `0x278A00`, `0x278DF0` | double-buffered deferred FMOD release queue | `src/audio/fmod_deferred_release.cpp` | Mix-consumer registration, enqueue, buffer swap, channel stop, DSP release, and detach-time clearing recovered. |
| `0x2783A0` | engine speaker configuration mapping | `src/audio/fmod_audio_system.cpp` | Mono, stereo, 5.1, and 7.1 FMOD modes and raw channel counts recovered. |
| `0x277BA0`, `0x278C80`-`0x278D72` | `fmod_audio_consume_timing_report` and percentage getters | `src/audio/fmod_timing_report.cpp` | Atomic snapshot/reset, buffer-duration normalization, rolling-window normalization, and per-source aggregation recovered. |
| `0x267E50`-`0x26837A` | `AudioClipFmod` release lifecycle | `src/audio/audio_clip_fmod.cpp` | Deferred low-level channel release, Studio event teardown, invalid-handle handling, and blocking synchronous-update drain recovered. |
| `0x266CB0`-`0x26759C` | `AudioClipFmod` DSP, playback startup, and Studio event callback | `src/audio/audio_clip_fmod.cpp` | `HMXRawAudioBus`, event/low-level fallback, HMX plugin binding, head-DSP insertion, parent routing, sample-rate base frequency, and callback readiness recovered. |
| `0x267BB0`-`0x268479` | `AudioClipFmod` runtime controls | `src/audio/audio_clip_fmod.cpp` | Pause/resume state transitions, millisecond playback position, 3D attribute updates, and named Studio parameters recovered. |
| `0x268380`, `0x268510`-`0x26943F` | `FmodAudioBusGenerator` pool and creation | `src/audio/fmod_audio_bus_generator.cpp` | Fixed pool, stale-handle rejection, sound creation, and Studio-bus route registration recovered. |
| `0x2692B0`, `0x2693B0`-`0x26AA9F` | `FmodAudioStreamGenerator` pool and playback | `src/audio/fmod_audio_stream_generator.cpp` | MP3 type, 288-byte pool, handles, asynchronous channel startup, loop points, seeking, pause control, volume, 3D updates, and teardown recovered. |
| `0x26AB60`-`0x26E419` | `FmodBufferedStreamGenerator` pool and rendering | `src/audio/fmod_buffered_stream_generator.cpp` | 568-byte pool, resource gate, decoder blocks, seeking, gain, ring refill, normal PCM16 stereo interpolation, synchronized-render controls, and the Optimal 32x six-point, fifth-order kernel recovered. |
| `0x26E990`-`0x2715FA` | `FmodDialogGenerator` and `FmodStudioSoundGenerator` | `src/audio/fmod_dialog_generator.cpp`, `src/audio/fmod_studio_sound_generator.cpp` | Dialog and generic pools, `.bank` routing, `event:/` and `snapshot:/` normalization, programmer sounds, Studio event lifecycle, timeline and parameter controls, 3D updates, and two-stage volume fading recovered. |
| `0x271660`-`0x272E02` | `FmodAudioStreamResource` | `src/audio/fmod_audio_stream_resource.cpp` | Streaming-audio extensions, platform-path load, FMOD PCM16 mono/stereo probing, status codes, normalized-path registry, lookup, and teardown recovered. |
| `0x272E60`-`0x27315C` | `FMODSoundToPCMCallback` | `src/audio/fmod_sound_to_pcm_callback.cpp` | Nonblocking-open wait, format and length query, PCM16 buffer sizing, block decode loop, cancellation, completion, rewind, and teardown recovered. |
| `0x273740`-`0x275180` | `FModBankResource` | `src/audio/fmod_bank_resource.cpp` | PS4 and localized path handling, per-Studio-system bank and sample-data load, bus locking, unload waits, event and bus path enumeration, and paired master-bank routing recovered. |
| `0x275490`-`0x275C00`, `0x27B3E0`-`0x27BA9B` | FMOD audio input manager and record devices | `src/audio/fmod_audio_input_manager.cpp` | Studio bus binding, authored-volume scaling, mute and channel-group access, fixed device slots, `GENERAL` driver filtering, duplicate suppression, and connection reconciliation recovered. |
| `0x275E20`-`0x2763C0` | FMOD recording audio render target | `src/audio/fmod_recording_audio_render_target.cpp` | Embedded FMOD state delegation, buffered-output mixer reads, dual-layer locking, mix-consumer dispatch, and asynchronous recording-thread lifecycle recovered. |

## Game initialization

`game_initialize` performs a fixed startup sequence: core services, UI and
stage-presence identifiers, `config/rockband.dta`, sound, engine type
registrations, primary game systems, UI resources, Dingo backend networking,
time-stretch audio, and player assignments. It then loads
`kLayoutGameStartup` and marks remaining command-line switches handled.

The game-system option block is exactly 16 bytes. Its first three bytes are set
to true and its final eight bytes are zero. Rendering initialization reads the
second flag directly; the meanings of the first and third flags are retained as
unknown fields until their virtual consumer is recovered.

## Command-line argument layout

IDA shows 16-byte entries containing a string pointer at offset 0 and a handled
flag at offset 8. The owning object begins with the usual three pointers for a
contiguous container: first element, one-past-last element, and capacity. The
reconstructed function walks the first two pointers and marks every unhandled
argument whose first character is `-`.

The structure names are descriptive because original symbols are unavailable.
Offset and size assertions preserve the observed binary layout.

## Special pad calibration reader

The special pad reader probes devices `0738:8261` and `0E6F:0173` through the
private `scePadOpenExt` API and assigns internal hardware IDs `0x1F` and
`0x20`. It controls a calibration sensor with feature report `0x30`, collects
up to 64 samples from `ScePadData::deviceUniqueData`, and transfers each batch
to calibration code. Numeric mode names are retained until their physical
sensor meanings are proven. See `docs/special-pad-reader.md` for the layouts
and call-site evidence.

## Stage presence IDs

The function at `0x997590` initializes a guarded table of 22 eight-byte engine
symbols and indexes it directly with the requested ID. Every symbol string is
embedded beside the function, which makes the enum order exact. The cleaned
source uses a function-local static array to express the same one-time
initialization without reproducing compiler guard internals.

## UI layout IDs

The function at `0xBB06A0` contains a guarded table for IDs 0 through 107 and a
separate `kLayoutInvalid` result for ID -1. The ordered list lives in
`src/ui/ui_layout_list.inc` so the enum and symbol table share one readable
source of truth. ID `0x2C` is `kLayoutGameStartup`, matching the startup layout
loaded by `game_initialize`.

The loader at `0xBB5D40` supplies direct `.layout` paths for 92 of those IDs.
The same include now drives `ui_layout_primary_path`, keeping every recovered
path aligned with its enum value. Sixteen deprecated, fallback, or unsupported
IDs have no direct path. SOMP IDs 101 and 102 also fall through in the original
switch to preload the subsequent session layouts; the helper reports the first
path associated with each requested ID.
