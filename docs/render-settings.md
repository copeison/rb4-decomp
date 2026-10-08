# Renderer settings

`render_settings_initialize` at `0x6BB470` constructs the packed renderer
configuration block, reads the `rnd` data section, applies platform capability
rules, and processes the command-line `resolution` override.

The principal defaults recovered from its constant stores are:

| Setting | Default |
| --- | --- |
| Content and output resolution | 1,920 × 1,080 |
| Initial PC window resolution | 1,280 × 720 |
| Fullscreen | false |
| LOD | true |
| GBuffer vertex normals | true |
| 64-bit light accumulation | false |
| 40-bit depth/stencil | true |
| Tiled lighting | false |
| Quality level | `Medium` (`1`) |
| Scene mask, shadows, post-processing, tone mapping, volumetric scattering | true |
| Multithreaded rendering, async compute, async copy | true |
| Stereo optimizations | true |
| Screenshot resolution | current target |
| Geometry, lighting, light-probe overdraw limits | 10, 20, 10 |
| Break on graphics error | true |

The loader reads nested settings for graphics API validation, a graphics
debugger, barrier validation, and shader-compilation diagnostics. If the
platform supports async compute and that option is enabled, multithreaded
rendering is disabled. Platforms without async-compute support force both
async compute and tiled lighting off.

A valid command-line `resolution` value must match one of the platform's
advertised modes. When it does, it replaces both the output resolution and the
initial window resolution and sets the explicit-override flag.

Quality levels are the case-insensitive names `Low`, `Medium`, and `High`, with
numeric values zero through two. Unknown names produce the invalid value `-1`.
