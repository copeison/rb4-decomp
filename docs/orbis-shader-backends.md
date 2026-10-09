# Orbis shader backends

The four `PS4ShaderProgram*` classes (`src/renderps4/shaders`) implement the
common `RndShaderProgram` slots: `_CreateImpl(BinStream&)`,
`_SelectImpl(RndContext&)`, `_FreeImpl()` and `_GetTypeImpl()`. The common
destructor calls `Free`, which runs `_FreeImpl` when the program was created.

## Loading

Every `_CreateImpl` follows the same pattern:

1. A `RndShaderCompilerBlob` (`src/render/shaders/RndShaderCompiler.cpp`,
   `0x11B2EE0`-`0x11B2FAF`) reads a 32-bit byte count and the shader binary
   from the stream inside a `MemPushTemp`/`MemPopTemp` scope.
2. `sce::Gnmx::parseShader` (`parseGsShader` for geometry) splits the binary
   into the shader header and its GPU code.
3. The code is copied into a 256-byte-aligned allocation from the `"gpu"`
   heap (`MemFindHeap` in a function-local static, then `MemPushHeap` and
   `MemPopHeap`); the header is copied into a 4-byte-aligned allocation sized
   by the header's `computeSize()`.
4. The header's `patchShaderGpuAddress` stores the code address, and the
   blob is freed.

The allocation names are `CShader`, `PShader`, `GShader` and `VShader`.

| Class | Fields (offset) | `_SelectImpl` |
| --- | --- | --- |
| `PS4ShaderProgramCompute` | `CsShader*` 40, header 48, code 56 | Graphics pipe: `GfxContext::setCsShader`. Compute pipe: `ComputeContext::setCsShader`, then `RndContext::_ReselectGlobalCBuffers`. |
| `PS4ShaderProgramPixel` | `PsShader*` 40, header 48, code 56 | `GfxContext::setPsShader`, then `PS4Context::SetCbEnabled(true)` |
| `PS4ShaderProgramGeometry` | `GsShader*` 40, header 48, GS code 56, copy-shader code 64 | `GfxContext::setGsVsShaders` |
| `PS4ShaderProgramVertex` | `VsShader*` 40, `EsShader*` 48, their headers 56/64, code 72, shader modifier 80, VS/ES fetch shaders 88/96 | See below |

Every `_FreeImpl` defers the owned allocations through
`PS4Device::DeferredDelete` and clears only the typed shader pointers. The
compute constructor initializes nothing; the others clear their pointers.

## Vertex shaders

One compiled vertex shader serves as both the VS-stage shader and, when a
geometry shader is active, the ES-stage shader. `_CreateImpl` (`0x8E4790`)
copies the parsed header twice (as `VsShader` and `EsShader`) and patches the
same code address into both. It then builds a
`sce::Gnm::FetchShaderInstancingMode` table as long as the larger of the two
input-semantic counts: one `kFetchShaderUseVertexIndex` entry for each input
semantic below 8 (the per-vertex streams) and `kFetchShaderUseInstanceId` for
the rest. `generateVsFetchShader` and `generateEsFetchShader` build the two
fetch shaders from it into `"gpu"`-heap allocations.

`_SelectImpl` (`0x8E4D80`) tests the geometry bit of
`RndContext::mActiveShaderStages`. With a geometry shader it clears the VS
stage and binds the ES variant; otherwise it binds the VS variant and clears
the ES stage.

## Binary and SDK

The game was built against SDK 2.500, and the reconstruction compiles against
5.500. The inline SDK bodies still match the binary:
- `GfxContext::setVsShader` refreshes the CUE's input-parameter cache
  (`ConstantUpdateEngine::initializeInputsCache`) when the shader changes,
  then calls the cached `setVsShader`.
- `setEsShader` calls `setOnChipEsShader` with an LDS size of 0.
- `ComputeContext::setCsShader` generates the input-resource offset table
  and calls `ComputeConstantUpdateEngine::setCsShader`.
