# Reconstruction progress

This report measures how much of the executable's own code has been
reconstructed. It is a snapshot of commit `5038476` ("decomp: finish the anim
property keys and start the rb_meta module", 2026-10-19 15:48 -0600). The
build outputs it quotes are from the Debug object build of that tree, written
at 15:44:59 the same day. Third-party and SDK code is reported separately and
left out of the totals.

## Summary

| Scope | Functions | Reconstructed | By count | Bytes | Reconstructed bytes | By bytes |
|---|---:|---:|---:|---:|---:|---:|
| Game code (engine and Rock Band) | 52,059 | 7,875 | **15.1%** | 13,219,506 | 2,329,258 | **17.6%** |
| Engine only (all but the `rb_*` modules) | 32,399 | 7,704 | 23.8% | 7,047,011 | 2,261,243 | 32.1% |
| Rock Band game modules (`rb_*`) | 19,660 | 171 | 0.9% | 6,172,495 | 68,015 | 1.1% |

The counts use the functions that IDA has defined. IDA has not defined every
routine: a further 3.50 MB of code in the game ranges belongs to no IDA
function (see [Methodology](#methodology)). Measured against all code bytes,
16.72 MB, the game code is **13.9%** reconstructed, and the engine alone is
25.1%.

IDA has an original mangled (`_Z`) name for 15.2% of the game-code functions,
and 23.9% of the engine functions.

Since `1fd3e74` the reconstructed set has grown by 3,126 functions and
1,192,273 bytes, from 9.1% to 15.1% of the game code by count and from 8.6%
to 17.6% by bytes. Bytes grew faster than the count because the new modules
have larger functions. Most of the growth is in four modules that had not
been started: `stategraph` (+1,009 functions), `anim` (+817), `ui` (+280)
and `persistence` (+124). `render` (+313) and `entity` (+282) also grew, and
the `rb_*` modules have their first real code (`rb_meta`, +168):

| Commit | Reconstructed functions | By count | By bytes |
|---|---:|---:|---:|
| `c6fee06` | 1,819 | 3.5% | 3.4% |
| `d5c8ada` | 1,820 | 3.5% | 3.4% |
| `3be3757` | 2,256 | 4.3% | 4.2% |
| `c58714e` | 2,508 | 4.8% | 4.7% |
| `608eab2` | 2,841 | 5.5% | 5.2% |
| `006db74` | 3,103 | 6.0% | 5.6% |
| `a7b94d9` | 4,362 | 8.4% | 7.8% |
| `1fd3e74` | 4,749 | 9.1% | 8.6% |
| `db1b376` (docs only) | 4,749 | 9.1% | 8.6% |
| `048b965` | 4,838 | 9.3% | 8.8% |
| `a8d3c3c` | 5,028 | 9.7% | 9.6% |
| `d365572` | 6,110 | 11.7% | 13.5% |
| `7f92221` | 6,896 | 13.2% | 15.4% |
| `55986dd` | 7,491 | 14.4% | 16.7% |
| `5d12080` (build tools only) | 7,491 | 14.4% | 16.7% |
| `5038476` | 7,875 | 15.1% | 17.6% |

Every row is measured against the current IDA export (54,914 functions). IDA
has gained 884 game-code functions since the previous report, so every
earlier row is 0.1 to 0.2 points lower than it was in that report: `1fd3e74`
was 9.3% and 8.7% there. See [Changes to the method](#changes-to-the-method).

Uncommitted work in the tree, including the `src/rb_meta` and `src/rb_game`
work in progress, is not measured.

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
| `audio` | 4,106 | 1,326 | 32.3% | 811,189 | 330,556 | 40.7% | 226,795 | 31.8% | 32.7% |
| `mic` | 90 | 84 | 93.3% | 12,141 | 11,384 | 93.8% | 266 | 91.8% | 95.6% |
| `entity` | 4,261 | 576 | 13.5% | 1,209,338 | 209,428 | 17.3% | 227,916 | 14.6% | 10.3% |
| `math` | 242 | 119 | 49.2% | 69,844 | 24,338 | 34.8% | 19,898 | 27.1% | 49.6% |
| `utl` | 989 | 400 | 40.4% | 281,991 | 80,567 | 28.6% | 26,303 | 26.1% | 42.2% |
| `os` | 1,122 | 219 | 19.5% | 268,010 | 45,548 | 17.0% | 67,082 | 13.6% | 19.3% |
| `render` | 11,374 | 2,462 | 21.6% | 2,426,304 | 850,739 | 35.1% | 618,016 | 27.9% | 22.8% |
| `renderps4` | 477 | 255 | 53.5% | 90,178 | 52,072 | 57.7% | 316 | 57.5% | 85.5% |
| `anim` | 1,938 | 817 | 42.2% | 391,210 | 225,227 | 57.6% | 155,065 | 41.2% | 40.1% |
| `net` | 487 | 29 | 6.0% | 120,205 | 9,848 | 8.2% | 35,010 | 6.3% | 6.0% |
| `persistence` | 253 | 124 | 49.0% | 51,700 | 31,246 | 60.4% | 4,473 | 55.6% | 48.2% |
| `physics` | 359 | 1 | 0.3% | 83,997 | 161 | 0.2% | 35,150 | 0.1% | 0.3% |
| `stategraph` | 5,911 | 1,009 | 17.1% | 1,027,240 | 285,115 | 27.8% | 528,537 | 18.3% | 15.7% |
| `ui` | 774 | 280 | 36.2% | 202,467 | 104,203 | 51.5% | 26,074 | 45.6% | 35.8% |
| `rb_*` game modules | 19,660 | 171 | 0.9% | 6,172,495 | 68,015 | 1.1% | 1,525,787 | 0.9% | 0.9% |
| **Game code** | **52,059** | **7,875** | **15.1%** | **13,219,506** | **2,329,258** | **17.6%** | **3,497,646** | **13.9%** | **15.2%** |

The `net` and `physics` figures are not network or physics code. The 29
`net` functions are nine `os/Debug.o` functions just before the first `os`
boundary, the 14 functions of `ConsoleLineEditor` and six `persistence`
`Profile` functions, which sit in the second `net` block. The `physics`
function is one `RndOptionsCom` function. The `stategraph` count includes
eleven `ui` functions (`UI`, `UIInputCom`, the lists and widgets) just before
the `ui` boundary. The `rb_*` count includes `MemProject` (`os`). See
[Limitations](#limitations).

Change by system since `1fd3e74`, on the same IDA export:

| System | Recon at `1fd3e74` | Recon at `5038476` | Change | Recon bytes change |
|---|---:|---:|---:|---:|
| `stategraph` (with 11 `ui` functions) | 0 | 1,009 | +1,009 | +285,115 |
| `anim` | 0 | 817 | +817 | +225,227 |
| `render` | 2,149 | 2,462 | +313 | +246,107 |
| `entity` | 294 | 576 | +282 | +143,255 |
| `ui` | 0 | 280 | +280 | +104,203 |
| `rb_*` (`rb_meta`) | 3 | 171 | +168 | +65,067 |
| `persistence` | 0 | 124 | +124 | +31,246 |
| `utl` | 334 | 400 | +66 | +16,942 |
| `audio` | 1,268 | 1,326 | +58 | +74,532 |
| `net` (`persistence` code) | 23 | 29 | +6 | +279 |
| `math` | 116 | 119 | +3 | +300 |
| `rockband`, `mic`, `os`, `renderps4`, `physics` | 562 | 562 | 0 | 0 |
| **Game code** | **4,749** | **7,875** | **+3,126** | **+1,192,273** |

The seven `rb_*` modules (`rb_audio`, `rb_beatmatch`, `rb_game`,
`rb_gtrsolo`, `rb_meta`, `rb_track`, `rb_world`) are still counted together.
The new `rb_meta` addresses fall between `0xADAF90` and `0xD685B0`, but too
few objects of the other modules are placed yet to set the boundaries between
them. By source file, the 171 functions are 169 of `rb_meta` (67,264 bytes),
`RBStagePresenceEnum` (`rb_game`) and `MemProject`.

Map objects with a source file at `5038476`, by name, against the map's
`.text` objects (the share of the module's map bytes is in parentheses):

| Module | At `1fd3e74` | At `5038476` |
|---|---:|---:|
| `anim` | 0 of 45 | 41 of 45 (97%) |
| `stategraph` | 0 of 32 | 32 of 32 (100%) |
| `persistence` | 0 of 9 | 9 of 9 (100%) |
| `ui` | 0 of 16 | 14 of 16 (98%) |
| `entity` | 16 of 33 (54%) | 20 of 33 (86%) |
| `render` | 163 of 321 (54%) | 165 of 321 (54%) |
| `rb_meta` | 0 of 136 | 4 of 136 (7%) |
| `rb_game` | 1 of 105 (1%) | 1 of 105 (1%) |

A source file does not mean a complete object: `stategraph` has every object
but 27.8% of its range's bytes. 64% of the range's unreconstructed bytes are
in functions whose size recurs three or more times, which look like template
copies (the property-setting nodes and comparison conditions) that the source
builds from one addressed template and that therefore do not count. The same
pattern makes up about half of `anim`'s and `persistence`'s remainder.

Excluded third-party, SDK and import code:

| Code | Funcs | Bytes | Undefined | `_Z` named |
|---|---:|---:|---:|---:|
| PS4 SDK (Gnm, Gnmx, GpuAddress, …), Bink, Positive Grid BIAS | 1,088 | 404,947 | 109,690 | 13.6% |
| RakNet (two blocks) | 694 | 257,658 | 82,401 | 0.0% |
| libpng, zlib, kiss_fft | 260 | 119,216 | 21,547 | 0.0% |
| Ogg Vorbis, LibTomCrypt | 162 | 110,620 | 31,671 | 0.0% |
| PLT import stubs | 651 | 16,682 | 471 | 30.1% |

FMOD is not in this table. The reference map links FMOD statically, but this
executable loads `libfmod` and `libfmodstudio` as PRX modules and only has
PLT stubs for them.

## Build health

These figures come from `build/orbis/Debug` (SDK 5.500, Debug), written at
15:44:59 on 2026-10-19, about three and a half minutes before commit
`5038476`. The set of translation units matches that commit's `src/*.cpp`
files exactly. Later builds by work in progress were not used.

| Metric | Value |
|---|---:|
| Translation units compiled (`objects.csv`) | 501 |
| Source files at `5038476` (`.cpp` / `.h`) | 501 / 620, about 209,200 lines |
| Relocatable link (`rb4_reconstruction.o`) | produced, 131.7 MB |
| Unresolved symbols (`undefined-symbols.txt`) | 1,106 |
| Warnings | 0 with `-Wall`, from the build run's output; the script still keeps no log |

The 1,106 unresolved symbols fall into these groups:

| Group | Symbols |
|---|---:|
| PS4 SDK services (`sce*`, 162 of them `sce::Gnm`/`Gnmx` members) | 248 |
| FMOD C API (`FMOD_*`) | 116 |
| C and C++ runtime, `std::`, `eastl::` | 77 |
| Engine and game code not yet reconstructed | 665 |

The engine group grew from 416 at `1fd3e74`: the new modules call further
into classes that are still only declared, and `rb_meta` reaches into the
game's managers. The 665 symbols are the work queue for linking. 188 of them
are free functions and globals:
- the `The*` and `the*` singletons: `TheArchive`, `TheContentMgr`,
  `TheLocale`, `TheMovieMgr`, `ThePlatformMgr`, `TheAppChild`,
  `TheGameMicManager`, and the game's `theBandUserMgr`, `theOvershell`,
  `theRBClanMgr`, `theRBProfileMgr`, `theRBSompSessionMgr` and `theSongMgr`;
- the `*Init` entries for `Archive`, `Joypad`, `MemTrack`, `ByteGrinder`,
  `StageKit`, `Net` and `Data`;
- the property-metadata helpers (`RemapPropMetadata`, 20 overloads, and
  `TypeSpecificMetadata`, 12), the property accessors (`_SetProp`,
  `_GetProp`, `_InsertProp`, `_StorePropState`, …) and the `Data*` and
  `File*` helpers from `utl` and `os`;
- the `*_register_types` and per-frame `*_update` entries that the frame loop
  calls (several are descriptive names, see [frame-loop.md](frame-loop.md)),
  and the screenshot helpers;
- about twenty `g*` globals and three weak thread-local initialization hooks
  (`_ZTH…`).

The other 477 are members of about 185 classes. The largest groups are:
- the property metadata tags, `TagBase<T>` (30 symbols, `_EvalScript` and
  friends for each value type);
- `Entity` (15), `WaveformResource` (12), `DataNode` (12), `PropRef` (9),
  `EntityResource` (7), `DynamicComBase` (6) and `BinStream` (6);
- the `ui` components' remaining paths: `UIListCom` (11), `UIInputCom` (9),
  `UIComboBoxBaseCom<T>` (9), `UIInputReceiverCom` (6) and the enum and
  file combo boxes (3 each), 48 `UI*` symbols in all;
- the render components' draw paths: `RndParticleCom` (10),
  `RndLightSpotCom` (10), `RndMeshResource` (10), `RndLightProbeCom` (9),
  `RndMeshUtl` (8), `RndLightPointCom` (7) and `RndSkeletonPoseCom` (6);
- `VirtualInstrument` (11), the `TKeyTrack` spline samplings (7) and the
  music generators (`Mogg*`, `MidiMusicGenerator`, `MusicTimelineGenerator`,
  three each).

`build/orbis/Debug/obj` also holds 313 stale objects from earlier source
layouts. Only the 501 listed in `objects.csv` are linked.

## Systems

### `rockband`

- **Done:** `Main.o` is complete: `App::Initialize`, `App::RunOneFrame` and
  `main` ([startup.md](startup.md), [frame-loop.md](frame-loop.md)). The other
  thirteen functions in the range are CRT and shim code. `Anim::Init` and
  `UI::Init` now replace their startup placeholders.
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
    ([audio-components.md](audio-components.md)), and since `1fd3e74`
    `FusionPatchCom`'s registration and most of its body.

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
    the rest of `FusionPatchCom`, `AudioClipCom`, the emitter's dialog
    playback, the remaining FMOD DSP plug-ins (delay, filter, wah, vibe,
    stutter, tremolo, bit crusher, signal tap, analysis), the `AmpModel*`
    classes, and the PCM and Mogg decoders.
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
  - `Component`, with its 41-slot vtable, `ComMetaData` and
    `ComMetaData::Init`, `DynamicCom`, `EntityHeaderCom` and `InstanceCom`;
  - `PropRegistry`, `PropInfo`, `PropArray`, `PropRef` and `PropPath`, more
    of `TransCom`, and the waveform model;
  - since `1fd3e74`, `PropMetadata` (273 addresses): the metadata tags, the
    typed metadata classes, creation, display names, dumps and value checks,
    with the vector, euler and transform types, and the class registration
    helpers.

  20 of the map's 33 entity objects have a source file (86% of the map's
  entity bytes, up from 54%).
- **Remaining:**
  - `ScriptCom`, `EditorRootCom`, `Utl`, `EntityPoolCom`, `DataResource`,
    `PollSort` and `LookAtCom`;
  - the metadata tags' script evaluation (`TagBase<T>`, 30 symbols) and the
    `RemapPropMetadata` and `TypeSpecificMetadata` overloads;
  - `Entity`'s cached and uncached load and save, `SetActive`, `Handle` and
    the immediate enter and poll paths (15 symbols);
  - `EntityResource`'s inline resources and stale-id fixups, and
    `DynamicComBase`'s storage and dynamic properties;
  - `WaveformResource` (12 symbols) and the component ordering
    (`GameObject::_BuildComponentOrder`, `0x117A90`).
- **Known issues:** several `EntityResource`, `Component` and poll slot names
  rest on their bodies and callers only (see those documents' "Weak evidence"
  sections).

### `math`

- **Done:**
  - `Color`, `Interp`, `Matrix3`, `Matrix4`, `Rand` and `Rand2`, `Rot`,
    `Transform`, `Trig` and `Half`;
  - the vector types, `DoubleExponentialSmoother` and the easing functions
    (with the curves the anim key tracks use);
  - the geometry objects `Capsule`, `Frustum`, `Geo`, `Plane` and
    `TruncatedRoundedCone`.
- **Remaining:** `trimesh`, `ConvexHull`, `ForceObj`, `SHA1`, `MD5`,
  `Sampler` and `Sphere` (whose `Multiply` helpers are referenced), the quat
  and euler helpers the anim code calls (`FastInterp`, `MakeEuler`,
  `Normalize`, `InterpEuler`). Also the two 36-byte objects built by the
  static initializer at `0x2195C0` ([math-foundation.md](math-foundation.md)).

### `utl`

- **Done:** `BinStream`, `FileStream`, `TextStream`, `Str`, `MakeString`,
  `UTF8`, `Thread`, `Thread_PS4`, `ThreadCall`, `Timer`, `MsgSink`,
  `StringTable`, `PollDep`, the licence text, and the core of `DataArray`,
  `DataNode` and `DataUtl` ([core-io.md](core-io.md),
  [foundation-runtime.md](foundation-runtime.md)). Since `1fd3e74` the whole
  poll manager: `PollMgr` and `PollGroup` (65 addresses), swapped to the
  binary's meaning, and the timelines clock sync. 22 of the map's 54 objects
  have a source file (54% of the map's bytes).
- **Remaining:**
  - `DataFunc` (90 KB), `DataFile`, `MtJobMgr`, `Symbol`, `GlitchFinder`,
    `DateTime`, `PngWrite`, `DataFlex` and `CsvReader`;
  - `DataVarIndex` and `DataVariable`, the remaining `DataNode` conversions
    (12 symbols), and the `Data*` reader and function-registry entries.

### `os`

- **Done:** the runtime core ([os-runtime.md](os-runtime.md)):
  - `MemMgr`, `MemHeap`, the pool allocators, `Mem_PS4` and `MemProject`;
  - `Debug`, `GlitchBreaker`, `System` and `System_PS4`, `Core`,
    `CallOnMainThread`;
  - the file layer (`File`, `FileUtl_PS4`), the keyboard source
    (`Keyboard_PS4`), the cheats, and the earlier special pad reader, thread
    affinity and `CritSec`.

  19 of the map's 97 `os` objects have a source file (26% of the map's `os`
  bytes). Unchanged since `1fd3e74`.
- **Remaining:**
  - `HolmesClient`, `Archive`, `MemTracker`, `Joypad`, `PlatformMgr_PS4`,
    `ContentMgr`, `NetCacheMgr`, `UsbInstrument_PS4`, `Locale` and `HDCache`;
  - `AppChild`, `ArkFile`, `AsyncFile`, `MovieMgr`, `PlatformMgr`'s
    virtuals (the persistence code now calls slots 10, 11 and 14),
    `JoypadClient`, `MapFile` and `MemTrack`, which are declared with their
    addresses only. Their singletons and `*Init` entries are a large share of
    the free functions in the link work queue.

### `render`

- **Done:** the device and frame infrastructure and the scene pipeline:
  - `RndDevice`, `RndContext`, `RndWindow`, `RndBufferCollection`,
    `RndCapabilities` and `RndDefaults`;
  - the shader system: `RndShader`, the backend loader and cache,
    `RndShaderMgr`, all 35 built-in shaders, and every caller of
    `RndShader::_SelectShaderCollection` but one
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
    `RndTypesetter`, and the GPU statistics;
  - since `1fd3e74`, the registry builders and `Init` registrations of the
    light, scene, mesh (126 addresses in `RndMeshCom`), particle, draw and
    instance components, the scene resource's polls and draws, the scene
    poll groups, and the scene's driven globals on the shared `DrivenProp`
    template.

  165 of the map's 321 render objects have a source file (54% of the map's
  render bytes).
- **Remaining:**
  - map objects with no source yet: `RndMaterial` (125 KB), `RndTextCom`,
    `RndSplineCom`, `RndTexture2DPropertiesCom`, `RndFontImportCom`,
    `RndTextureUtilityCom`, and most shader graph nodes (the waveform,
    intrinsic and root nodes are the largest);
  - declared draw paths: `RndParticleCom`'s update steps, the spot, point and
    probe lights' shadow and deferred draws, `RndMeshResource` and
    `RndMeshUtl`, `RndSkeletonPoseCom` (which the anim clips read),
    `RndPixelDataCube`;
  - `RndShaderGraph::Select` (`0x4FE630`), the last
    `_SelectShaderCollection` caller, which needs the graph's resource
    binding (`0x4FE850`).
- **Known issues:**
  - `RndDefaults`' `TextureFamily` layout looks transposed
    ([render-lighting-shaders.md](render-lighting-shaders.md));
  - font kerning pairs are sorted with `std::sort` where the binary uses
    EASTL's sort;
  - several shared-header constants are not modelled, and the component
    documents list weak-evidence slot names.

### `renderps4`

- **Done:** 23 of the map's 24 objects have source: `PS4Device`,
  `PS4Context`, `PS4Factory`, `PS4Window`, every texture, mesh, buffer,
  query and shader-program class, and video output. Only `PS4Common` (empty
  in the map) is missing. Unchanged since `608eab2`.
- **Remaining:** 222 functions in the range, mostly small helpers and
  inlined template copies inside those objects.

### `anim`

Started and nearly finished since `1fd3e74` (`7f92221`, `55986dd`,
`5038476`).

- **Done:** 41 of the map's 45 objects have source (97% of the map's anim
  bytes) ([anim-runtime.md](anim-runtime.md),
  [anim-key-tracks.md](anim-key-tracks.md),
  [anim-prop-keys.md](anim-prop-keys.md),
  [anim-prop-keys-numeric.md](anim-prop-keys-numeric.md),
  [anim-characters.md](anim-characters.md),
  [anim-clips.md](anim-clips.md), [anim-ik.md](anim-ik.md),
  [anim-skeleton.md](anim-skeleton.md)):
  - `AnimCom`, `AnimDriverCom`, the anim and property drivers,
    `AnimResource`, `AnimInstID`, `AnimOptions` and `Anim::Init`;
  - the `TKeyTrack` template, key-track sampling with the easing curves and
    the waveform curve tables;
  - the property-key framework and every key component: bool, int, object,
    symbol, string, waveform, float, Vec3, quaternion, colour, rotation and
    transform; `PropAnimCom`, `PropAnimResource` and `PropertyBlenderCom`;
  - the character animation, skeleton and skeleton pose, bone mask, IK and
    clip classes.
- **Remaining:**
  - map objects `AnimOptionsCom`, `TimeControlsCom`, `CharDrawAnims` and
    `CharBoneTransform` (14 KB together);
  - the offline `CharAnim::Create` builder (`0x32D4B0`, 9.4 KB) and the
    `CharLipsync` XML import, which shipped data does not use;
  - the other `Key.o` spline samplings (seven `TKeyTrack` symbols), and
    `PropAnimResource::_GetSongBeatSource`, which needs `rb_audio`'s
    `RBSongResource`.
- **Known issues:** the `anim` range is smaller than the map's module (see
  [Limitations](#limitations)); the documents list many weak-evidence slot
  and field names.

### `stategraph`

Started and finished since `1fd3e74` (`d365572`, `7f92221`).

- **Done:** all 32 of the map's objects have source
  ([stategraph-runtime.md](stategraph-runtime.md),
  [stategraph-components.md](stategraph-components.md)): the driver
  component, the graph resource and instance, every node, condition and
  transition, the blend tree components, the transition registry builder and
  `DrivenProp::Register`. No stategraph symbol is unresolved.
- **Remaining:** no object. The range's low share (27.8% of its bytes) is
  mostly template copies that do not carry their own address (see
  [Per system](#per-system)), and 47 reconstructed addresses lie in code
  that IDA has not made a function.
- **Known issues:** the classes newer than the map (Folder, TriggerAction,
  the increment nodes, the blend trees and others) have inferred member
  names.

### `persistence`

Started and finished since `1fd3e74` (`55986dd`).

- **Done:** all nine of the map's objects have source
  ([persistence-storage.md](persistence-storage.md),
  [persistence-saveload.md](persistence-saveload.md),
  [persistence-profiles.md](persistence-profiles.md)): `StorageMgr` and
  `StorageMgr_PS4`, `StorageRef_PS4`, `SaveLoadMgr` and its profile and
  system-options clients, `ProfileMgr`, `Profile` and `SystemOptions`.
- **Remaining:** template copies in the range, and the two static
  initializers' shared-header globals (`0x1A6E2A8`, `0x1A6E2D8`).
- **Known issues:** `ProfileMgr`'s slots 13-15, `PlatformMgr`'s slots 10, 11
  and 14 and several field names are inferred.

### `ui`

Started since `1fd3e74` (`048b965`, `a8d3c3c`, `d365572`).

- **Done:** 14 of the map's 16 objects have source (98% of the map's `ui`
  bytes, [ui-system.md](ui-system.md)): `UIMgr` whole, `UI::Init` and
  `Terminate`, most of `UILayoutResource` with its polls and draws, and the
  registrations and much of `UILayoutCom`, `UIInputCom`,
  `UIInputReceiverCom`, the list, list item, list data provider, scrollbar,
  slider and combo box components. The `ui_*` startup adapters are gone.
- **Remaining:** 48 `UI*` symbols are still declared only: `UIListCom`'s
  grid, focus and poll paths (11), `UIInputCom`'s enter, poll, message and
  registration paths (9), the combo boxes' `_Enter`, `_Poll` and `Handle` for
  each value type, and `UIInputReceiverCom`'s poll and messages. The map
  objects `UIMsg` and `UICreatables` have no source.
- **Known issues:** `UILayoutCom` and `UIListEditorDataCom` are newer than
  the map; many members are named from their bodies.

### `net`, `physics`

Neither has been started. The few reconstructed functions in their ranges
belong to `os`, `render` and `persistence` (see [Per system](#per-system)).
The largest objects are:

| System | Largest map objects |
|---|---|
| `net` | `jsoncpp` (105 KB), `DingoSvr`, `DataPointMgr`, `HttpGet`, `WebSvcReq` |
| `physics` | `PhysClothCom` (67 KB), `PhysOptionsCom`, the collidables |

### `rb_*` game modules

- **Done:** `rb_meta` was started in `5038476`
  ([rb-meta-state.md](rb-meta-state.md),
  [rb-meta-dialogs.md](rb-meta-dialogs.md),
  [rb-meta-options-and-layouts.md](rb-meta-options-and-layouts.md)): the
  `RBMetaState` state machine and `RBMetaStateCom` (59 addresses), the dialog
  manager `DialogMgrCom` (44), `RBSystemOptions` (45) and `RBUILayoutCom`
  (21), four of the module's 136 objects (7% of its map bytes). The layout id
  enum moved to `RBMetaStateCom`, where the map puts it. Elsewhere only
  `RBStagePresenceEnum` (`rb_game`) is reconstructed.
- **Remaining:** everything else, about 7.6 MB of code. In `rb_meta`, the
  business-intelligence reports (`0xBB7550`-`0xBC3BB0`),
  `_UpdateJoinedSlots` and `GetCurrentBandName`, and the declared-only
  `SongMgr`, `RBUIInputCom`, `RBCharacterLayoutTrackerCom` and
  `RBProfileMgr`. The largest objects are:
  - `RBCampaignMgr`, `RBGameData`, `RBGigVoteCom` and `RBGigCom`
    (`rb_game`);
  - `RBCharOutfitCom` and `RBCameraShotMgrCom` (`rb_world`);
  - `RBOvershellSlotCom`, `RBCharacterMenuSessionMgrCom` and
    `CalibrationCom` (`rb_meta`);
  - `RBMidiGameData` (`rb_beatmatch`) and `RBVenueAuthoringCom`
    (`rb_audio`).
- **Blockers:** the game managers' singletons (`theBandUserMgr`,
  `theSongMgr`, `theRBProfileMgr`, `theOvershell`, …) are unresolved; they
  belong to `rb_game` and `rb_meta` objects that are not reconstructed.

## Project milestones

The entity-lifecycle milestone was closed by `006db74`. One milestone in
[status.md](status.md) is still open:

1. Link a complete reconstructed executable. This needs the 665 engine and
   game symbols above (or adapters for them) and the matching FMOD libraries.
   The SDK 5.500 compiler is newer than the game's, so the linked code will
   not be byte-identical.

## Methodology

### Reconstructed set

Every reconstructed definition carries a comment of the form
`// Reconstructed from eboot.elf at 0xADDR.`. Some comments list several
addresses, and some wrap onto the next comment line. For each source file at
the commit (`git show <rev>:<path>`), every hex address in the comment's
clause counts, up to the end of the sentence. At `5038476` that gives 8,035
comments and 8,010 distinct addresses.

A function counts as reconstructed when its IDA start address is in this
set. Of the 8,010 addresses:
- 7,875 are IDA function starts;
- eleven lie inside an IDA function or one of its chunks;
- 107 are in code that IDA has not made a function (47 in `stategraph`, 28
  in `entity`, 16 in `anim`, 12 in `render` and four elsewhere);
- fifteen are bytes that IDA has not defined (among them `0x40AC0`,
  `0x1D50E0`, `0x3A2180` and six in `stategraph`);
- two are data (`0x68CB1B`, `0x1934360`).

The cross-check against the compiled objects was not repeated for this
report: other work was rebuilding `build/orbis/Debug/obj` while it was
measured, so the objects no longer matched the commit.

### Binary totals

All 54,914 IDA functions were exported with their start, end, name and size.
The size sums every chunk, including tails. Code that IDA has not placed in
any function was measured separately as "undefined":
- the code bytes between functions, with nop and `int3` padding left out;
- 3.50 MB in the game ranges, in 30,505 runs.

These runs contain 21,496 `push rbp; mov rbp, rsp` prologues, against 22,430
among the defined game functions. IDA therefore still misses nearly as many
frame-pointer functions as it has. The executable probably holds about
73,000-75,000 game functions, not the 52,059 that IDA has defined. Some runs
are not separate functions but the tails of functions that IDA cut short. The
map's older build lists 67,527 game function symbols and 14.9 MB, which
agrees with the larger estimate.

### Changes to the method

- **IDA export refreshed.** IDA's function list changed since `1fd3e74`
  (54,030 to 54,914 functions, all 884 new ones in the game ranges, mostly
  in `stategraph` (+367), `anim` (+265), `ui` (+97) and `persistence`
  (+50), and many more `_Z` names applied). Steps 1 and 2 below were rerun,
  and every row of the trend table, including the earlier commits, is
  measured against the new export. This moves `1fd3e74` from 9.3% and 8.7%
  to 9.1% and 8.6% (its 4,749 functions and 1,136,985 bytes are unchanged,
  only the totals grew); the earlier rows move by 0.1 or 0.2 points. The
  `_Z`-named share rose from 10.0% to 15.2%, mainly because of the renames.
- **Scripts rewritten.** The scripts of the earlier reports are not
  committed and were rewritten from the steps below. The address collection
  reproduces the earlier reports exactly (4,776 addresses at `1fd3e74`,
  2,858 at `608eab2`); it counts 4,801 comments at `1fd3e74` where the
  earlier report counted 4,797. The undefined-code walk gives slightly
  lower figures than before for code IDA did not change: the third-party
  rows are up to 1.4% lower (for example 109,690 against 111,248 bytes for
  the SDK; the PLT is unchanged). Part of the game ranges' drop from 3.64 to 3.50 MB is this
  difference; most of it is the 165 KB that the 884 new functions now cover.
  The run count (30,505 against 36,317) is not comparable, because the new
  walk does not split a run at padding.
- **Map-object coverage.** The per-module object counts are now listed for
  the new modules in a table. A map `.text` object counts when a `.cpp` file
  of the same name exists anywhere under `src/` at the commit. The method
  reproduces the earlier report's counts (for example 42 of 115 `audio`
  objects and 163 of 321 `render` objects at `1fd3e74`).
- **Ranges.** The system ranges are unchanged, so the numbers stay
  comparable with every earlier report. The boundary fixes noted under
  [Limitations](#limitations) were not applied, nor were the two new ones
  found for this report. Applying the new ones would move eleven functions
  (10,576 bytes) from `stategraph` to `ui` and six (279 bytes) from `net` to
  `persistence`; the game total does not change.

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

The `mic` ranges (`0xA1C30`-`0xA2DEC`, `0xE1530`-`0xE1B01`,
`0x275490`-`0x275C06` and `0x27B3E0`-`0x27C7F8`) sit inside `audio`. Each
one runs to the end of the last known `mic` function in it.

The thousands of new reconstruction addresses in `anim`, `stategraph`,
`persistence`, `ui` and `rb_meta` confirm the layout: apart from the
anchors listed under Limitations and two `DrivenProp` header templates
instantiated in render objects, every one of them falls in its own
module's range.

### Limitations

- **Approximate boundaries.** They fall between the nearest anchors and may
  be off by a few kilobytes. Some objects of the map's modules sit in another
  module's range in this build: `PerfMgr` (`os`) inside `utl`, and an `anim`
  resource inside `entity`. The `entity` range (1.4 MB) is much larger than
  the map's `entity` module (0.8 MB), and `anim` is smaller, so part of the
  animation code is probably counted under `entity`. The game total is not
  affected.
- **Misplaced anchors.** These were left in place to keep the numbers
  comparable:
  - `os/Debug.o` starts at about `0x35BA60`, before the `os` boundary
    (`0x35C200`), so nine of its functions count under `net`;
  - `ConsoleLineEditor` (`0x11AECB0`-`0x11B0D20`, an inferred name for code
    the map does not contain) lies in the second `net` block and counts
    there, although it is render code;
  - `MemProject` (`os`, `0x92AA50`) lies in the `rb_*` range, and one
    `RndOptionsCom` function (`0x3BF4C0`) lies in `physics`;
  - new for this report: the `ui` code starts at about `0x891990`, before
    the `ui` boundary (`0x894000`), so eleven `ui` functions count under
    `stategraph`; and `persistence`'s `Profile` functions from `0x11B1AF0`
    lie before the second `persistence` boundary (`0x11B1DE0`), so six of
    them count under `net`.
- **Inlined code.** A reconstructed function whose callees the compiler
  inlined counts once, at its own address. Code reconstructed only as an
  inlined helper, or as a template whose copies carry no address of their
  own, adds nothing.
- **IDA boundaries.** Sizes come from IDA's boundaries. A reconstructed
  function that IDA cut short is undercounted, and functions that IDA has
  not defined cannot count as reconstructed, even when they are (107 such
  addresses at `5038476`, against fourteen at `1fd3e74`).
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
   gaps between them in the code segment, keep the code heads, drop nop and
   `int3` padding, and count `55 48 89 E5` prologues.
3. Collect the reconstruction addresses from `src/` at the commit as
   described above (`git ls-tree` and `git cat-file --batch`, never the
   working tree).
4. Assign every function and undefined run to a system with the layout table,
   and match the reconstructed addresses against the function starts.

The scripts used for this report are not committed. The layout table above
is the only input that is not derived mechanically. Rerun steps 3 and 4 after
each milestone. Rerun steps 1 and 2 only after IDA's functions change, and
then remeasure the earlier commits of the trend table too.
