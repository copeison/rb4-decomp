# Tiled-light buffers

`RndLightMgrCom::_InitBuffers` at `0x48A400` creates the shared
compute buffers used by tiled lighting when that renderer feature is enabled.
The recovered settings capacities and exact element strides are:

| Buffer | Elements | Stride |
| --- | --- | --- |
| Point lights | `max_point_lights` (256) | 208 bytes |
| Spotlights | `max_spot_lights` (32) | 352 bytes |
| Directional lights | `max_directional_lights` (16) | 112 bytes |
| Light probes | `max_light_probes` (128) | 96 bytes |
| Slice-zero light IDs | point + spot + probe capacities | 4 bytes |

Every descriptor uses flags `0x12`. The embedded name for the final buffer is
the original misspelled string `Slice Zero Ligth Ids`, which is retained for
binary provenance. Directional lights are deliberately absent from that
buffer's capacity sum, matching the executable.

The final call to `0x48AB30` rebuilds the shared spot-shadow depth texture
array. It first disables and releases the previous array, then reads the active
16-byte shadow configuration from the lighting system. The configuration
selects a layer count and one of seven exact square resolutions: 256, 512,
1024, 1600, 2048, 3200, or 4096 pixels.

Each layer receives an empty mip descriptor using the resolved
`{16, 12, 0, 1, -1}` depth format. The array uses the original depth creation
mode, resource flags, sampler defaults, and `Spot Shadow Depth TexArray` name.
The texture factory deep-copies those temporary descriptors before their
storage is released. The owner flag, configuration pointer/index, texture
slot, and five noncontiguous tiled-buffer slots are now modeled directly,
removing the tiled-light adapter.

The matching section of the lighting-system destructor at `0x480AD0` invokes
each buffer's virtual deleting destructor and clears all five owner slots in
the same order.

`RndBufferCollection::_AllocTiledLightingBuffers` at `0x6B3380` allocates two
light-ID buffers and one range buffer for each target. The number of ranges is
`ceil(width / tile_size) * ceil(height / tile_size) * depth_slices`. Range
records are 32 bytes. Light IDs are 16-bit values packed two per four-byte
buffer element, so each ID buffer contains half of
`range_count * max_lights_per_tile` elements. All three descriptors use flag
`1`.

Stereo mode creates a second set with the embedded `Both Eyes` names. The
optional interpolation target rounds the width up to an even value and halves
the height with upward rounding; its specialized render-target construction
remains behind an adapter while that descriptor type is recovered. The exact
resource slots occupy offsets `0x58` through `0x88` in the target-resource
block.

The matching section of the render-target resource teardown at `0x6AFFE0`
invokes each resource's virtual deleting destructor and clears all seven
slots. This includes both double-buffered light-ID pairs, both range buffers,
and the optional interpolation target.
