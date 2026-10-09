# Shader classes

The built-in shaders are classes deriving from `RndShader`
(`src/render/shaders/RndShader.{h,cpp}`). Compute shaders derive from
`RndShaderCompute`. Each shader describes its permutation defines, constant
buffer, and resources once. It then loads the compiled program for every
permutation from the shader cache, and selects programs by key when it
draws.

Class and method names come from the reference map's `render` objects
(`RndShader.o`, `RndShaderDefines.o`, `RndShaderCBufferConfig.o`,
`RndShaderResourceConfig.o`, `RndShaderCollection.o`, and one object per
shader). Each built-in shader's class name is also the string its
`_GetClassNameImpl` returns. Field names are reconstructions.

## `RndShader` virtual slots

The base vtable is at 0x192EF60.

| Slot | Method | Base |
| --- | --- | --- |
| 0, 1 | destructors | 0x638110, 0x638250 |
| 2 | `_GetClassNameImpl() const` | pure |
| 3 | `_GetShaderFilePath() const` | pure |
| 4 | `_InitConfigImpl(RndShaderFixedDefines&, RndShaderDefinesGroup&, RndShaderCBufferConfig&, RndShaderResourceConfig&)` | pure |
| 5 | `_GetLoadOption() const` (name not in the map) | 0x450850, returns 0 |
| 6 | `_GetShaderStages() const` | 0x6388E0, returns 13; `RndShaderCompute` returns 16 (0x63C220) |
| 7 | `_UsesShaderKeyImpl(RndShaderProgramType, RndShaderKey) const` | 0x6388F0, returns true |
| 8 | `_SelectErrorShader(RndContext&) const` | 0x638900 |
| 9 | `_SupportsRTSlicing() const` | 0x450870, returns false |
| 10 | `_UsesCustomGeometryShader() const` | 0x450880, returns false |

Slot 5 picks the startup option that allows loading: 0 means the rendering
option, 1 means the third option. No shader overrides it.

## Lifecycle

| Address | Method | Role |
| --- | --- | --- |
| 0x6380B0 | constructor | |
| 0x638270 | `InitConfig` | Creates the four configuration objects, adds the global `HX_NUM_RT_SLICES` define, and calls slot 4 |
| 0x6383D0 | `Init` | `InitConfig`, then loads the programs when the startup option allows |
| 0x638430 | `_InitShaderCollection` | Finds the generated cache and loads it under a recursive `CritSec`; a stale cache falls back to an unvalidated load |
| 0x638A40 | `_LoadCached` | Reads the cache header, its four checksums, and the include checksums, then loads the programs |
| 0x638D70 | `_ChecksumDefines` | Hashes the fixed defines, every define, and every valid permutation key |
| 0x638920 | `_SelectShaderCollection` | Writes the render-target slice count into the keys and selects the programs |
| 0x638A20 | `_Register` | Links the shader into the shader manager and initializes it when the manager already has |
| 0x6388C0 | `Reload` | Frees the programs so the next select reloads them |

## Configuration classes

- **`RndShaderFixedDefines`** (0x63D520-0x63D6E0). Holds `#define` lines
  and comments emitted ahead of the shader source.
- **`RndShaderDefines`** (0x63C3F0, 0x63C550). Holds one program type's
  permutation defines. `Add` returns the `RndShaderDefInfo` that locates
  the define in a key. Global defines use the key's high 32 bits.
- **`RndShaderDefinesGroup`**. Holds the global defines and one
  `RndShaderDefines` per program type. Its `Visit` (0x63D100, 0x63C8F0)
  enumerates every permutation.
- **`RndShaderCBufferConfig`** (0x63A0C0-0x63A8C0). Describes the constant
  buffer layout in 16-byte registers.
- **`RndShaderResourceConfig`** (0x643260-0x644E40). Holds the declared
  textures, samplers, and buffers, per program type.
- **`RndShaderCollection`** (0x63B2B0, 0x63B500). Holds the compiled
  programs per program type, sorted by key.

The `PrintCode` methods write into the engine's hashing text stream. That
stream is modeled by its running FNV-1a hash, which the cache checksums
compare.

## Earlier names

| Earlier reconstruction | Original |
| --- | --- |
| `RenderPrimaryShaderResource` | `RndShader` |
| `render_primary_shader_prepare`, `_finalize` | `InitConfig`, `Init` |
| `render_primary_shader_initialize_backend` | `_InitShaderCollection` |
| `render_primary_shader_layout_hash` | `_ChecksumDefines` |
| `render_primary_shader_bind` | `_SelectShaderCollection` |
| `render_primary_shader_register` | `_Register` |
| `render_primary_shader_clear_compiled_objects` | `Reload` |
| dispatch slots `source_identifier`, `backend_path`, `initialize_support_objects`, `mode`, `variant`, `validate_permutation`, `bind_fallback`, `supports_render_target_slices`, `uses_geometry_program` | slots 2-10 above |
| `RenderShaderConstantRegistry` | `RndShaderFixedDefines` |
| `RenderShaderParameterRegistry`, `...Set`, `...Binding` | `RndShaderDefines`, `RndShaderDefinesGroup`, `RndShaderDefInfo` |
| `RenderShaderConstantBlock`, `RenderShaderConstantType` | `RndShaderCBufferConfig`, `RndShaderNumericType` |
| `RenderShaderBackendState` | `RndShaderResourceConfig` |
| `RenderManagedObjectArray[6]` | `RndShaderCollection` |
| program key arrays | `RndShaderKeyGroup` |
| `FogDeferredShaderResource` | `RndShaderFogDeferred` |
