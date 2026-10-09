# Render buffer collections

`RndBufferCollection` (`src/render/targets/RndBufferCollection.{h,cpp}`)
owns a view's render targets. It holds the back buffer and every
intermediate buffer the passes draw into. `RndBufferCollection2D`
(`RndBufferCollection2D.cpp`) allocates them as 2D textures and 2D texture
arrays. A window owns or presents one collection (see
[render-window.md](render-window.md)).

The class and method names come from the reference map's
`RndBufferCollection.o` and `RndBufferCollection2D.o`. Field names, the flag
names, and the scene-mask tile allocator's name are reconstructions.

## Virtual slots

| Slot | Method | Base (0x1936DB8) | 2D (0x1936DF0) |
| --- | --- | --- | --- |
| 0, 1 | destructors | 0x6AFFC0, 0x6B0730 | 0x6B40D0, 0x6B40E0 |
| 2 | `_ValidateBackBufferImpl(const RndTextureBase&)` | pure | 0x6B4100 |
| 3 | `_AllocBufferImpl(name, format, dataFormat, size, attachment, flags, reuse)` | pure | 0x6B4120 |
| 4 | `_AllocBufferArrayImpl(name, format, dataFormat, size, count, attachment, flags, reuse)` | pure | 0x6B41F0 |

Every allocator calls slot 3 or 4 through the vtable. This build adds the
data format, attachment index, and reused texture to the map's signatures.

## Methods

| Address | Method | Buffers |
| --- | --- | --- |
| 0x6AFEA0 | constructor | Inline storage for 38 registered buffers and 4 frame intervals |
| 0x6AFFE0 | `Destroy` | Everything, in a fixed order |
| 0x6B0760 | `InstallBackBuffer` | Runs the allocators selected by the flags, then propagates the target mode |
| 0x6B0B20 | `_AllocLightAccumBuffers` | Two light-accum buffers and three blurred ones |
| inlined | `_AllocLightProbeAccumBuffer` | Light-probe accumulation, unless tiled lighting is on |
| 0x6B0E80 | `_AllocAtmosphereBuffers` | Four sky buffers |
| 0x6B12D0 | `_AllocDownsampleBuffers` | Half-, quarter- and eighth-size buffers, two of each |
| 0x6B1760 | `_AllocSceneMaskBuffer` | Mask, scratch and tile buffers |
| 0x6B1970 | `_AllocShadowBlurBuffers` | Contribution array, stencil, scratch and soften tiles |
| 0x6B1E60 | `_AllocCMAABuffers` | Color, two edge buffers and the compressed edges |
| 0x6B2140 | `_AllocSceneMaskTileBuffers` | Two tile masks and the tile quad mesh |
| 0x6B2660 | `_AllocFrameIntervalBuffers` | One `FrameIntervalBuffers` set |
| 0x6B2A80 | `_AllocDepthStencilBuffer` | Depth/stencil |
| 0x6B2C90 | `_AllocLinearDepthBuffer` | Linear depth and the tiled depth range |
| 0x6B2E80 | `_AllocOneLightAccumBuffer` | One light-accum buffer |
| 0x6B3040 | `_AllocGBuffer` | Color, pixel normals, and optional vertex normals |
| 0x6B3270 | `_AllocAOBuffers` | AO; also inlined into 0x6B2660 |
| 0x6B3380 | `_AllocTiledLightingBuffers` | Tiled light ids and ranges, the interpolation buffer |
| 0x6B3730 | `_AllocVolumetricScatteringBuffers` | Inscattering and accumulated scattering volumes |
| 0x6B28D0 | `SetTargetMode` | Writes the mode into every registered buffer |
| 0x6B2910 | `ObtainPartialFramerateData` | Creates missing partial-framerate intervals |
| 0x6D18A0 | `RndScenePartialFramerateData::RndScenePartialFramerateData` | Per-interval partial-framerate state, in its own `RndScenePartialFramerateData.cpp` as in the map |
| 0x6B2A20 | `SelectPartialFramerateBuffers` | Selects the active interval |

## Corrections made during the conversion

- **Missing registrations.** The light-accum, sky, and downsample buffers are
  now registered, as the binary does. Without registration, `SetTargetMode`
  never reached them.
- **CMAA color buffer.** It is registered only when non-null. The color
  buffer exists only when the previous collection had one.
- **Light-accum allocator.** `_AllocOneLightAccumBuffer` takes a signed
  attachment selector (a non-negative value takes the next attachment slot)
  and a target-flags argument, as in the binary.
- **Virtual dispatch.** The allocators call slots 3 and 4 instead of calling
  the 2D implementation directly.
- **Signed sizes.** Sizes are `Vector2i`, matching the binary's signed
  halving and clamping.
- **One translation unit.** The 17 per-buffer source files are merged into
  `RndBufferCollection.cpp`, matching the map's object. Their duplicated
  helpers are replaced by shared ones.

## Earlier names

| Earlier reconstruction | Original |
| --- | --- |
| `RenderTargetResources`, `RenderTargetState` | `RndBufferCollection` |
| `RenderTargetResourceBlock` | `RndBufferCollection::FrameIntervalBuffers` |
| `RenderPartialFrameState`, `RndBufferCollection::PartialFramerateData` | `RndScenePartialFramerateData` |
| `render_target_resources_initialize` | `InstallBackBuffer` |
| `render_target_resources_release` | `Destroy` |
| `render_target_resources_set_resource_mode` | `SetTargetMode` |
| `render_target_resources_acquire_partial_frame_state` | `ObtainPartialFramerateData` |
| `render_target_resources_select_partial_frame` | `SelectPartialFramerateBuffers` |
| `render_target_resource_block_initialize` | `_AllocFrameIntervalBuffers` |
| `render_light_accumulation_targets_create` | `_AllocLightAccumBuffers` |
| `render_light_accumulation_target_create` | `_AllocOneLightAccumBuffer` |
| `render_sky_targets_create` | `_AllocAtmosphereBuffers` |
| `render_scaled_targets_create` | `_AllocDownsampleBuffers` |
| `render_scene_mask_targets_create` | `_AllocSceneMaskBuffer` |
| `render_shadow_contribution_targets_create` | `_AllocShadowBlurBuffers` |
| `render_cmaa_targets_create` | `_AllocCMAABuffers` |
| `render_scene_mask_tiles_create` | `_AllocSceneMaskTileBuffers` |
| `render_depth_stencil_target_create` | `_AllocDepthStencilBuffer` |
| `render_linear_depth_targets_create` | `_AllocLinearDepthBuffer` |
| `render_gbuffer_targets_create` | `_AllocGBuffer` |
| `render_ambient_occlusion_target_create` | `_AllocAOBuffers` |
| `render_tiled_light_target_buffers_create` | `_AllocTiledLightingBuffers` |
| `render_volumetric_scattering_textures_create` | `_AllocVolumetricScatteringBuffers` |
| `render_target_resources_create_texture_2d`, `..._array_2d` | `RndBufferCollection2D::_AllocBufferImpl`, `_AllocBufferArrayImpl` |
