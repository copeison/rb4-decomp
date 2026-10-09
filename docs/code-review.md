# Source review and map cross-reference

A full read of `src/` (406 files, about 40k lines) in six slices. It was
cross-referenced against `files/rockband_ps4_r.map`, a linker map from an
**older** release build. Its object paths look like
`D:\rb4_release_candidate\main\rockband\build\zmake\temp\ORBIS_Release\<module>\<Class>.o`.
Map addresses do not match the current binary, so use the map for class,
method and module names only, and confirm behaviour in IDA.

## Original module layout

The original tree is flat by module, with one class per object file:

| Original module | Objects | Our location |
|---|---:|---|
| `render` (`Rnd*`) | 321 | `src/render/core`, `resources`, pass folders |
| `renderps4` (`PS4*`) | 24 | `src/render/platform/orbis` |
| `utl` (BinStream, FileStream, Symbol, Thread, DataArray, …) | 54 | `src/core/io`, `threading`, `types` |
| `os` (File, MemMgr, Joypad, PlatformMgr, …) | 97 | `src/core`, `src/input` |
| `math` (Rand2, Color, SHA1, Vec, Matrix, …) | 42 | `src/core/random`, `color` |
| `audio` | 115 | `src/audio` |
| `ui` | 16 | `src/ui` |
| `rb_game`, `rb_meta`, `rb_world`, `rb_track`, `rb_beatmatch`, `rb_gtrsolo`, `rb_audio` | 448 | `src/game` |

## Confirmed defects

These were checked against the binary in IDA.

- **Missing resource registration.** The binary adds every target it creates
  to the owner's registered-resource list; the source registers none.
  - Sky targets: 0x6B0E80, source `render/sky/sky_targets.cpp`.
  - Scaled targets: 0x6B12D0, source `render/intermediate/scaled_targets.cpp`.
  - Light-accumulation targets: 0x6B0B20, source
    `render/lighting/accumulation/light_accumulation_targets.cpp`.
- **CMAA color target.** The binary registers it only when it is non-null;
  `render/lighting/.../cmaa_targets.cpp` registers it unconditionally.
- **Light-accumulation factory.** It takes a signed attachment selector and a
  target-flags argument. The source narrows the first to `bool` and hard-codes
  the second.
- **Wrong address label.** *(Fixed in the render-device conversion.)*
  `render_system_release_builtin_buffers` was labelled 0x3DDE60, which is
  `RndDevice::Terminate`; the release is inlined there through
  `RndShaderCBuffer::SafeDelete`.
- **Mislabelled FMOD constant.** *(Fixed in the audio conversion.)*
  `FMOD_CHANNELCONTROL_DSP_HEAD = -3` is really
  `DSP_TAIL`, so the clip DSP is inserted at the tail.
- **Cache "defines" are include checksums.** The records read by the cache
  loader are `RndShaderIncludeChecksums` (`Symbol` + checksum), not global
  defines.
- **Texture slot 13 has two names.** The base table calls it `update_gpu_data`,
  while every subclass calls it `release_source_data`. The map confirms the
  subclass name (`_FreePixelDataImpl`).

## Structural issues

- **Dispatch tables are type-punned.** The 11-slot primary-shader dispatch,
  and the `shader_field` / `primary_shader` helpers, are re-declared in about
  24 files, some with `void*` parameter types. They should come from
  `render/resources/shaders/primary_shader_dispatch.h`.
- **Target-file helpers are duplicated.** `divide_round_up`, `target_slot`,
  `reusable_target`, `release_target` and the registration push are each
  copied into 6-12 `*_targets.cpp` files.
- **Copy-pasted mesh files.** Seven Orbis mesh files differ only in vertex
  type; they were one template (`PS4MeshTyped<T>`).
- **Copy-pasted generator pools.** Five audio generator managers copy the same
  pool code; the original shares a base (`_InitGeneratorPool`, `GetIndex`, …).
- **Duplicate layouts for one object.** `RenderTargetState` and
  `RenderTargetResources` describe the same object.
  `TiledLightTargetResources` duplicates `RenderTargetResourceBlock`.
- **`RenderSystem` layout lives in raw offsets.** *(Fixed: `RndDevice` and
  `PS4Device` are now classes with asserted layouts; see
  [render-device.md](render-device.md).)*
- **Layer inversions.**
  - *(Fixed.)* `render/core/system` included `game/startup/system_init_options.h`,
    which is really `RndInitParams`; it is now declared in `RndDevice.h`.
  - `core/io` uses `render/.../render_resource_name.h`, which is really the
    engine `String` and belongs in `core/types`.
  - *(Fixed in the audio conversion.)* `Vector3` and the transform type were
    defined in an audio header; they are now in `src/math`.
- **Platform-neutral code under `orbis`.** The mesh-format functions at
  0x4429xx-0x4438xx and the resource-barrier types belong in `render/core`.
- **Unbounded writes into fixed inline storage.** Affected:
  - target-resource blocks and registered resources;
  - frame owners;
  - `resources[12]`.

  Check these for a growth path in the binary.

## Original names

| Reconstruction | Original |
|---|---|
| `RenderSystem` | `RndDevice` (backend `PS4Device`) |
| `RenderContext` | `RndContext` (backend `PS4Context`) |
| `RenderPrimaryShaderResource` | `RndShader` |
| Shader dispatch slots 0-10 | `~RndShader` ×2, `_GetClassNameImpl`, `_GetShaderFilePath`, `_InitConfigImpl`, (inline, returns 0), `_GetShaderStages`, `_UsesShaderKeyImpl`, `_SelectErrorShader`, `_SupportsRTSlicing`, `_UsesCustomGeometryShader` |
| `render_primary_shader_initialize_backend` | `RndShader::_InitShaderCollection` |
| `render_primary_shader_load_cache` | `RndShader::_LoadCached(const char*, bool)` |
| `render_primary_shader_bind` | `RndShader::_SelectShaderCollection(RndContext&, RndShaderKeyGroup&)` |
| compiled-object arrays | `RndShaderCollection` |
| parameter registry | `RndShaderDefines` / `RndShaderDefinesGroup` |
| constant registry | `RndShaderFixedDefines` |
| constant block | `RndShaderCBufferConfig` |
| backend state | `RndShaderResourceConfig` |
| `RenderResourceManager` | `RndShaderMgr` |
| `DefaultRenderResources` | `RndDefaults` |
| `render_<x>_shader_draw` | `RndShader<X>::Select(RndContext&, …)` |
| compute shaders | `RndCShader<X>::Dispatch` |
| `RenderTargetResources` | `RndBufferCollection` |
| `RenderTargetResourceBlock` | `RndBufferCollection::FrameIntervalBuffers` |
| `render_*_targets_create` | `RndBufferCollection::_Alloc*Buffers` |
| `RenderFrameOwner` | `RndWindow` |
| `RenderPlatformConfig` | `RndCapabilities` |
| `RenderGpuStatBlock` | `RndGpuStatsMgr` |
| `RenderShader` | `RndShaderProgram` (backend `PS4ShaderProgram*`) |
| `RenderConstantBuffer` | `RndShaderCBuffer` |
| `RenderComputeBuffer` | `RndComputeBuffer` |
| `RenderTexture*` | `RndTextureBase`, `RndTexture2D`, … (backend `PS4Texture*`) |
| `RenderMesh` | `RndMesh` (backend `PS4MeshTyped<T>`) |
| `RandomGenerator` | `Rand2` |
| `EngineFile` | `File` |
| `EngineThread` | `Thread` |
| `AudioClipFmod` | `FmodAudioBusGenerator` |
| `FmodAudioState` | `FModSystem` |
| `FmodAudioInputManager` | `MicHwManager_FMOD` |
| `UiLayoutId` | `RBUILayout` |
| `game_initialize` / `game_run_frame` | `App::Initialize` / `App::RunOneFrame` |
