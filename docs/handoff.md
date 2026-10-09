# Reconstruction handoff

## Snapshot

This document describes the repository on branch `main` after the
reconstruction of the PS4 shader programs, the twenty-third step of
the conversion to the reference map's original names, classes, and module
layout (see [naming.md](naming.md) and [code-review.md](code-review.md)). The
engine foundation, the render resource objects, textures, meshes, and the
render context, the render device (`RndDevice`/`PS4Device`), the windows (`RndWindow`/`PS4Window`), the buffer collections (`RndBufferCollection`), the shader system (`RndShader`, its 35 built-in subclasses, and `RndShaderMgr`), and the audio and
microphone subsystems are converted; the rest of the renderer and the game
code still use the earlier names. The
working tree was clean when the snapshot was taken.

The current PS4 object build compiles **142 C++ translation units**. It creates
a complete relocatable object and archive, but it does not yet produce a game
executable. The latest unresolved-symbol report contains 680 entries, most of the
growth since the previous snapshot coming from FMOD loaders and decoders the
audio conversion declared but has not reconstructed. It covers
engine code that has not been reconstructed, external runtime APIs,
and middleware dependencies.

Rendering is roughly three quarters structurally reconstructed. The resource,
target, texture, mesh, synchronization, Orbis context, and built-in shader
foundations are extensive. Runtime integration is less complete because the
primary shader backend loader, several high-level passes, engine adapters, and
the final executable link remain unfinished. Treat percentages as planning
estimates rather than coverage measurements.

## Ground rules

Read [AGENTS.md](../AGENTS.md) before changing the tree. Its rules apply to the
whole project:

- Continue autonomously through focused milestones and commits.
- Stop only for a real external blocker or when the user explicitly asks.
- Organize source first by subsystem and then by coherent responsibility.
- Use source-root-qualified includes across folders.
- Keep platform-neutral code separate from `platform/orbis` code.
- Update the relevant documentation and run the PS4 object build before each
  milestone commit.

Do not collect unrelated implementation files in broad folders. New renderer
passes belong in their functional domain, such as `postprocessing/bloom`,
`postprocessing/blur`, `masking`, or `core/debug`.

## Binary and analysis artifacts

The original executable is an unencrypted and uncompressed PS4 SELF. The
important local artifacts are:

| Artifact | Purpose | Size | SHA-256 |
| --- | --- | ---: | --- |
| `files/eboot.bin` | Original PS4 SELF | 29,455,849 | `3ECE8A3779D5EB2003A96C0C4A82639B14A2F2C62E483257B3A0EF4E3569EF8D` |
| `files/eboot.elf` | Extracted executable | 29,431,241 | `375479BC8C1CB44373DCF897D770CE56D23BD0D4053A9D420F9912BEF38DA704` |
| `files/eboot_ida.elf` | ELF type normalized for IDA | 29,431,241 | `F3ACBE2EE79AB72D4F664E4FC19D38CB0DF215ABFC194E7EF073F38FF37A1537` |
| `analysis/ida/eboot.i64` | Persistent IDA 9.4 database | about 259 MB | local/ignored |

The IDA-compatible file differs from `eboot.elf` only in `e_type`: Sony's
`ET_SCE_DYNEXEC` is changed to `ET_DYN`. Program data is unchanged. Full
container and ELF details are in [binary.md](binary.md).

To reproduce extraction:

```powershell
python tools/extract_ps4_self.py files/eboot.bin files/eboot.elf `
  --ida-output files/eboot_ida.elf
```

The baseline export reports 52,152 functions, 37,828 strings, 161 recovered
source paths, and Hex-Rays availability. Machine-readable evidence is under
`analysis/exports`; see [ida-baseline.md](ida-baseline.md).

## SDK and build environment

The executable's process parameter records revision `5.008.001` and belongs to
the PS4 SDK 5.000 generation. The locally preserved 5.008 files provide headers
and stub libraries, but no matching compiler toolchain. They are useful for
import recovery only.

The working compiler is the installed SDK at:

```text
C:\Program Files (x86)\SCE\ORBIS SDKs\5.500
```

It provides Clang 5.0.1 and the Orbis 5.50 linker. This compiler validates PS4
ABI compatibility and source structure; it is newer than the original compiler
and cannot be expected to produce byte-identical code. Visual Studio is not
required for the current reconstruction workflow.

Run the full build from the repository root:

```powershell
.\tools\build_ps4.cmd `
  -SdkDir "C:\Program Files (x86)\SCE\ORBIS SDKs\5.500" `
  -Configuration Debug
```

The `.cmd` wrapper bypasses restrictive PowerShell execution policy. Build
products are ignored under `build/orbis/Debug`:

- `librb4_reconstruction.a`: archive of reconstructed objects.
- `rb4_reconstruction.o`: relocatable link of every translation unit.
- `objects.csv`: object sizes and SHA-256 hashes.
- `undefined-symbols.txt`: demangled unresolved dependencies.
- `obj/`: per-source Orbis objects.

The build must complete without warnings introduced by the current milestone.
Afterward, search `undefined-symbols.txt` for any adapter being removed and run:

```powershell
git diff --check
git status --short
```

See [ps4-build.md](ps4-build.md) for tool versions and optional disassembly.

## IDA workflow

Use the direct IDA integration against `analysis/ida/eboot.i64`. Database
instance IDs are session-specific; discover the active instance instead of
copying an old ID. A reliable workflow is:

1. List open databases and select the database whose path ends in
   `analysis/ida/eboot.i64`.
2. Open or attach to that database and make it the current target.
3. Consult the IDA API reference before the first Python operation in a
   session.
4. Decompile constructors and read their seven-entry dispatch tables.
5. Verify small leaf methods with raw disassembly when Hex-Rays returns `None`.
6. Apply evidence-backed function names after the source implementation is
   understood.
7. Save the database after each naming milestone.

Some optimized leaf methods are not defined as IDA functions. Do not assume
that `None` from Hex-Rays means no code exists. Inspect bytes and boundaries.
When necessary, undefine only the verified range, recreate instructions, and
add a function with explicit start/end addresses. An incorrect end address can
swallow the next function and corrupt later pseudocode.

For built-in shaders, the dispatch normally contains:

1. destructor;
2. deleting destructor;
3. source identifier;
4. backend HLSL path;
5. support-object initializer;
6. mode;
7. platform/stage variant.

Recover constructor field initialization exactly, including zero values and
intentional gaps. Use the initializer to identify parameter registries,
constant definitions, constant-block members, texture/buffer bindings, shader
stages, resource dimensions, and masks. Preserve the address in a nearby
`Reconstructed from eboot.elf at ...` comment.

## Import recovery

All 706 PLT imports have names. SDK 5.008 stubs resolve the Orbis and runtime
APIs; reviewed maps resolve the private pad and FMOD 1.10.04 symbols. Reproduce
the import table with:

```powershell
python tools/resolve_ps4_imports.py files/eboot.elf `
  --sdk tools/ps4-sdk `
  --output analysis/exports/imports.csv
```

The local SDK copy under `tools/ps4-sdk` is intentionally ignored by Git. Do
not commit proprietary SDK files. Details and evidence policy are in
[imports.md](imports.md), [fmod-imports.md](fmod-imports.md), and
[special-pad-reader.md](special-pad-reader.md).

## Current reconstruction state

The high-confidence reconstructed foundation includes:

- Startup, top-level system initialization/shutdown, and the main frame loop.
- UI layout IDs and primary asset-path mappings.
- FMOD initialization, Orbis integration, IO, playback generators, recording,
  bank/resource handling, listener state, and timing reports, written as the
  original classes (`FModSystem`, `FmodAudioStreamGenerator`,
  `FmodStudioSoundGenerator`, `FmodDialogGenerator`, `FmodAudioBusGenerator`,
  `FmodFileWrapper`, `MicHwManager_FMOD`, `Mic_FMOD`, the DSP plug-ins).
- Renderer settings, platform capabilities, debug commands, frame lifecycle,
  resource manager, and default resources.
- Common texture, render-target, buffer, mesh, particle, fence, query, and
  shader resource bases.
- Orbis render system, contexts, command submission, video output, barriers,
  synchronization, GPU timing, texture backends, buffers, and mesh backends.
- Render targets for depth, GBuffer, lighting, masks, CMAA, sky, intermediate,
  shadows, ambient occlusion, light probes, and volumetric scattering.
- Source-owned dispatch for many compute and graphics shaders, including
  FogDeferred, CMAA, SSAO, linear/tiled depth, buffer operations,
  signed-distance fields, blur classification, DOF, volumetric scattering,
  FXAA, downsample, scene masks, debug displays, and basic/error fallbacks.

The detailed checklist is [status.md](status.md). The address-to-source ledger
is [reconstruction.md](reconstruction.md). Update both when completing a new
milestone.

## Most recent milestones

The latest focused commits, newest first, are:

| Commit | Milestone |
| --- | --- |
| (this) | `PS4Texture2D` storage, syncs and back buffer; `PS4Window` constructor reconstructed |
| `9110da5` | `PS4TextureArray2D` color and depth syncs reconstructed |
| `7f68082` | `PS4TextureCube` color and depth syncs reconstructed |
| `75d3fba` | PS4 1D, 3D, 1D-array and cube-array texture syncs reconstructed |
| `d010b77` | `PS4OcclusionQuery` commands reconstructed on the Gnm SDK |
| `ec6009e` | `PS4ComputeBuffer` creation and stage binding reconstructed |
| `100d054` | PS4 shader programs, compute contexts and constant-buffer select reconstructed |
| `a736a52` | Last `rb4` code moved to the global namespace with SDK render-target types; `PS4ParticleBuffer` draw and creation reconstructed |
| `c2913b0` | PS4 backend switched to the Gnm SDK types; `PS4Context::SetupDraw` reconstructed; screenshot callback is a `std::function` |
| `66ad08a` | Remaining render helpers converted (`PlatformMgr`, `RndPixelFormat`, `RndPrimitiveMeshes`, `RndAudioTextures`, `BinkRenderMgr`, `RndLightMgrCom`); main-loop calls named from the map |
| `072eb2a` | Render debug and shader helpers converted to `RndBufferInspection`, `RndShaderIncludeChecksums`, `RndShaderUtl`, `RndShaderDrawUtl` |
| `cd781a8` | Game, input and UI code moved to `App`/`main`, `StagePresence`, `PembrokeGuitarController`, `UILayoutId` |
| `0d6f812` | Remaining `src/render/core` and `src/render/resources` files moved into the `src/render` domains |
| `8432703` | `src/render/platform/orbis` folded into `PS4Context`, `PS4Device`, `PS4Window`, `PS4MeshTyped`, `PS4RenderUtl` |
| `592987d` | Render settings and capabilities converted to `RndConfig` and `RndCapabilities` |
| `1081c74` | `RndLightGlobals`, `RndCommands`, `Rnd::Init`/`Terminate`, `PS4TransientBuffer`, `PS4RenderStateUtl` |
| `d857895` | Default resources converted to `RndDefaults` |
| `80d6619` | GPU statistics converted to `RndGpuStatsMgr` |
| `4e0a874` | Resource manager converted to `RndShaderMgr` |
| `0fd029c` | Shader system (`RndShader`, configuration classes, 35 built-in shader subclasses) converted |
| `f161a7b` | Render buffer collections (`RndBufferCollection`, `RndBufferCollection2D`) converted; missing registrations restored |
| `7fb80f3` | Render windows (`RndWindow`, `RndBufferedWindow`, `PS4Window`) converted to original classes |
| `920b374` | Render device (`RndDevice`, `PS4Device`, `CritSec`, `Condition`) converted to original classes |
| `6f3b574` | Docs for the audio merge |
| `528bbea` | Audio and microphone subsystems converted to original classes (merge) |
| `00c11a0` | Render context release fix |
| `00bc0d4` | Render context converted to original classes |
| `e149abf` | Meshes converted to original classes |
| `32dba93` | Textures converted to original classes |
| `c589005` | Render resource objects converted to original classes |
| `3338af8` | Engine foundation converted to original classes |
| `3797688` | DOF sprite draw and per-stage binding |
| `45a942c` | Test-pattern and render-test-simple draws |
| `9c93567` | Single-texture linearize-depth, refine-mask, sphere-map draws |
| `2537e18` | Downsample draw |
| `d0cdc60` | Output-conversion draw and sRGB conversion |
| `b94af4c` | Bloom draw and shared pass-draw helpers |
| `0139c87` | Primary-shader permutation bind and error-shader fallback |
| `d7172aa` | Primary-shader backend initialization and cache loading |
| `5a02e7f` | Shader permutation enumeration and layout hash |
| `8075c64` | All eleven primary-shader dispatch slots |
| `b689eaa` | Shader cache validation hashes |
| `9441703` | Core `BinStream`, `FileStream`, and engine file wrappers |
| `f5c6c6c` | Compiled-shader object loader |
| `06bca38` | Handoff update after adapter retirement |
| `30cdd55` | Test-pattern and render-test-simple shaders; shader adapter files deleted |
| `55550d8` | Bink conversion shader |
| `025da37` | Output-conversion shader |
| `0c5132a` | Blur shader |
| `b197f28` | Bloom shader |
| `ab29d14` | Basic and error shader dispatches |
| `53961ae` | Debug display shader trio |
| `4c3b48a` | Refine/stencil scene-mask shaders |
| `9acba79` | Linear-depth graphics shader |
| `ea25dc9` | Downsample shader and graphics texture-binding helper |
| `250d229` | DOF sprite shader |
| `fcb9c59` | FXAA shader |
| `d2bca03` | Render-test compute shader; final generic compute adapter removed |
| `74a0613` | Volumetric-scattering compute shader trio |
| `7f30c56` | DOF disc-blur shader and structured output binding |
| `3a708cf` | Blur-classification compute shader |
| `648da08` | Signed-distance compute shader pair |

Every milestone's IDA renames and function-boundary repairs have been applied
and the database saved. Four shader milestones were first reconstructed from
raw ELF disassembly while IDA Python access was blocked. Their renames were
applied afterwards.

## Basic and error shader milestone in depth

The last implementation milestone was the reconstruction of the basic and
error fallback shaders in commit `ab29d144623b692131bf4bdb78ea8b164bd692b8`
(`decomp: reconstruct basic shaders`). That work is complete; there was no
partially edited source file or uncommitted IDA rename at the stopping point.
The next shader milestone should start with one of the six adapters listed in
the next section rather than revisiting these two classes without new binary
evidence.

### Shared object and dispatch layout

Both classes inherit the reconstructed 288-byte
`RndShader` prefix. Their class-specific parameter bindings
start at object offset 288 and occupy 20 bytes each. The recovered 56-byte
dispatch table has seven entries in this order:

1. destructor
2. deleting destructor
3. source identifier
4. backend shader path
5. support-object initializer
6. shader mode
7. shader variant

Both dispatches use the primary shader destructor, release the allocation in
their deleting destructor, return mode `0`, and return primary graphics variant
`13`. The source reconstruction represents this common shape with
`BasicShaderDispatch` in
`src/render/shaders/RndShaderBasic.cpp`. The shared binary leaf
functions for mode and variant are at `0x450850` and `0x6388E0`, respectively.

### Error shader reconstruction

`render_error_shader_construct` was recovered from `0x63E650` through
`0x63E837`. Its original dispatch table is at `0x192F2A0`. The verified binary
methods were named and saved in IDA as follows:

| Address | IDA name | Recovered behavior |
| ---: | --- | --- |
| `0x63E690` | `error_shader_destruct` | Calls the primary shader destructor |
| `0x63E6A0` | `error_shader_delete` | Destructs and releases the object |
| `0x63E730` | `error_shader_backend_path` | Returns `../../system/data/shaders/Error.hlsl` |
| `0x63E740` | `error_shader_initialize_support_objects` | Registers the two compile parameters |
| `0x63E830` | `error_shader_source_identifier` | Returns `RndShaderError` |

The constructor clears exactly two parameter bindings. The support initializer
then registers:

| Symbol | Parameter registry | Range |
| --- | ---: | ---: |
| `HX_GEO_TYPE` | `registries[1]` | `[0, 2)` |
| `HX_SHADING_MODE` | `registries[4]` | `[0, 19)` |

The initializer at `0x63E740` initially had no usable function boundary, so
Hex-Rays could not decompile it. The byte range `0x63E740`-`0x63E829` was
undefined without touching adjacent code, recreated as instructions, and then
made into an explicit function. The recovered accesses at parameter-set
offsets 40 and 160 proved registry indices 1 and 4. This repair is already
stored in the IDA database and should be preserved.

### Basic shader reconstruction

`render_basic_shader_construct` was recovered from `0x6398D0` through
`0x639F07`. Its original dispatch table is at `0x192F018`. The verified binary
methods were named and saved in IDA as follows:

| Address | IDA name | Recovered behavior |
| ---: | --- | --- |
| `0x639950` | `basic_shader_destruct` | Calls the primary shader destructor |
| `0x639960` | `basic_shader_delete` | Destructs and releases the object |
| `0x639BD0` | `basic_shader_backend_path` | Returns `../../system/data/shaders/Basic.hlsl` |
| `0x639BE0` | `basic_shader_initialize_support_objects` | Registers parameters, constants, and resources |
| `0x639F00` | `basic_shader_source_identifier` | Returns `RndShaderBasic` |

The constructor clears four parameter bindings and initializes its four
class-specific 64-bit fields exactly as the binary does:

| Object offset | Initial value | Value assigned by support initialization |
| ---: | ---: | --- |
| `368` | `-1` | Handle for vector4 constant `gColor` |
| `376` | `0` | Constant block `next_offset` after adding `gColor` |
| `384` | `-1` | Handle for `gTexture2D` / `gTex2DSampler` |
| `392` | `-1` | Handle for `gTexture2DRTSliced` / `gTex2DRTSlicedSampler` |

All four compile parameters use pixel-stage registry `registries[4]`:

| Symbol | Registration |
| --- | --- |
| `HX_SHADING_MODE` | Integer range `[0, 19)` |
| `HX_TEXTURE_MODE` | Integer range `[0, 3)` |
| `HX_ALPHA_CUT` | Ternary parameter |
| `HX_USE_TEX_RED_AS_ALPHA` | Ternary parameter |

The support initializer also registers the constant definitions
`HX_TEXTURE_MODE_NONE = 0`, `HX_TEXTURE_MODE_2D = 1`, and
`HX_TEXTURE_MODE_2D_RTSLICED = 2`. It adds `gColor` as a vector4 constant,
then preserves the resulting constant-block extent in the field at offset 376.

The ordinary texture binding uses dimension `1`, stage `4`, and mask `12`.
The render-target-sliced texture also uses dimension `1` and mask `12`, but it
must go through `render_shader_backend_add_graphics_texture_binding`. That
graphics-specific helper was reconstructed earlier at `0x6438D0`; it supplies
the fixed pixel stage and additional graphics binding metadata. Do not replace
it with the general texture helper merely because their call shapes are
similar.

### Source ownership and cleanup performed

The milestone added the source-owned dispatches and constructors in
`src/render/shaders/RndShaderBasic.cpp`. It removed
`render_error_shader_install_dispatch` and
`render_basic_shader_install_dispatch` from
`src/render/shaders/RndShader.h`, then removed the two
adapter-backed constructor bodies from `builtin_shader_resources.cpp`.

The generic helpers `construct_shader`, `construct_parameterized_shader`, and
`shader_field` remain in `builtin_shader_resources.cpp` only because the six
unreconstructed built-in shaders still use them. Reassess and remove those
helpers after migrating the final adapters; do not move the completed basic or
error classes back into that shared file.

### Validation and exact handoff state

The SDK 5.500 Debug object build compiled all 169 translation units and created
both the archive and relocatable-link outputs. Neither removed adapter appeared
in `undefined-symbols.txt`. `git diff --check` passed, the IDA database was
saved after the rename and function-boundary work, and the repository was clean
after commit `ab29d14`.

The documentation-only handoff commit `138662f` followed that implementation
commit. Therefore, a later dirty working tree should be treated as new work,
not as residue from the basic/error shader milestone.

## Built-in shader adapters are complete

All 35 built-in shader constructors and every class-specific dispatch are now
source-owned. `builtin_shader_adapters.h` and `builtin_shader_resources.cpp`
were deleted; `builtin_shader_resources.h` still declares the constructors for
the resource manager. The final six live at:

| Shader | Source |
| --- | --- |
| Bloom | `src/render/postprocessing/bloom/RndShaderBloom.cpp` |
| Blur | `src/render/postprocessing/blur/RndShaderBlur.cpp` |
| Output conversion | `src/render/postprocessing/output/RndShaderOutputConversion.cpp` |
| Bink conversion | `src/render/video/RndShaderBinkConvert.cpp` |
| Test pattern, render-test-simple | `src/render/debug/RndShaderTestPattern.cpp`, `src/render/debug/RndShaderRenderTestSimple.cpp` |

### Dispatch tables have 11 slots

The binary's primary-shader dispatch tables have 11 slots, and every source
dispatch now models all of them. See
[render-builtin-shaders.md](render-builtin-shaders.md) for the defaults and
overrides. Several IDA functions had absorbed these small slot leaves.
Codex's earlier error-initializer repair was one of them. Those boundaries
have been corrected.

### Raw-ELF fallback when IDA Python is unavailable

The ELF has no section headers, so `orbis-objdump -d` prints nothing. Cut bytes
from the loadable program segment and disassemble them as raw x86-64 instead:
`orbis-objdump -D -b binary -m i386:x86-64 -M intel --adjust-vma=<start>`.
Dispatch tables are filled by `R_X86_64_RELATIVE` relocations (type `8`), so
read a table by scanning for 24-byte relocation records whose offset falls
inside it; the addend is the slot target. This matched IDA exactly for the
blur table.

## Shader backend loading is complete

`RndShader::_InitShaderCollection` (`0x638430`) and everything below it
are source-owned. That covers the cache validator, the compiled-object loader,
the validation hashes, permutation enumeration, and the core binary and file
streams. See [render-primary-shader.md](render-primary-shader.md) and
[core-io.md](core-io.md).

The remaining boundaries on this path are engine services, declared in their
original class headers:

| Address | Adapter | Notes |
| ---: | --- | --- |
| `0x1AD8B0` | `FileFindGenerated` | Generated-file timestamps and archive mode |
| `0x1AF950` | `FileResolvePath` | Path normalization into a symbol |
| `0x376D40` | `File::NewFile` | File-system open |
| `0x37AA30`, `0x37AAF0` | `MemPushTemp`/`MemPopTemp` | Thread-local heap mode |
| `0x256410` | `IntToStaticString` | Interned small-integer text |
| `0x367C50`, `0x117B560` | `StreamChecksum::Update`, `CSHA1::~CSHA1` | `FileStream` checksums |

The permutation bind (`0x638920`) is now source-owned too. The next rendering
milestones are the per-pass draw functions that build program keys and call
it. Each one belongs in its pass's domain folder:

- blur at `0x634BB0`, a 3.8 KB function with an auto-vectorized Gaussian
  weight loop that needs raw-assembly reading;
- 36 further callers of `RndShader::_SelectShaderCollection`. List them by
  cross-referencing `0x638920` in IDA; several still lack IDA function
  definitions (`0x5F8DF0`, a single-texture draw for a shader outside the built-in set,
  and `0x6F3E80`, the render-test compute dispatch, are now defined but not
  reconstructed).

Bloom's draw function (`0x6346E0`) is done. Use it as the pattern: the shared
helpers in `render/shaders/RndShaderDrawUtl.h` and
`RndShaderResource::Select` cover texture
stamping and binding, constant-buffer selection and commit, and key packing.
The surrounding bloom pass (`0x6305A0`, now defined in IDA) is a 3.7 KB
render-target and state function and a larger, separate milestone.

The final executable link also depends on many engine functions and matching
FMOD libraries. The 585-entry unresolved report is a work queue, not a list of
compile failures. Prioritize dependencies that sit on reconstructed runtime
paths and collapse groups of related adapters rather than adding arbitrary
stubs.

## Source patterns and cautions

- Keep source-owned dispatch tables local to the responsible translation unit
  until a proven shared abstraction exists.
- The primary shader prefix is 288 bytes. Many derived shader fields begin at
  offset 288, but parameter bindings are 20 bytes and can introduce four-byte
  alignment gaps before later 64-bit fields.
- `RndShaderResourceConfig` is 864 bytes with 24 binding arrays and 12 stage
  counters. Texture, unordered output, numeric buffer, and structured buffer
  helpers have distinct array and register semantics.
- Graphics-specific texture binding at `0x6438D0` differs from the general
  stage binding by fixed pixel-stage selection and metadata at offset 40.
- Structured buffer inputs and outputs preserve the structure symbol in the
  third symbol slot and use buffer register counts offset by 12.
- Shader parameter registry indices correspond to shader stages. Preserve the
  exact registry chosen by the binary.
- Constructor initialization must match the binary. A zero is often a
  constant-block extent, while `-1` is commonly an unresolved binding handle.
- The source aims for readable C++ and recovered behavior, not mechanically
  transliterated Hex-Rays output.

## Milestone completion checklist

Before committing a reconstruction milestone:

- Source is placed in the correct project-wide domain.
- Cross-folder includes are source-root-qualified.
- Names and types have concrete IDA evidence.
- Relevant IDA functions are renamed and the database is saved.
- `docs/status.md` and `docs/reconstruction.md` are current.
- The full Debug object build passes.
- Removed adapter symbols do not appear in `undefined-symbols.txt`.
- `git diff --check` passes.
- The commit contains one coherent milestone and leaves a clean tree.

Do not commit `build/`, extracted ELF files, the IDA database, or proprietary
SDK content; `.gitignore` intentionally excludes them.
