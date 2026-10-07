# Source reconstruction

Files under `src/` are cleaned C++ reconstructions. Generated Hex-Rays output
stays under `analysis/exports/` and is used as evidence rather than copied into
the source tree.

| Address | Reconstructed symbol | Source | Status |
| --- | --- | --- | --- |
| `0xA0` | `game_initialize` | `src/game/initialize.cpp` | Complete top-level initialization order and arguments recovered; two option-field meanings remain unknown. |
| `0x190` | `game_run_frame` | `src/game/frame.cpp` | Update order and render/skip/exit control flow recovered; several owning class names remain unknown. |
| `0x3C0` | `game_main` | `src/game/main.cpp` | Control flow recovered; dependent functions are still being reconstructed. |
| `0x997590` | `stage_presence_id_to_symbol` | `src/game/stage_presence.cpp` | All 22 enum values and their interned symbol strings recovered. |
| `0xBB06A0`, `0xBB5D40` | `ui_layout_id_to_symbol`, layout asset map | `src/ui/ui_layout_id.cpp` | All 108 ordered IDs and their 92 direct layout paths recovered. |
| `0x252BC0` | `command_line_mark_switches_handled` | `src/core/command_line.cpp` | Complete behavior and observed container layout reconstructed. |
| `0x8D28A0`, `0x8D2EA0`, `0x8D3000`, `0x8D3060` | special pad reader and calibration sample path | `src/input/special_pad_reader.cpp` | Hardware probing, sensor-mode report, rolling sample collection, and transfer recovered. |
| `0x2773C0`, `0x278270` | `fmod_audio_initialize`, custom DSP registration | `src/audio/fmod_audio_system.cpp` | FMOD 1.10.04 startup, advanced settings, file callbacks, driver format, DSP buffers, and all 12 custom plugins recovered. |
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
| `0x26AB60`-`0x26C45A` | higher-level FMOD streaming clip | focused IDA exports | Resource gate, FMOD sound creation, decoder buffers, and nested bus-generator allocation identified; decoder callbacks remain in progress. |

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
