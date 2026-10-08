# Render-target resource-owner initialization

`render_target_resources_initialize` at `0x6B0760` first releases the owner's
old contents, derives its extent from the supplied `RenderTexture`, binds that
texture as the owner source, and registers it in the owner's resource list.
It then dispatches the owner-level resource groups in this order:

| Flag | Resource group |
| ---: | --- |
| `0x0008` | Primary and blurred light accumulation |
| `0x0010` | Light-probe accumulation fallback |
| `0x0080` | Sky chain |
| `0x0100` | Generic scaled target pairs |
| `0x0200` | Scene-mask targets |
| `0x0400` | Shadow-contribution resources |
| `0x2000` | CMAA targets |
| `0x4000` | Scene-mask tile targets and mesh |

The owner always contains one primary 216-byte per-scene block. Flag
`0x40000000` expands the block array to one plus
`max_partial_framerate_scenes` and initializes the added blocks as partial
frames. Matching blocks from the previous owner supply reusable resources.
The block array has four inline entries in the constructor; its storage and
growth remain behind a narrow container adapter.

After all resources are created, the owner propagates its mode field to every
registered resource. Flag `0x20000000` forces the 64-bit light-accumulation
format, while flag `0x80000000` marks the source texture as externally owned
during release.

`render_target_resources_release` at `0x6AFFE0` conditionally releases the
source texture, tears down every owner-level target group, and visits every
active per-scene block. The block pass releases partial-frame state, depth,
GBuffer, linear depth, ambient occlusion, tiled-light buffers, and volumetric
textures. It then clears the block count, extent, attachment cursor, and
registered-resource count.

The owner pointer at `0x188` and per-scene pointers at `0x18` and `0x20` are
confirmed virtual resources but do not yet have feature identities. Their
exact null-safe virtual teardown and slot clearing are retained behind narrow
unclassified-resource adapters.
