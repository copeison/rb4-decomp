# Renderer settings

`RndConfig::RndConfig` at `0x6BB470` constructs the 232-byte renderer
configuration block, reads the `rnd` data section, applies platform capability
rules, and processes the command-line `resolution` override.

The render-system lifecycle now allocates this exact typed size through the
renderer allocator and releases the same block directly. The settings pointer
at render-system offset `0x128` is cleared immediately during destruction.

The principal defaults recovered from its constant stores are:

| Setting | Default |
| --- | --- |
| Content and output resolution | 1,920 × 1,080 |
| Initial PC window resolution | 1,280 × 720 |
| Fullscreen | false |
| Vsync enabled | true |
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

The constructor reads the block with `SystemConfig("rnd")`. It then uses
`DataArray::FindArray` and the `FindData` overloads for booleans, integers
and symbols, all with failure off.
- **Resolutions.** Each resolution is the second and third node of its
  array.
- **64-bit limits.** These are read as 32-bit integers and sign-extended.
- **Partial frame rate.** It is enabled as soon as
  `max_partial_framerate_scenes` is read.
- **Quality level.** It changes only when `quality_level` names a symbol.

The nested blocks for graphics API validation, the graphics debugger, barrier
validation and shader compilation are read without a null check. If the
current platform slot's feature bit `0x10` supports async compute and that
option is enabled, multithreaded rendering is disabled. Platforms without the
feature force both async compute and tiled lighting off. This capability test
now reads the typed platform configuration directly.

The numeric `vsync_mode` loaded from configuration is separate from the
runtime `vsync_enabled` flag. `toggle_vsync` at `0x6BA880` flips the latter;
its default is true. The exact structure places those fields at `0x14` and
`0x98`, matching the submit worker directly.

Five values look like 32-bit settings at the config boundary but occupy signed
64-bit slots in memory. These are the two scene/buffer limits at `0x68` and
`0x70`, followed by the geometry, lighting, and light-probe overdraw limits at
`0xC0`, `0xC8`, and `0xD0`. The original reads a 32-bit value and sign-extends
it into each slot.

The shader generator and render-target allocation paths identify most of the
other 64-bit defaults:

| Offset | Field | Default |
| --- | --- | --- |
| `0x20` | Light tile size | 32 |
| `0x28` | Light tile depth slices | 8 |
| `0x30` | Volumetric-scattering tile size | 16 |
| `0x38` | Maximum lights per tile | 256 |
| `0x40` | Maximum point lights | 256 |
| `0x48` | Maximum spotlights | 32 |
| `0x50` | Maximum directional lights | 16 |
| `0x58` | Maximum light probes | 128 |
| `0x78` | Shadow-softening tile size | 16 |
| `0x80` | Mask tile size | 16 |

The byte at `0x60` retains an offset-based name until its consumers establish
its meaning. Its exact default and all padding are preserved. The render-target
mask allocation at `0x6B1760` identifies the `0x80` value: it divides the
target width and height into 16-pixel tiles.

The startup `-resolution` switch replaces both the output resolution and the
initial window resolution and sets the explicit-override flag. The default is
the final mode in platform slot seven's sorted resolution vector. The original
startup code checks that default against the advertised list before applying
the parsed override; it does not check the parsed extent itself. The
reconstruction preserves this observed behavior.

`ParseResolution` accepts `WIDTHxHEIGHT` using a lowercase `x`. A
single positive number is treated as the height and expanded to a 16:9 width;
for example, `1080` becomes 1,920 × 1,080. The separate runtime
`set_resolution` command only enables the override when the resulting extent
appears in the platform's advertised mode list. Calling the command without a
value clears the override.

Quality levels are the case-insensitive names `Low`, `Medium`, and `High`, with
numeric values zero through two. Unknown names produce the invalid value `-1`.

## Platform configuration

Three more readers take their data from the system configuration:
- **`RndCapabilities::InitForPlatform` (`0x6B99B0`)** parses
  `rnd <platform> resolutions` into its `eastl::vector<Vector2i>` with
  `ParseResolution`. If none parse it falls back to 1920 x 1080, then sorts
  the list.
- **`RndGfxApiForPlatform` (`0x4414A0`)** compares the symbol in
  `rnd <platform> api` with each `GfxApiSymbol`.
- **`GetSupportedPlatforms` (`0x3641B0`)** returns the integers of
  `platform_mgr supported_platforms` as an `eastl::vector<int>`.

`PlatformSymbol` and `GfxApiSymbol` return `Symbol`s from static tables, and
the empty symbol out of range.

The resolution override has a quirk: the `-resolution` option is accepted
when the current output resolution is supported, not the requested one.
