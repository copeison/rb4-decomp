# Reconstruction progress

This report measures how much of the executable's own code has been
reconstructed. It is a snapshot of commit `1fd3e74` ("decomp: reconstruct the
entity instances and the rest of the scene drawer", 2026-10-18 13:21 -0600).
The build outputs it quotes are from the Debug object build of that tree,
written at 13:19:36 the same day. Third-party and SDK code is reported
separately and left out of the totals.

## Summary

| Scope | Functions | Reconstructed | By count | Bytes | Reconstructed bytes | By bytes |
|---|---:|---:|---:|---:|---:|---:|
| Game code (engine and Rock Band) | 51,175 | 4,749 | **9.3%** | 13,054,472 | 1,136,985 | **8.7%** |
| Engine only (all but the `rb_*` modules) | 31,548 | 4,746 | 15.0% | 6,889,726 | 1,134,037 | 16.5% |
| Rock Band game modules (`rb_*`) | 19,627 | 3 | 0.0% | 6,164,746 | 2,948 | 0.0% |

The counts use the functions that IDA has defined. IDA has not defined every
routine: a further 3.64 MB of code in the game ranges belongs to no IDA
function (see [Methodology](#methodology)). Measured against all code bytes,
16.69 MB, the game code is **6.8%** reconstructed, and the engine alone is
12.6%.

IDA has an original mangled (`_Z`) name for 10.0% of the game-code functions,
and 16.1% of the engine functions.

Since `608eab2` the reconstructed set has grown by 1,908 functions and
446,129 bytes, from 5.6% to 9.3% of the game code by count and from 5.3% to
8.7% by bytes. Most of the growth is in `render` (+1,305 functions), `audio`
(+201) and `os` (+198):

| Commit | Reconstructed functions | By count | By bytes |
|---|---:|---:|---:|
| `c6fee06` | 1,819 | 3.6% | 3.5% |
| `d5c8ada` | 1,820 | 3.6% | 3.5% |
| `3be3757` | 2,256 | 4.4% | 4.2% |
| `c58714e` | 2,508 | 4.9% | 4.8% |
| `608eab2` | 2,841 | 5.6% | 5.3% |
| `006db74` | 3,103 | 6.1% | 5.6% |
| `a7b94d9` | 4,362 | 8.5% | 7.9% |
| `1fd3e74` | 4,749 | 9.3% | 8.7% |

Every row is measured against the current IDA export (54,030 functions). IDA
has gained 650 game-code functions since the previous report, so the
`3be3757` and `c58714e` rows are 0.1 points lower than they were in that
report. See [Changes to the method](#changes-to-the-method).

Uncommitted work in the tree, including the `src/ui` work in progress, is not
measured.

## Per system

A system is a module directory of the reference map (`audio`, `render`, …).
Its address ranges in this binary are listed under
[Binary layout](#binary-layout). The table's columns are:
- **Funcs** and **Recon**: the IDA functions in the system's ranges, and how
  many of them start at a reconstructed address.
- **Bytes** and **Recon bytes**: the same, in bytes.
- **Undefined**: the code bytes in the ranges that belong to no IDA function.
- **% of code**: the reconstructed bytes as a share of the function bytes plus
  the undefined bytes.
- **`_Z` named**: the share of functions that IDA names with a mangled
  original name.

| System | Funcs | Recon | % | Bytes | Recon bytes | % | Undefined | % of code | `_Z` named |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| `rockband` | 16 | 3 | 18.8% | 1,197 | 811 | 67.8% | 958 | 37.6% | 12.5% |
| `audio` | 4,100 | 1,268 | 30.9% | 810,326 | 256,024 | 31.6% | 228,283 | 24.7% | 31.5% |
| `mic` | 90 | 84 | 93.3% | 12,141 | 11,384 | 93.8% | 266 | 91.8% | 95.6% |
| `entity` | 4,216 | 294 | 7.0% | 1,199,616 | 66,173 | 5.5% | 236,861 | 4.6% | 6.9% |
| `math` | 241 | 116 | 48.1% | 69,736 | 24,038 | 34.5% | 19,989 | 26.8% | 48.5% |
| `utl` | 977 | 334 | 34.2% | 279,967 | 63,625 | 22.7% | 27,927 | 20.7% | 36.0% |
| `os` | 1,122 | 219 | 19.5% | 268,010 | 45,548 | 17.0% | 67,142 | 13.6% | 19.3% |
| `render` | 11,368 | 2,149 | 18.9% | 2,422,479 | 604,632 | 25.0% | 621,891 | 19.9% | 20.2% |
| `renderps4` | 477 | 255 | 53.5% | 90,178 | 52,072 | 57.7% | 316 | 57.5% | 85.5% |
| `anim` | 1,673 | 0 | 0.0% | 345,908 | 0 | 0.0% | 188,609 | 0.0% | 0.0% |
| `net` | 485 | 23 | 4.7% | 120,161 | 9,569 | 8.0% | 35,074 | 6.2% | 4.7% |
| `persistence` | 203 | 0 | 0.0% | 42,924 | 0 | 0.0% | 10,457 | 0.0% | 0.0% |
| `physics` | 359 | 1 | 0.3% | 83,997 | 161 | 0.2% | 35,178 | 0.1% | 0.3% |
| `stategraph` | 5,544 | 0 | 0.0% | 965,225 | 0 | 0.0% | 585,023 | 0.0% | 0.0% |
| `ui` | 677 | 0 | 0.0% | 177,861 | 0 | 0.0% | 47,783 | 0.0% | 0.4% |
| `rb_*` game modules | 19,627 | 3 | 0.0% | 6,164,746 | 2,948 | 0.0% | 1,534,279 | 0.0% | 0.0% |
| **Game code** | **51,175** | **4,749** | **9.3%** | **13,054,472** | **1,136,985** | **8.7%** | **3,640,036** | **6.8%** | **10.0%** |

The `net` and `physics` figures are not network or physics code. The 23
`net` functions are nine `os/Debug.o` functions just before the first `os`
boundary and the 14 functions of `ConsoleLineEditor`, which sit in the
second `net` block. The `physics` function is one `RndOptionsCom` function.
The `rb_*` count includes `MemProject` (`os`) and `UILayoutId` (an inferred
`ui` object), as well as `RBStagePresenceEnum`. See
[Limitations](#limitations).

Change by system since `608eab2`, on the same IDA export:

| System | Recon at `608eab2` | Recon at `1fd3e74` | Change | Recon bytes change |
|---|---:|---:|---:|---:|
| `audio` | 1,067 | 1,268 | +201 | +27,632 |
| `entity` | 174 | 294 | +120 | +26,003 |
| `math` | 112 | 116 | +4 | +1,001 |
| `utl` | 279 | 334 | +55 | +9,339 |
| `os` | 21 | 219 | +198 | +42,969 |
| `render` | 844 | 2,149 | +1,305 | +329,197 |
| `net` (`os` and `render` code) | 0 | 23 | +23 | +9,569 |
| `physics` (`render` code) | 0 | 1 | +1 | +161 |
| `rb_*` (`os` code) | 2 | 3 | +1 | +258 |
| `rockband`, `mic`, `renderps4` | 342 | 342 | 0 | 0 |
| **Game code** | **2,841** | **4,749** | **+1,908** | **+446,129** |

The seven `rb_*` modules (`rb_audio`, `rb_beatmatch`, `rb_game`,
`rb_gtrsolo`, `rb_meta`, `rb_track`, `rb_world`) are counted together. Too
few named functions fall inside them to place the boundaries between them.

Excluded third-party, SDK and import code:

| Code | Funcs | Bytes | Undefined | `_Z` named |
|---|---:|---:|---:|---:|
| PS4 SDK (Gnm, Gnmx, GpuAddress, …), Bink, Positive Grid BIAS | 1,088 | 404,947 | 111,248 | 13.6% |
| RakNet (two blocks) | 694 | 257,658 | 82,643 | 0.0% |
| libpng, zlib, kiss_fft | 260 | 119,216 | 21,601 | 0.0% |
| Ogg Vorbis, LibTomCrypt | 162 | 110,620 | 31,819 | 0.0% |
| PLT import stubs | 651 | 16,682 | 471 | 30.1% |

FMOD is not in this table. The reference map links FMOD statically, but this
executable loads `libfmod` and `libfmodstudio` as PRX modules and only has
PLT stubs for them.

## Build health

These figures come from `build/orbis/Debug` (SDK 5.500, Debug), written at
13:19:36 on 2026-10-18, 101 seconds before commit `1fd3e74`. The set of
translation units matches that commit's `src/*.cpp` files exactly.

| Metric | Value |
|---|---:|
| Translation units compiled (`objects.csv`) | 375 |
| Source files at `1fd3e74` (`.cpp` / `.h`) | 375 / 469, about 129,700 lines |
| Relocatable link (`rb4_reconstruction.o`) | produced, 24.7 MB |
| Unresolved symbols (`undefined-symbols.txt`) | 851 |
| Warnings | 0 with `-Wall`, from the build run's output; the script still keeps no log |

The 851 unresolved symbols fall into these groups:

| Group | Symbols |
|---|---:|
| PS4 SDK services (`sce*`, 166 of them `sce::Gnm`/`Gnmx` members) | 242 |
| FMOD C API (`FMOD_*`) | 116 |
| C and C++ runtime, `std::`, `eastl::` | 77 |
| Engine code not yet reconstructed | 416 |

The engine group grew from 264 at `608eab2`: the new code calls further into
classes that are still only declared. The 416 symbols are the work queue for
linking. 116 of them are free functions and globals:
- the `The*` singletons (`TheArchive`, `TheContentMgr`, `TheLocale`,
  `TheMovieMgr`, `ThePlatformMgr`, `TheAppChild`) and the `*Init` entries
  for `Archive`, `Joypad`, `MemTrack`, `ByteGrinder`, `StageKit` and `Net`;
- `Data*` and `File*` helpers from `utl` and `os`, and the property helpers
  (`_SetProp`, `_InsertProp`, `_StorePropState`, …);
- the `*_register_types` and per-frame `*_update` entries that the frame loop
  calls (several are descriptive names, see [frame-loop.md](frame-loop.md)),
  and the screenshot helpers;
- three weak thread-local initialization hooks (`_ZTH…`).

The other 300 are members of about 143 classes. The largest groups are:
- `PollMgr` (14 symbols), `Entity` (13), `VirtualInstrument` (11),
  `EntityResource` (6), `DynamicComBase` (6), `PollGroup` (5) and
  `GameObject` (4);
- the render components' draw paths: `RndParticleCom` (10),
  `RndLightSpotCom` (9), `RndLightProbeCom` (9), `RndLightPointCom` (7),
  `RndPixelDataCube` and `RndSceneResource` (4 each);
- `FusionPatchCom` (7), `AudioEmitterCom` (4), and the music generators
  (`Mogg*`, `MidiMusicGenerator`, `MusicTimelineGenerator`, three each).

`build/orbis/Debug/obj` also holds 312 stale objects from earlier source
layouts. Only the 375 listed in `objects.csv` are linked.

## Systems

### `rockband`

- **Done:** `Main.o` is complete: `App::Initialize`, `App::RunOneFrame` and
  `main` ([startup.md](startup.md), [frame-loop.md](frame-loop.md)). The other
  thirteen functions in the range are CRT and shim code.
- **Remaining:** none in the module itself. Many of the frame loop's callees
  still have descriptive names, because their owning classes are unknown;
  they make up most of the free functions in the unresolved list.

### `audio`

- **Done:** the core engine:
  - `SoundManager`, `AudioGenerator` and its managers, the bus, composite,
    dialog, Fusion, MIDI, Mogg and timeline generators, and the SynthRack and
    MultiFusion instrument generators (with the instrument generator vtable
    corrected, [instrument-generators.md](instrument-generators.md));
  - `FusionSampler`, `FusionVoice` and `FusionVoicePool`, `ADSR`, `LFO` and
    the modulators;
  - `AudioBus`, the mixer and render targets, `StreamReaderThread`,
    `AudioBuffer`, `WaveFile` and the RIFF chunks;
  - the DSP effects (filters, bit crusher, delay, distortion, `PitchDetector`,
    `SndAnalysis`, `SmbPitchShift`) and `AmpSimulator`;
  - `AudioEmitterCom` and `AudioListenerCom` as real components
    ([audio-components.md](audio-components.md)).

  The FMOD integration is done too: `FModSystem` and `FmodPlatform`, the
  bus, stream, buffered-stream, dialog and Studio generators, the bank and
  stream resources, `FmodFileWrapper`, and the gain and pitch-shift plug-ins.
  42 of the map's 115 audio objects have a source file (60% of the map's
  audio bytes).
- **Remaining:**
  - map objects with no source yet: `MidiMsgBroadcasterCom`, `VorbisReader`,
    `MoggResource`, `ByteGrinder`, `StandardStream`, `MidiPlayCursor` and
    `MidiPlayCursorMgr`, `MidiReader` and `PcmAudioData`;
  - declared but not reconstructed: `VirtualInstrument` (11 symbols),
    `FusionPatchCom`, the audio resource and clip component `Init`s, the
    emitter's dialog playback, the remaining FMOD DSP plug-ins (delay,
    filter, wah, vibe, stutter, tremolo, bit crusher, signal tap, analysis),
    the `AmpModel*` classes, and the PCM and Mogg decoders.
- **Blockers:** linking needs the FMOD 1.10.04 libraries for the 116 FMOD
  imports.

### `mic`

- **Done:** almost complete. `Mic`, `MicHwManager`, `MicReaderThread`,
  `MicHwManager_FMOD` and `Mic_FMOD` are reconstructed
  ([mic-fmod.md](mic-fmod.md)). Unchanged since `608eab2`.
- **Remaining:** `MicClientMapper` and `MicNull` from the map. Neither may be
  linked into this build.
- **Note:** this build places the `mic` objects inside the audio ranges. Its
  ranges are the spans of its known functions.

### `entity`

- **Done:** the resource, component and lifecycle core
  ([entity-resources.md](entity-resources.md),
  [entity-components.md](entity-components.md),
  [messages-and-polls.md](messages-and-polls.md)):
  - `Resource`, `ResourceMetaData`, `ResourcePath` and the cache paths;
  - `EntityResource` and `TransEntityResource`, with their 33-slot vtables,
    and `EntityResource`'s `Load`, `_LoadEntity` and `Save`;
  - `MsgSource`, `PollDepBase` and the load-progress listeners, and the
    entity's constructor, `Enter`, `Exit`, `_Poll` and `_Destroy`, its copy
    and imprint, and the load pass through `AreResourcesReady`;
  - `GameObject`'s component creation, sorting and destruction;
  - `Component`, with its 41-slot vtable, `ComMetaData`, `DynamicCom`,
    `EntityHeaderCom` and `InstanceCom`;
  - `PropRegistry`, `PropInfo`, `PropArray` and `PropRef`, parts of
    `TransCom`, and the waveform model (`Waveform.h`).

  16 of the map's 33 entity objects have a source file (54% of the map's
  entity bytes). The entity-lifecycle milestone of [status.md](status.md) is
  closed.
- **Remaining:**
  - `PropMetadata` (215 KB in the map, the largest), `ScriptCom`,
    `EditorRootCom`, `Waveform.o`'s code, `EntityPoolCom` and `PropPath`;
  - `Entity`'s cached and uncached load and save, `SetActive`, `Handle` and
    the immediate enter and poll paths;
  - `EntityResource`'s inline resources and stale-id fixups, and
    `DynamicComBase`'s storage and dynamic properties;
  - `ComMetaData::Init` (`0xE5B80`) and the component ordering
    (`GameObject::_BuildComponentOrder`, `0x117A90`).
- **Known issues:** several `EntityResource`, `Component` and poll slot names
  rest on their bodies and callers only (see those documents' "Weak evidence"
  sections).

### `math`

- **Done:**
  - `Color`, `Interp`, `Matrix3`, `Matrix4`, `Rand` and `Rand2`, `Rot`,
    `Transform`, `Trig` and `Half`;
  - the vector types, `DoubleExponentialSmoother` and the easing functions;
  - the geometry objects `Capsule`, `Frustum`, `Geo`, `Plane` and
    `TruncatedRoundedCone`.
- **Remaining:** `trimesh`, `ConvexHull`, `ForceObj`, `SHA1`, `MD5`,
  `Sampler` and `Sphere` (whose `Multiply` helpers are now referenced). Also
  the two 36-byte objects built by the static initializer at `0x2195C0`
  ([math-foundation.md](math-foundation.md)).

### `utl`

- **Done:** `BinStream`, `FileStream`, `TextStream`, `Str`, `MakeString`,
  `UTF8`, `Thread`, `Thread_PS4`, `ThreadCall`, `Timer`, `MsgSink`,
  `StringTable`, `PollDep`, the licence text, and the core of `DataArray`,
  `DataNode` and `DataUtl` ([core-io.md](core-io.md),
  [foundation-runtime.md](foundation-runtime.md)). 22 of the map's 54 objects
  have a source file.
- **Remaining:**
  - `DataFunc` (90 KB), `DataFile`, `MtJobMgr`, `Symbol`, `GlitchFinder`,
    `DateTime`, `PngWrite`, `DataFlex` and `CsvReader`;
  - `PollMgr` and `PollGroup` (19 unresolved symbols, the largest class
    group), `DataVarIndex` and `DataVariable`, and the `Data*` reader and
    function-registry entries.

### `os`

- **Done:** the runtime core ([os-runtime.md](os-runtime.md)):
  - `MemMgr`, `MemHeap`, the pool allocators, `Mem_PS4` and `MemProject`;
  - `Debug`, `GlitchBreaker`, `System` and `System_PS4`, `Core`,
    `CallOnMainThread`;
  - the file layer (`File`, `FileUtl_PS4`), the keyboard source
    (`Keyboard_PS4`), the cheats, and the earlier special pad reader, thread
    affinity and `CritSec`.

  19 of the map's 97 `os` objects have a source file (26% of the map's `os`
  bytes, up from 16%).
- **Remaining:**
  - `HolmesClient`, `Archive`, `MemTracker`, `Joypad`, `PlatformMgr_PS4`,
    `ContentMgr`, `NetCacheMgr`, `UsbInstrument_PS4`, `Locale` and `HDCache`;
  - `AppChild`, `ArkFile`, `AsyncFile`, `MovieMgr`, `PlatformMgr`'s
    virtuals, `MapFile` and `MemTrack`, which are declared with their
    addresses only. Their singletons and `*Init` entries are a large share of
    the free functions in the link work queue.

### `render`

- **Done:** the device and frame infrastructure and, since `608eab2`, the
  scene pipeline:
  - `RndDevice`, `RndContext`, `RndWindow`, `RndBufferCollection`,
    `RndCapabilities` and `RndDefaults`;
  - the shader system: `RndShader`, the backend loader and cache,
    `RndShaderMgr`, all 35 built-in shaders, and every caller of
    `RndShader::_SelectShaderCollection` but one, including the blur draw
    (`0x634BB0`) and the bloom pass (`0x6305A0`)
    ([render-builtin-shaders.md](render-builtin-shaders.md));
  - `RndShaderGraph`, its resource and node base, and the time and
    transform-info nodes;
  - the scene component and resource, `RndSceneDrawer` with its passes and
    job graph, the basic culler and billboards
    ([render-scene.md](render-scene.md),
    [render-scene-drawer.md](render-scene-drawer.md));
  - the camera, light (directional, point, spot, probe, environment), light
    manager and light flare components ([render-lights.md](render-lights.md));
  - `RndMaterialCom`, `RndMeshCom`, their runtime data resources, the draw
    node, drawable and entity-instance components, and the particle
    component, collection and system manager;
  - the atmosphere, fog and sky, CMAA, SSAO, volumetric and post-processing
    chain components, and the options components;
  - textures and pixel data, meshes and mesh builders, compute and particle
    buffers, the occlusion queries and their manager, the texture renderer
    manager;
  - the debug overlays, the console and its line editor, fonts and
    `RndTypesetter`, and the GPU statistics.

  163 of the map's 321 render objects have a source file (54% of the map's
  render bytes, up from 107 objects and 25%).
- **Remaining:**
  - map objects with no source yet: `RndMaterial` (125 KB), `RndTextCom`,
    `RndSplineCom`, `RndTexture2DPropertiesCom`, `RndFontImportCom`,
    `RndTextureUtilityCom`, and most shader graph nodes (the waveform,
    intrinsic and root nodes are the largest);
  - declared draw paths of the new components: `RndParticleCom`'s update
    steps, the spot, point and probe lights' shadow and deferred draws,
    `RndSceneResource` and `RndPixelDataCube`;
  - `RndShaderGraph::Select` (`0x4FE630`), the last
    `_SelectShaderCollection` caller, which needs the graph's resource
    binding (`0x4FE850`).
- **Known issues:**
  - `RndDefaults`' `TextureFamily` layout looks transposed
    ([render-lighting-shaders.md](render-lighting-shaders.md));
  - font kerning pairs are sorted with `std::sort` where the binary uses
    EASTL's sort;
  - several component property metadata blocks and shared-header constants
    are not modelled, and the new component documents list weak-evidence
    slot names.

### `renderps4`

- **Done:** 23 of the map's 24 objects have source: `PS4Device`,
  `PS4Context`, `PS4Factory`, `PS4Window`, every texture, mesh, buffer,
  query and shader-program class, and video output. Only `PS4Common` (empty
  in the map) is missing. Unchanged since `608eab2`.
- **Remaining:** 222 functions in the range, mostly small helpers and
  inlined template copies inside those objects.

### `anim`, `net`, `persistence`, `physics`, `stategraph`, `ui`

None of these has been started. The few reconstructed functions in the
`net` and `physics` ranges belong to `os` and `render` (see
[Per system](#per-system)). In `ui`, only `UIMgr`, `RBUILayout` and the
layout ids are declared, for the frame loop. The largest objects are:

| System | Largest map objects |
|---|---|
| `anim` | `AnimInit` (626 KB), the `PropKeys*Com` classes, `PropertyBlenderCom`, `CharIkBoneInterpCom` |
| `net` | `jsoncpp` (105 KB), `DingoSvr`, `DataPointMgr`, `HttpGet`, `WebSvcReq` |
| `persistence` | `SaveLoadMgr`, `StorageRef_PS4`, `ProfileMgr` |
| `physics` | `PhysClothCom` (67 KB), `PhysOptionsCom`, the collidables |
| `stategraph` | `StateConditionComparisonInit_A`/`_B`, `StateConditionEventParameter`, `StateNodeSetPropertyInit_*`; mostly template expansions |
| `ui` | `UIListCom` (93 KB), `UIInputCom`, `UIInputReceiverCom`, `UI`, `UILayoutResource`, `UIMgr` |

### `rb_*` game modules

- **Done:** `RBStagePresenceEnum` and isolated `RBMetaStateCom` and
  `RBProfileMgr` entry points used by startup.
- **Remaining:** everything else, about 7.7 MB of code. The largest objects
  are:
  - `RBCampaignMgr`, `RBGameData`, `RBGigVoteCom` and `RBGigCom`
    (`rb_game`);
  - `RBCharOutfitCom` and `RBCameraShotMgrCom` (`rb_world`);
  - `RBMetaStateCom` and `RBOvershellSlotCom` (`rb_meta`);
  - `RBMidiGameData` (`rb_beatmatch`) and `RBVenueAuthoringCom`
    (`rb_audio`).

## Project milestones

The entity-lifecycle milestone was closed by `006db74`. One milestone in
[status.md](status.md) is still open:

1. Link a complete reconstructed executable. This needs the 416 engine
   symbols above (or adapters for them) and the matching FMOD libraries. The
   SDK 5.500 compiler is newer than the game's, so the linked code will not
   be byte-identical.

## Methodology

### Reconstructed set

Every reconstructed definition carries a comment of the form
`// Reconstructed from eboot.elf at 0xADDR.`. Some comments list several
addresses, and some wrap onto the next comment line. For each source file at
the commit (`git show <rev>:<path>`), every hex address in the comment's
clause counts, up to the end of the sentence. At `1fd3e74` that gives 4,797
comments and 4,776 distinct addresses.

A function counts as reconstructed when its IDA start address is in this
set. Of the 4,776 addresses:
- 4,749 are IDA function starts;
- seven lie inside an IDA function or one of its chunks;
- fourteen are in code that IDA has not made a function;
- three are bytes that IDA has not defined (`0x40AC0`, `0x1D50E0`,
  `0x3A2180`);
- three are data (`0x68CB1B`, `0x192EFC8`, `0x1934360`).

As a cross-check, the compiled objects listed in `objects.csv` were mapped
back to their comments with the symbol-to-address helper that reads
`orbis-objdump -d -l` line information, reading the sources at the commit.
It found 4,729 compiled functions at 4,646 addresses and no address missing
from the comment set.

### Binary totals

All 54,030 IDA functions were exported with their start, end, name and size.
The size sums every chunk, including tails. Code that IDA has not placed in
any function was measured separately as "undefined":
- the code bytes between functions, with nop and `int3` padding left out;
- 3.64 MB in the game ranges, in 36,317 runs.

These runs contain 21,994 `push rbp; mov rbp, rsp` prologues, against 21,848
among the defined game functions. IDA therefore still misses roughly as many
frame-pointer functions as it has. The executable probably holds about
73,000-75,000 game functions, not the 51,175 that IDA has defined. Some runs
are not separate functions but the tails of functions that IDA cut short. The
map's older build lists 67,527 game function symbols and 14.9 MB, which
agrees with the larger estimate.

### Changes to the method

- **IDA export refreshed.** IDA's function list changed since `608eab2`
  (53,380 to 54,030 functions, all 650 new ones in the game ranges, and many
  more `_Z` names applied). Steps 1 and 2 below were rerun, and every row of
  the trend table, including the earlier commits, is measured against the
  new export. This moves `3be3757` and `c58714e` down by 0.1 points; the
  `608eab2` row is unchanged (2,841 functions, 5.6% and 5.3%). The
  `_Z`-named share rose from 6.6% to 10.0% mainly because of the renames.
- **Undefined code.** The undefined runs were recomputed from the gaps
  between all function chunks. On the old export the same walk had found
  3.73 MB; the new export's 3.64 MB reflects the code IDA now covers. The
  PLT now shows 471 undefined bytes that the earlier walk did not report.
- **Cross-check.** The helper now reads only the objects in `objects.csv`
  (not the stale ones) and reads the sources at the commit instead of the
  working tree.
- **Ranges.** The system ranges are unchanged, so the numbers stay
  comparable. The new reconstruction addresses suggest two refinements,
  described under [Limitations](#limitations); they were not applied.

### Binary layout

The map is older than the executable, and its link order is different. It
lists the modules alphabetically, while this build groups them as below and
places a second, smaller block of engine and game code after the SDK
libraries. The ranges were placed for the `608eab2` report from three kinds
of anchors:
- 2,220 IDA or compiled names that match a map symbol, giving the map object;
- the 2,858 reconstruction addresses of that commit, giving their source
  module;
- 1,136 functions that reference a class-name string listed in the map.

Strings sampled across the gaps then confirmed or refined each boundary.
Each system runs from its start address to the next row:

| Start | System | Start | System |
|---:|---|---:|---|
| `0x0` | `rockband` | `0x8EEEF0` | RakNet |
| `0x9D0` | `audio` | `0x8F2700` | `rb_*` |
| `0xE56C0` | `entity` | `0x1090600` | `audio` (`AmpSimulator`) |
| `0x211A00` | `math` | `0x1090A00` | Positive Grid, Bink, PS4 SDK |
| `0x219C30` | `utl` | `0x111D000` | `audio` (second block) |
| `0x260000` | `audio` (FMOD integration) | `0x11295F0` | `entity` |
| `0x283000` | Ogg Vorbis, LibTomCrypt | `0x1170000` | `math` |
| `0x2A7080` | `anim` | `0x117F000` | `utl` |
| `0x33A600` | `net` | `0x1186E00` | libpng, zlib, kiss_fft |
| `0x35C200` | `os` | `0x11A9900` | `net` |
| `0x3AB600` | `persistence` | `0x11B1DE0` | `persistence` |
| `0x3B9000` | `physics` | `0x11B2CD0` | `render` |
| `0x3D8170` | `render` | `0x11B3100` | RakNet |
| `0x6F6000` | `stategraph` | `0x1209300` | `rb_*` |
| `0x894000` | `ui` | `0x1243900` | PLT stubs |
| `0x8CEF00` | `os` (PS4 joypads) | | |
| `0x8D5DE0` | `renderps4` | | |
| `0x8ECA00` | `os` (PS4 threads) | | |

The `mic` ranges (`0xA1C30`, `0xE1530`, `0x275490` and `0x27B3E0`) sit
inside `audio`. Each one runs to the end of the last known `mic` function in
it.

### Limitations

- **Approximate boundaries.** They fall between the nearest anchors and may
  be off by a few kilobytes. Some objects of the map's modules sit in another
  module's range in this build: `PerfMgr` (`os`) inside `utl`, and an `anim`
  resource inside `entity`. The `entity` range (1.4 MB) is much larger than
  the map's `entity` module (0.8 MB), and `anim` is smaller, so part of the
  animation code is probably counted under `entity`. The game total is not
  affected.
- **Misplaced anchors found since `608eab2`.** These were left in place to
  keep the numbers comparable:
  - `os/Debug.o` starts at about `0x35BA60`, before the `os` boundary
    (`0x35C200`), so nine of its functions count under `net`;
  - `ConsoleLineEditor` (`0x11AECB0`-`0x11B0D20`, an inferred name for code
    the map does not contain) lies in the second `net` block and counts
    there, although it is render code;
  - `MemProject` (`os`, `0x92AA50`) and `UILayoutId` (`0xBB06A0`) lie in the
    `rb_*` range, and one `RndOptionsCom` function (`0x3BF4C0`) lies in
    `physics`.
- **Inlined code.** A reconstructed function whose callees the compiler
  inlined counts once, at its own address. Code reconstructed only as an
  inlined helper adds nothing.
- **IDA boundaries.** Sizes come from IDA's boundaries. A reconstructed
  function that IDA cut short is undercounted, and functions that IDA has
  not defined cannot count as reconstructed, even when they are (fourteen
  such addresses at `1fd3e74`).
- **What "reconstructed" means.** Every addressed function counts in full,
  whatever its fidelity. Weak-evidence names and unmodelled vtable slots are
  noted per system but not weighed.
- **Third-party code.** SDK, middleware and import code is excluded: PS4
  SDK, Bink, Positive Grid, RakNet, libpng, zlib, kiss_fft, Ogg Vorbis,
  LibTomCrypt, and the PLT. EASTL and libc++ are inlined into the game code
  and counted with it.

### Regenerating

1. Export the functions from the IDA database (read-only): for each function
   in `idautils.Functions()`, its start, `end_ea`, the summed size of
   `idautils.Chunks`, and its name.
2. Export the undefined code: collect every function's chunks, walk the
   gaps between them, keep the code heads, drop nop and `int3` padding, and
   count `55 48 89 E5` prologues.
3. Collect the reconstruction addresses from `src/` at the commit as
   described above.
4. Assign every function and undefined run to a system with the layout table,
   and match the reconstructed addresses against the function starts.

The scripts used for this report are not committed. The layout table above
is the only input that is not derived mechanically. Rerun steps 3 and 4 after
each milestone. Rerun steps 1 and 2 only after IDA's functions change, and
then remeasure the earlier commits of the trend table too.
