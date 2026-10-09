# Reconstruction handoff

## Snapshot

This document describes the repository at commit
`ab29d144623b692131bf4bdb78ea8b164bd692b8` on branch `main`, after the
`decomp: reconstruct basic shaders` milestone. The working tree was clean when
the snapshot was taken.

The current PS4 object build compiles **169 C++ translation units**. It creates
a complete relocatable object and archive, but it does not yet produce a game
executable. The latest unresolved-symbol report contains 590 unique entries,
covering engine code that has not been reconstructed, external runtime APIs,
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
  bank/resource handling, listener state, and timing reports.
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

The IDA database was saved after each associated rename pass.

## Last active milestone in depth

The last implementation milestone was the reconstruction of the basic and
error fallback shaders in commit `ab29d144623b692131bf4bdb78ea8b164bd692b8`
(`decomp: reconstruct basic shaders`). That work is complete; there was no
partially edited source file or uncommitted IDA rename at the stopping point.
The next shader milestone should start with one of the six adapters listed in
the next section rather than revisiting these two classes without new binary
evidence.

### Shared object and dispatch layout

Both classes inherit the reconstructed 288-byte
`RenderPrimaryShaderResource` prefix. Their class-specific parameter bindings
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
`src/render/resources/shaders/basic_shaders.cpp`. The shared binary leaf
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
`src/render/resources/shaders/basic_shaders.cpp`. It removed
`render_error_shader_install_dispatch` and
`render_basic_shader_install_dispatch` from
`src/render/resources/shaders/builtin_shader_adapters.h`, then removed the two
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

## Immediate remaining shader work

Six built-in dispatch adapters remain in
`src/render/resources/shaders/builtin_shader_adapters.h`:

| Constructor | Address | Recommended domain |
| --- | ---: | --- |
| `render_bink_convert_shader_construct` | `0x5F4980` | `src/render/resources/video` |
| `render_bloom_shader_construct` | `0x634640` | `src/render/postprocessing/bloom` |
| `render_blur_shader_construct` | `0x634AE0` | `src/render/postprocessing/blur` |
| `render_output_conversion_shader_construct` | `0x6367A0` | `src/render/postprocessing/output` |
| `render_test_pattern_shader_construct` | `0x6452E0` | `src/render/core/debug` |
| `render_test_shader_construct` | `0x642500` | `src/render/core/debug` |

For each one:

1. Decompile the constructor and recover its dispatch-table address.
2. Inspect all seven dispatch entries and define missing leaf functions only
   after verifying their boundaries.
3. Reconstruct a source-owned dispatch and exact constructor state.
4. Reuse or extend `shader_backend_state` only when IDA proves a new binding
   layout. Do not approximate a helper with a superficially similar one.
5. Remove the matching declaration and constructor body from the shared
   adapter/constructor files.
6. Rename the verified IDA methods and save the database.
7. Compile directly for quick feedback, then run the full 5.500 build.
8. Confirm the removed adapter is absent from `undefined-symbols.txt`, update
   both tracking documents, and make a focused commit.

After these six, remove any helper in `builtin_shader_resources.cpp` that has
become unused and verify that `builtin_shader_adapters.h` can be deleted or
reduced to genuinely unresolved behavior.

## Important deeper blocker

`render_primary_shader_initialize_backend` at `0x638430` remains unresolved.
Its helper near `0x638A40` participates in compiled-shader cache loading,
source-hash and platform validation, and backend object construction. This is
central runtime behavior and should not be replaced with a shallow success
stub. Finish the small built-in dispatches first, then reconstruct this path
from its callers, cache record layout, resource arrays, and platform shader
factories.

The final executable link also depends on many engine functions and matching
FMOD libraries. The 590-entry unresolved report is a work queue, not a list of
compile failures. Prioritize dependencies that sit on reconstructed runtime
paths and collapse groups of related adapters rather than adding arbitrary
stubs.

## Source patterns and cautions

- Keep source-owned dispatch tables local to the responsible translation unit
  until a proven shared abstraction exists.
- The primary shader prefix is 288 bytes. Many derived shader fields begin at
  offset 288, but parameter bindings are 20 bytes and can introduce four-byte
  alignment gaps before later 64-bit fields.
- `RenderShaderBackendState` is 864 bytes with 24 binding arrays and 12 stage
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
