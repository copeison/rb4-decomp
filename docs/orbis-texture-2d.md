# Orbis 2D texture backend

`orbis_create_texture_2d` at `0x8D89B0` is virtual slot 24 of the Orbis render
system. It allocates a 520-byte object and passes the supplied texture
descriptor to `orbis_texture_2d_construct` at `0x8D62C0`.

The platform constructor first invokes the common 2D texture constructor at
`0x6900B0`, replaces the object vtable with the Orbis implementation, and
zeros its platform-specific backend state. The common resource registration
near that constructor identifies the class as `RndTexture2DResource` and
describes its `texture_path` property, providing direct class evidence for the
factory name.

The common object is exactly 408 bytes. It contains the 168-byte texture base,
a normalized 144-byte descriptor snapshot, one 80-byte mip-chain state, and an
optional linked-resource pointer with a signed index. Construction initializes
the link to `nullptr` and `-1`, publishes the mip properties through the base
texture state, and selects resource index zero. The Orbis subclass adds 112
bytes holding two color texture views, an auxiliary plane view, three memory
requirement records, the active storage bank, allocation ownership, two color
targets, one depth target, and auxiliary backend state. The eight bytes at
offset 480 are not initialized or read by the recovered methods and remain
reserved.

Backend initialization at `0x8D6460` selects a depth path when the common
texture usage field is 2. That path creates a 52-byte Gnm depth-target
descriptor, calculates aligned depth, stencil, and HTILE regions within one
shared allocation, and creates 32-byte shader texture views for the available
planes. The descriptors use resource memory type `109`.

The color path creates one or two 32-byte Gnm texture descriptors. Flag bit 0
enables the second storage bank, while flag bit `0x10` selects resource memory
type `16` instead of the usual `109`. It calculates the tiled size and
alignment, reuses a compatible texture's shared allocation when possible, and
uploads every CPU-side mip into each newly allocated bank. Flag combination
`(flags & 0x12) == 2` adds a 64-byte render-target descriptor for each bank.

The optional initializer argument is the compatible color texture whose
allocation may be shared. Depth initialization does not use it. The update
method at `0x8D6D10` switches the active bank and uploads every mip into that
bank using Gnm tiled offsets. This preserves the binary's double-buffered
dynamic-texture behavior without exposing descriptor-packet mechanics in the
clean reconstruction.

The render-target accessor at `0x8D71E0` selects the descriptor for the active
bank and falls back to the first descriptor if that slot is absent. The depth
accessor at `0x8D7240` returns the single depth-target descriptor.

Destruction at `0x8D6310` releases both color texture descriptors, the
secondary plane view, both render-target descriptors, the depth descriptor,
the auxiliary backend object, and the shared allocation control block before
invoking the common texture destructor. The deleting destructor follows at
`0x8D6440`. The reconstruction now performs this teardown directly through the
typed backend fields rather than hiding their order behind an opaque adapter.

`analysis/exports/orbis-texture-2d-backend.asm` preserves the complete backend
initializer because Hex-Rays does not currently produce pseudocode for that
function.

Common lifecycle and link-setter evidence is preserved in
`analysis/exports/render-texture-2d.asm` and
`analysis/exports/render-texture-2d.c`.

Virtual methods at `0x8D6E40` through `0x8D713F` bind the texture to all six
engine shader stages. Flag bit 2 selects the secondary depth/stencil view.
Color textures otherwise select the active storage bank; render-target-backed
textures use the active frame index and fall back to bank zero when that frame
has no target. Each wrapper forwards the selected view, address mode, filter,
flags, and border color to the shared Orbis texture-binding layer.
