# Render debug commands

`RndCommands::Init` at `0x6BB0E0` registers 24 console commands.
The clean reconstruction preserves their exact spelling and registration order:

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

Sixteen handlers are now source-owned. Twelve directly toggle their typed
`RndConfig` byte, partial-framerate toggling also enforces the nonzero
scene-limit gate, the screenshot handlers publish the pending request and
advance the six-mode setting, and HDR normalizes the render-system mode at
offset `0x68` between zero and one.

`reload_shaders` now enters the typed render resource manager directly. It
releases the six compiled-object arrays for every primary shader resource and
marks the applicable secondary resources for refresh.
