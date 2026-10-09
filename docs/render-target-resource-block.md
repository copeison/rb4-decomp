# Render-target resource blocks

`RndBufferCollection::_AllocFrameIntervalBuffers` at `0x6B2660` populates one
216-byte per-scene resource block. The primary block registers its targets
with the enclosing owner. Secondary partial-frame blocks allocate an 80-byte
state object at block offset zero and retain their resources through the
block.

The initializer dispatches these resource flags in binary order:

| Flag | Resource group |
| ---: | --- |
| `0x0002` | Depth/stencil target |
| `0x0040` | GBuffer targets |
| `0x0004` | Linear and tiled-depth targets |
| `0x0800` | Ambient-occlusion target |
| `0x0020` | Per-target tiled-light resources |
| `0x1000` | Volumetric-scattering textures |

Flag `0x0008` also creates the partial light-accumulation target for secondary
blocks. The primary light-accumulation targets belong to the enclosing owner
and are created earlier by `RndBufferCollection::InstallBackBuffer`.

The primary tiled-light block creates its interpolation target. It first tries
the corresponding target from a reusable block; if that is absent, it can
reuse the primary shadow-contribution scratch target when its dimensions are
large enough. Secondary blocks skip the interpolation target. Owner mode three
adds the stereo tiled-light resource set.
