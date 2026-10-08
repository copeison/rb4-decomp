# Tiled-light buffers

`render_tiled_light_buffers_initialize` at `0x48A400` creates the shared
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

The final call to `0x48AB30` continues initialization of the enclosing lighting
system. Its implementation remains behind a narrow adapter until that larger
owner layout is reconstructed.

The matching section of the lighting-system destructor at `0x480AD0` invokes
each buffer's virtual deleting destructor and clears all five owner slots in
the same order. The source models those noncontiguous fields through a typed
buffer-kind accessor until the complete enclosing layout is available.
