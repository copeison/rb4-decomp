# Render debug commands

`RndCommands::Init` at `0x6BB0E0` registers 24 console commands with
`DataRegisterFunc`, in this order. Each handler is a script function,
`DataNode (DataArray*)`, and returns the integer 0.

| Command | Purpose |
| --- | --- |
| `toggle_overlay` | Toggle the renderer diagnostic overlay. |
| `overlay_help` | Show overlay help. |
| `reload_shaders` | Reload renderer shaders. |
| `set_resolution` | Select the output resolution. |
| `set_quality_level` | Select the renderer quality preset. |
| `toggle_vsync` | Toggle vertical synchronization. |
| `toggle_scene_mask` | Toggle scene-mask generation. |
| `toggle_shadows` | Toggle shadows. |
| `toggle_postproc` | Toggle post-processing. |
| `toggle_tonemapping` | Toggle tone mapping. |
| `toggle_vscat` | Toggle volumetric scattering. |
| `set_drawn_scene_range` | Restrict the inclusive scene-index range. |
| `toggle_multithreaded_rendering` | Toggle multithreaded rendering. |
| `toggle_async_compute` | Toggle asynchronous compute. |
| `toggle_async_copy` | Toggle asynchronous copies. |
| `toggle_tiled_light_interpolation` | Toggle tiled-light interpolation. |
| `toggle_partial_framerate` | Toggle partial-rate rendering when supported. |
| `toggle_stereo_optimizations` | Toggle stereo optimizations. |
| `toggle_64_bit_light_accum` | Toggle the 64-bit light-accumulation path. |
| `toggle_hdr` | Toggle HDR output. |
| `take_screenshot` | Set the pending screenshot flag. |
| `cycle_screenshot_resolution` | Advance through six screenshot modes. |
| `set_shading_mode` | Set the material draw-debug mode. |
| `set_buffer_inspection_mode` | Set the buffer debug view. |

The screenshot resolution command wraps modulo six, covering the current
target plus the five fixed dimensions documented in `docs/screenshot-capture.md`.
The final two commands accept a mode name, and accept `help` to enumerate their
respective name tables.

Twenty-one handlers are source-owned. The two overlay commands need
`RndOverlay`, and `set_quality_level` formats the quality-level names into a
string this build discards; those three are not reconstructed yet.

- **`set_resolution`** takes a width and a height, a `WxH` string
  (`ParseResolution`), or a height with a 16:9 width. It overrides the
  output resolution only when the PS4 capabilities list it. Without an
  argument it clears the override.
- **`set_drawn_scene_range`** resets the range to 0 through -1, then takes the
  first and last scene from its arguments.
- **`set_shading_mode` and `set_buffer_inspection_mode`** apply to the main
  window through `_SetShadingMode` (`0x6BACB0`) and
  `_SetBufferInspectionMode` (`0x6BAF80`). These evaluate the argument and
  accept a string or symbol name, or an integer. `help` walks the name table,
  which this build does not print, and an unknown name leaves the mode
  unchanged.

The other handlers: Twelve directly toggle their typed
`RndConfig` byte, partial-framerate toggling also enforces the nonzero
scene-limit gate, the screenshot handlers publish the pending request and
advance the six-mode setting, and HDR normalizes the render-system mode at
offset `0x68` between zero and one.

`reload_shaders` now enters the typed render resource manager directly. It
releases the six compiled-object arrays for every primary shader resource and
marks the applicable secondary resources for refresh.
