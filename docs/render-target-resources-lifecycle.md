# Render-target resource-owner initialization

`render_target_resources_construct` at `0x6AFEA0` initializes the exact
1,552-byte owner layout. The owner embeds storage for 38 registered-resource
pointers and four 216-byte scene blocks; construction points both containers
at their inline storage, sets their capacities, selects primary block zero,
and initializes the active scene context to `-1`.
`render_target_resources_destruct` at `0x6AFFC0` restores the base dispatch
table before running the common release path.

The 32-byte `RenderTargetState` header is the prefix of this owner. Its state
flags map to owner flags, its reserved word maps to resource mode, draw/debug
values occupy the owner's reserved 64-bit field, and width/height map to the
owner extent. The concrete state constructor at `0x6B40A0` runs the common
owner constructor and installs the concrete dispatch table. The deleting
destructor at `0x6B40E0` runs common teardown before freeing the 1,552-byte
allocation. The common `RenderTarget` lifecycle now calls these recovered
paths directly.

The base and concrete five-entry dispatch tables are now represented directly.
Both provide normal and deleting destructors. The concrete table additionally
accepts only textures whose virtual descriptor type is `1`, then exposes the
reconstructed 2D and 2D-array creation methods. Construction and destruction
install the appropriate table without external dispatch adapters.

Code that consumes the verified layout now reads stable owner fields directly:
flags, extent, attachment cursor, source texture, block storage and count,
resource mode, active block, scene context, and every identified target or
texture slot. Small local enum-to-field helpers keep multi-target subsystem
code readable. Adapters remain only where an operation still performs
container growth, registration, allocation, or virtual dispatch.

Source binding now mirrors the inlined code at `0x6B0760`: copy the source
texture dimensions, invoke the owner's compatibility virtual, store the
source, and append it to the inline resource list. Common release directly
tears down the three still-unidentified target slots and resets block count,
extent, attachment cursor, and registered-resource count. Stereo target
selection is the observed `resource_mode == 3` test.

The block container is the binary's fixed inline array: growing value-clears
new 216-byte records, shrinking only changes the count, and the constructor's
capacity is four. Mode propagation walks registered pointers and writes the
common resource-mode field at offset `0xA0`. The tiled-light view is now a
verified typed overlay on the scene block rather than an external offset
adapter.

Shadow contribution uses the common 2D-array factory for its
`{8, 10, 0, 1, -1}` contribution layers. At resolutions above 1920x1080 it
halves the working extent and adds a `{24, 11, 0, 1, -1}` stencil texture with
creation-state type `2` and target flag `16`, plus a second scratch texture.
Scratch textures use `{64, 4, 2, 1, -1}` and the two soften-tile textures use
`{8, 10, 0, 1, -1}` at the rounded tile extent. The 2D resources use filter
value `1`; every created texture is registered with the owner.

The primary per-scene tiled-light block also creates an interpolation texture
at an even-width, half-height extent. Its direct descriptor uses creation
values six and eight set to `1`, value nine set to `2`, value ten set to `10`,
and format `{64, 4, 2, 1, -1}`. Reuse prefers the prior block's interpolation
texture, then the owner's primary shadow-contribution scratch texture when it
is large enough for the requested extent.

Volumetric scattering now reaches the common 3D factory directly. Mono,
stereo, and accumulated textures use resource-kind `5` defaults, creation
value ten `2`, and format `{64, 4, 2, 1, -1}`. Accumulated textures provide
the recovered alternating half-float voxel pattern as source data; the common
texture constructor copies that temporary data before its local allocation is
released. Existing depth-by-depth reuse behavior is preserved.

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
The block array has four inline entries in the constructor; its growth remains
behind a narrow container adapter.

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

Three adjacent owner helpers complete the active-block controls:

- `render_target_resources_set_resource_mode` at `0x6B28D0` updates the owner
  mode only when it changes, then propagates it to registered resources.
- `render_target_resources_acquire_partial_frame_state` at `0x6B2910` maps a
  partial-scene index to block `index + 1`, grows and initializes the block
  array as needed, and returns the block's 80-byte state. That state is now a
  typed allocation with the exact constructor defaults recovered from
  `0x6D18A0`; block teardown releases it directly.
- `render_target_resources_select_partial_frame` at `0x6B2A20` stores the
  active block index and scene context. Passing `-1, -1` selects the primary
  block and clears the context.

The owner's remaining creation virtuals ultimately use common render-factory
slots `+0x28` and `+0x48` for 2D and 2D-array textures. Those slots now have
typed common dispatch wrappers, matching the already reconstructed Orbis
factory vtable and providing the final platform-neutral handoff needed by the
owner's descriptor-building routines at `0x6B4120` and `0x6B41F0`.

The 2D path proved that a single-texture descriptor contains a complete
80-byte owned mip-chain descriptor after its 144-byte texture state. The 1D,
2D, and 3D declarations now use the verified 224-byte size rather than the
earlier 168-byte prefix view. The mip descriptor's implementation pointer,
width, height, depth, and data-format offsets are typed; the remaining 56-byte
ownership state stays opaque until its resource variants are reconstructed.

The virtuals return render textures directly: normal targets are 2D textures,
and the shadow-contribution array is a 2D-array texture. Owner and per-scene
slots therefore use the common `RenderTexture` base, including release and
reuse paths. The separate 32-byte `RenderTarget` wrapper remains limited to
its own active-state lifecycle.

The shared texture fields at `+0x90` and `+0x94` are the attachment index and
attachment count for these resources. Light accumulation and depth/stencil
creation now update the owner's attachment cursor directly from those fields;
an index of `-1` continues to mean that no attachment was assigned.

The concrete creation virtuals at `0x6B4120` and `0x6B41F0` are reconstructed
in `render_target_resource_factory.cpp`. Both copy the caller's 44-byte
creation state into a common descriptor, resolve its defaults, construct
80-byte mip descriptors with depth one, dispatch through the shared render
factory, and invoke the texture backend initializer. The array path allocates
one mip descriptor per layer and releases that temporary range after the
factory copies it. The common initializer at `0x69B7A0` preserves texture
reuse for platform backends and performs the source-free GPU update through
virtual slots `+0x78` and `+0x68`.

The shared light-accumulation factory also uses the common path directly. Its
32- and 64-bit variants select `{32, 2, 2, 1, -1}` and
`{64, 4, 2, 1, -1}` format descriptors. The address- and filter-mode lookups
at `0x50CE00` and `0x50CE30` are reconstructed as their exact resource-kind
bit tables, so kind `18` no longer depends on adapter-provided defaults.

Sky target creation now calls this factory directly. Its four levels share the
verified `{32, 4, 1, 1, -1}` data-format descriptor and common creation-state
defaults; only creation-state value nine changes from `1` for the full target
to `2` for reduced targets. This removes the sky-specific creation adapter
while preserving the original first-owner and prior-owner reuse rules.

The fallback light-probe accumulation texture is allocated only when tiled
lighting is disabled. Its direct descriptor uses creation-state values six,
eight, and nine set to `1`, value ten set to `10`, and the verified
`{64, 4, 2, 1, -1}` format descriptor. The resulting full-resolution texture
is registered with the owner and can reuse the matching texture from the
previous owner.

The three scene-mask textures share the verified `{8, 10, 0, 1, -1}` format
and resource-kind `32` address/filter defaults. The mask and scratch textures
use the owner's full extent; the mask-tile texture rounds each dimension up by
the configured mask tile size and overrides creation-state filter value nine
to `1`. All three use the common 2D factory, register with the owner, and
accept matching reusable textures.

The separate pair used by the scene-mask grid shares the same
`{8, 10, 0, 1, -1}` format at the light-tile extent. Both textures use fixed
address and filter values of `1`; the primary carries target flag `4`, while
the secondary carries zero. Both textures are owner-registered after direct
common 2D creation.

CMAA creates its optional color texture only when the previous owner supplies
a reusable one. That color uses the same 32/64-bit format selection as light
accumulation and creation-state filter value `2`. Its two edge textures use
`{8, 10, 0, 1, -1}`, while the half-resolution compressed edge texture uses
`{32, 4, 3, 1, -1}`; those three use filter value `1`. Every created texture
is registered directly, leaving only the renderer capability query behind an
adapter.
