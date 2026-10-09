# Reconstruction progress

This report measures how much of the executable's own code has been
reconstructed. It is a snapshot of commit `608eab2` ("decomp: reconstruct the
component base, the property system and the audio effects", 2026-10-09 13:12
-0600). The build outputs it quotes are from the Debug object build of that
tree, written at 13:12:11 the same day. Third-party and SDK code is reported
separately and left out of the totals.

## Summary

| Scope | Functions | Reconstructed | By count | Bytes | Reconstructed bytes | By bytes |
|---|---:|---:|---:|---:|---:|---:|
| Game code (engine and Rock Band) | 50,525 | 2,841 | **5.6%** | 12,947,601 | 690,856 | **5.3%** |
| Engine only (all but the `rb_*` modules) | 30,898 | 2,839 | 9.2% | 6,782,855 | 688,166 | 10.1% |
| Rock Band game modules (`rb_*`) | 19,627 | 2 | 0.0% | 6,164,746 | 2,690 | 0.0% |

The counts use the functions that IDA has defined. IDA has not defined every
routine: a further 3.73 MB of code in the game ranges belongs to no IDA
function (see [Methodology](#methodology)). Measured against all code bytes,
16.68 MB, the game code is **4.1%** reconstructed, and the engine alone is
7.7%.

IDA has an original mangled (`_Z`) name for 6.6% of the game-code functions,
and 10.7% of the engine functions.

The reconstructed set has grown quickly over the last five commits:

| Commit | Reconstructed functions | By count | By bytes |
|---|---:|---:|---:|
| `c6fee06` | 1,819 | 3.6% | 3.5% |
| `d5c8ada` | 1,820 | 3.6% | 3.5% |
| `3be3757` | 2,256 | 4.5% | 4.3% |
| `c58714e` | 2,508 | 5.0% | 4.8% |
| `608eab2` | 2,841 | 5.6% | 5.3% |

Uncommitted work in the tree when this report was written adds 131
addresses, 80 of them IDA functions, mainly in `audio`, `entity` and `utl`
(for example `MsgSink`). These would raise the game-code figures to 5.8% by
count and 5.5% by bytes.

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
| `rockband` | 15 | 3 | 20.0% | 1,147 | 811 | 70.7% | 1,008 | 37.6% | 13.3% |
| `audio` | 3,962 | 1,067 | 26.9% | 793,516 | 228,392 | 28.8% | 242,169 | 22.1% | 27.5% |
| `mic` | 90 | 84 | 93.3% | 12,141 | 11,384 | 93.8% | 266 | 91.8% | 95.6% |
| `entity` | 4,179 | 174 | 4.2% | 1,192,983 | 40,170 | 3.4% | 242,976 | 2.8% | 4.1% |
| `math` | 241 | 112 | 46.5% | 69,736 | 23,037 | 33.0% | 19,985 | 25.7% | 46.9% |
| `utl` | 977 | 279 | 28.6% | 279,967 | 54,286 | 19.4% | 27,916 | 17.6% | 30.4% |
| `os` | 1,079 | 21 | 1.9% | 262,034 | 2,579 | 1.0% | 72,218 | 0.8% | 3.9% |
| `render` | 10,941 | 844 | 7.7% | 2,345,591 | 275,435 | 11.7% | 689,061 | 9.1% | 10.1% |
| `renderps4` | 477 | 255 | 53.5% | 90,178 | 52,072 | 57.7% | 316 | 57.5% | 85.5% |
| `anim` | 1,673 | 0 | 0.0% | 345,908 | 0 | 0.0% | 188,130 | 0.0% | 0.0% |
| `net` | 481 | 0 | 0.0% | 119,647 | 0 | 0.0% | 35,568 | 0.0% | 0.0% |
| `persistence` | 203 | 0 | 0.0% | 42,924 | 0 | 0.0% | 10,457 | 0.0% | 0.0% |
| `physics` | 359 | 0 | 0.0% | 83,997 | 0 | 0.0% | 35,150 | 0.0% | 0.0% |
| `stategraph` | 5,544 | 0 | 0.0% | 965,225 | 0 | 0.0% | 584,708 | 0.0% | 0.0% |
| `ui` | 677 | 0 | 0.0% | 177,861 | 0 | 0.0% | 47,751 | 0.0% | 0.4% |
| `rb_*` game modules | 19,627 | 2 | 0.0% | 6,164,746 | 2,690 | 0.0% | 1,531,136 | 0.0% | 0.0% |
| **Game code** | **50,525** | **2,841** | **5.6%** | **12,947,601** | **690,856** | **5.3%** | **3,728,815** | **4.1%** | **6.6%** |

The seven `rb_*` modules (`rb_audio`, `rb_beatmatch`, `rb_game`,
`rb_gtrsolo`, `rb_meta`, `rb_track`, `rb_world`) are counted together. Too
few named functions fall inside them to place the boundaries between them.

Excluded third-party, SDK and import code:

| Code | Funcs | Bytes | Undefined | `_Z` named |
|---|---:|---:|---:|---:|
| PS4 SDK (Gnm, Gnmx, GpuAddress, …), Bink, Positive Grid BIAS | 1,088 | 404,947 | 109,690 | 13.6% |
| RakNet (two blocks) | 694 | 257,658 | 82,401 | 0.0% |
| libpng, zlib, kiss_fft | 260 | 119,216 | 21,547 | 0.0% |
| Ogg Vorbis, LibTomCrypt | 162 | 110,620 | 31,671 | 0.0% |
| PLT import stubs | 651 | 16,682 | 0 | 30.1% |

FMOD is not in this table. The reference map links FMOD statically, but this
executable loads `libfmod` and `libfmodstudio` as PRX modules and only has
PLT stubs for them.

## Build health

These figures come from `build/orbis/Debug` (SDK 5.500, Debug), written at
13:12:11 on 2026-10-09, 46 seconds before commit `608eab2`. The set of
translation units matches that commit's `src/*.cpp` files exactly.

| Metric | Value |
|---|---:|
| Translation units compiled (`objects.csv`) | 291 |
| Source files at `608eab2` (`.cpp` / `.h`) | 291 / 361, about 82,000 lines |
| Relocatable link (`rb4_reconstruction.o`) | produced, 15.9 MB |
| Unresolved symbols (`undefined-symbols.txt`) | 655 |
| Warnings | not recorded; the script builds with `-Wall` but keeps no log |

The 655 unresolved symbols fall into these groups:

| Group | Symbols |
|---|---:|
| PS4 SDK services (`sce*`) | 217 |
| FMOD C API (`FMOD_*`) | 116 |
| C and C++ runtime, `std::`, `eastl::` | 58 |
| Engine code not yet reconstructed | 264 |

The 264 engine symbols are the work queue for linking. 122 of them are free
functions and globals:
- `File*`, `Mem*` and `System*` from `os`;
- `Data*` from `utl`;
- the `*_register_types` and per-frame `*_update` entries that the frame loop
  calls (several are descriptive names, see [frame-loop.md](frame-loop.md)).

The other 142 are members of about 85 classes. The largest groups are:
- `VirtualInstrument` (11 symbols), `Entity` (9), `FusionPatchCom` (6) and
  `EntityResource` (4);
- the music generators (`Mogg*`, `Midi*`, `MusicTimeline*`, three each);
- the remaining FMOD DSP plug-ins and amp models;
- the audio, light, material and scene components' `Init`s.

`build/orbis/Debug/obj` also holds about 311 stale objects from earlier
source layouts. Only the 291 listed in `objects.csv` are linked.

## Systems

### `rockband`

- **Done:** `Main.o` is complete: `App::Initialize`, `App::RunOneFrame` and
  `main` ([startup.md](startup.md), [frame-loop.md](frame-loop.md)). The other
  twelve functions in the range are CRT and shim code.
- **Remaining:** none in the module itself. Many of the frame loop's callees
  still have descriptive names, because their owning classes are unknown.

### `audio`

- **Done:** the core engine:
  - `SoundManager`, `AudioGenerator` and its managers, the bus, composite,
    dialog, Fusion, MIDI, Mogg and timeline generators;
  - `FusionSampler`, `FusionVoice` and `FusionVoicePool`, `ADSR`, `LFO` and
    the modulators;
  - `AudioBus`, the mixer and render targets, `StreamReaderThread`,
    `AudioBuffer`, `WaveFile` and the RIFF chunks;
  - the DSP effects (filters, bit crusher, delay, distortion, `PitchDetector`,
    `SndAnalysis`, `SmbPitchShift`) and `AmpSimulator`.

  The FMOD integration is done too: `FModSystem` and `FmodPlatform`, the
  bus, stream, buffered-stream, dialog and Studio generators, the bank and
  stream resources, `FmodFileWrapper`, and the gain and pitch-shift plug-ins.
- **Remaining:**
  - map objects with no source yet: `MidiMsgBroadcasterCom`, `VorbisReader`,
    `MoggResource`, `ByteGrinder`, `StandardStream`, `MidiPlayCursor` and
    `MidiPlayCursorMgr`, `MidiReader` and `PcmAudioData`;
  - declared but not reconstructed: `VirtualInstrument`, the audio resource
    and component `Init`s, the remaining FMOD DSP plug-ins (delay, filter,
    wah, vibe, stutter, tremolo, bit crusher, signal tap, analysis), the
    `AmpModel*` classes, and the PCM and Mogg decoders.
- **Blockers:** linking needs the FMOD 1.10.04 libraries for the 116 FMOD
  imports.

### `mic`

- **Done:** almost complete. `Mic`, `MicHwManager`, `MicReaderThread`,
  `MicHwManager_FMOD` and `Mic_FMOD` are reconstructed
  ([mic-fmod.md](mic-fmod.md)).
- **Remaining:** `MicClientMapper` and `MicNull` from the map. Neither may be
  linked into this build.
- **Note:** this build places the `mic` objects inside the audio ranges. Its
  ranges are the spans of its known functions.

### `entity`

- **Done:** the resource and component core
  ([entity-resources.md](entity-resources.md),
  [entity-components.md](entity-components.md)):
  - `Resource`, `ResourceMetaData`, `ResourcePath` and the cache paths;
  - `EntityResource` and `TransEntityResource`, with their 33-slot vtables;
  - the entity's object lookup and resource loading;
  - `GameObject`'s component creation, sorting and destruction;
  - `Component`, with its 41-slot vtable, and `ComMetaData`;
  - `PropRegistry`, `PropInfo` and `PropArray`, and parts of `TransCom`.
- **Remaining:**
  - `PropMetadata` (215 KB in the map, the largest), `ScriptCom`,
    `EditorRootCom`, `Waveform`, `InstanceCom` and `EntityPoolCom`;
  - the entity resource loaders, `ComMetaData::Init` (`0xE5B80`) and the
    component ordering (`0x117A90`).
- **Blockers:** the entity's constructor, `Enter`, `Exit`, `_Poll` and
  `_Destroy` need the `MsgSource` and `PollDepBase` bases (vtables
  `0x18E6818` and `0x18E6888`), which are not modelled yet. This is one of
  the two open milestones in [status.md](status.md).
- **Known issues:** several `EntityResource` and `Component` slot names rest
  on their bodies and callers only (see those documents' "Weak evidence"
  sections).

### `math`

- **Done:**
  - `Color`, `Interp`, `Matrix3`, `Matrix4`, `Rand` and `Rand2`, `Rot`,
    `Transform`, `Trig` and `Half`;
  - the vector types, `DoubleExponentialSmoother`;
  - the geometry objects `Frustum`, `Geo`, `Plane` and
    `TruncatedRoundedCone`.
- **Remaining:** `trimesh`, `ConvexHull`, `ForceObj`, `SHA1`, `MD5`,
  `Sampler`, `Sphere` and `Capsule`. Also the two 36-byte objects built by
  the static initializer at `0x2195C0`
  ([math-foundation.md](math-foundation.md)).

### `utl`

- **Done:** `BinStream`, `FileStream`, `TextStream`, `Str`, `MakeString`,
  `UTF8`, `Thread`, `Thread_PS4`, `ThreadCall`, `Timer`, and the core of
  `DataArray`, `DataNode` and `DataUtl` ([core-io.md](core-io.md),
  [foundation-runtime.md](foundation-runtime.md)).
- **Remaining:**
  - `DataFunc` (90 KB), `DataFile` and `MsgSink` (in progress in the working
    tree);
  - `MtJobMgr`, `Symbol`, `GlitchFinder`, `DateTime`, `PngWrite`, `DataFlex`
    and `StringTable`;
  - `PollMgr::IsWorkerThread`, `DataVarIndex` and `DataVariable`.

### `os`

- **Done:** small pieces: the special pad reader, thread affinity, `CritSec`,
  and parts of the debug, file, memory and platform code. These are 1% of
  the system's bytes.
- **Remaining:**
  - nearly all of it: `HolmesClient`, `Archive`, `MemTracker`, `Joypad`,
    `PlatformMgr_PS4`, `ContentMgr`, `NetCacheMgr`, `UsbInstrument_PS4`,
    `MemHeap` and `MemMgr`;
  - most of the `File*`, `Mem*` and `System*` functions. Reconstructed code
    already calls them, so they are a large share of the link work queue.

### `render`

- **Done:** the device and frame infrastructure:
  - `RndDevice`, `RndContext`, `RndWindow`, `RndBufferCollection`,
    `RndCapabilities` and `RndDefaults`;
  - the shader system: `RndShader`, the backend loader and cache,
    `RndShaderMgr` and all 35 built-in shaders;
  - textures and pixel data, meshes and mesh builders, compute and particle
    buffers, occlusion queries;
  - the tiled-light, SSAO, CMAA and volumetric compute passes;
  - the debug overlays, fonts and `RndTypesetter`, and the GPU statistics.

  107 of the map's 321 render objects have a source file.
- **Remaining:** the scene and material side:
  - `RndMeshCom` (143 KB), `RndParticleCom` (142 KB), `RndMaterial`
    (125 KB), `RndTextCom`, `RndShaderGraph` and its nodes;
  - `RndSplineCom`, `RndSceneCom`, `RndSceneDrawer` and
    `RndTexture2DPropertiesCom`;
  - most per-pass draw functions: 36 callers of
    `RndShader::_SelectShaderCollection`, the blur draw (`0x634BB0`) and the
    bloom pass (`0x6305A0`).
- **Known issues:**
  - `RndDefaults`' `TextureFamily` layout looks transposed
    ([render-lighting-shaders.md](render-lighting-shaders.md));
  - the console input's slots 6-9 are not modelled;
  - font kerning pairs are sorted with `std::sort` where the binary uses
    EASTL's sort.

### `renderps4`

- **Done:** 23 of the map's 24 objects have source: `PS4Device`,
  `PS4Context`, `PS4Factory`, `PS4Window`, every texture, mesh, buffer,
  query and shader-program class, and video output. Only `PS4Common` (empty
  in the map) is missing.
- **Remaining:** 222 functions in the range, mostly small helpers and
  inlined template copies inside those objects.

### `anim`, `net`, `persistence`, `physics`, `stategraph`, `ui`

None of these has been started. Only `UIMgr` and `RBUILayout` have
declarations, for the frame loop. The largest objects are:

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

Two milestones in [status.md](status.md) are still open:

1. Reconstruct the entity resource loaders and the entity's enter, poll and
   destroy. This first needs the load-progress listeners, `MsgSource` and
   `PollDepBase`.
2. Link a complete reconstructed executable. This needs the 264 engine
   symbols above (or adapters for them) and the matching FMOD libraries. The
   SDK 5.500 compiler is newer than the game's, so the linked code will not
   be byte-identical.

## Methodology

### Reconstructed set

Every reconstructed definition carries a comment of the form
`// Reconstructed from eboot.elf at 0xADDR.`. Some comments list several
addresses, and some wrap onto the next comment line. For each source file at
the commit (`git show <rev>:<path>`), every hex address in the comment's
clause counts, up to the end of the sentence. At `608eab2` that gives 2,884
comments and 2,858 distinct addresses.

A function counts as reconstructed when its IDA start address is in this
set. Of the 2,858 addresses:
- 2,841 are IDA function starts;
- two lie inside an IDA function;
- thirteen are in code that IDA has not made a function;
- two are data tables.

As a cross-check, the existing compiled objects were mapped back to their
comments with the symbol-to-address helper that reads `orbis-objdump -d -l`
line information. It found 2,852 compiled functions and no address missing
from the comment set.

### Binary totals

All 53,380 IDA functions were exported with their start, end, name and size.
The size sums every chunk, including tails. Code that IDA has not placed in
any function was measured separately as "undefined":
- the code bytes between functions, with nop and `int3` padding left out;
- 3.73 MB in the game ranges, in 37,420 runs.

These runs contain 22,329 `push rbp; mov rbp, rsp` prologues, against 22,682
among the defined functions. IDA therefore misses roughly as many
frame-pointer functions as it has. The executable probably holds about 75,000
game functions, not the 50,525 that IDA has defined. Some runs are not
separate functions but the tails of functions that IDA cut short. The map's
older build lists 67,527 game function symbols and 14.9 MB, which agrees with
the larger estimate.

### Binary layout

The map is older than the executable, and its link order is different. It
lists the modules alphabetically, while this build groups them as below and
places a second, smaller block of engine and game code after the SDK
libraries. The ranges were placed from three kinds of anchors:
- 2,220 IDA or compiled names that match a map symbol, giving the map object;
- the 2,858 reconstruction addresses, giving their source module;
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
- **Inlined code.** A reconstructed function whose callees the compiler
  inlined counts once, at its own address. Code reconstructed only as an
  inlined helper adds nothing.
- **IDA boundaries.** Sizes come from IDA's boundaries. A reconstructed
  function that IDA cut short is undercounted, and functions that IDA has
  not defined cannot count as reconstructed, even when they are.
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
2. Export the undefined code: walk the gaps between functions, keep the code
   heads that `ida_funcs.get_func` does not claim, drop nop and `int3`
   padding, and count `55 48 89 E5` prologues.
3. Collect the reconstruction addresses from `src/` at the commit as
   described above.
4. Assign every function and undefined run to a system with the layout table,
   and match the reconstructed addresses against the function starts.

The scripts used for this report are not committed. The layout table above
is the only input that is not derived mechanically. Rerun steps 3 and 4 after
each milestone. Rerun steps 1 and 2 only after IDA's functions change.
