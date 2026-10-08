# Orbis cube texture-array backend

`orbis_create_texture_array_cube` at `0x8D8AA0` is virtual slot 29 of the
Orbis render system. It allocates a 360-byte object and passes the supplied
descriptor to `orbis_texture_array_cube_construct` at `0x8E6640`.

The platform constructor invokes the common cube texture-array constructor at
`0x69AAC0`, replaces the object vtable with the Orbis implementation, and
zeros the final 16 bytes of backend state. The common constructor copies each
six-face, 480-byte cube descriptor and stores the cube count. The wrapper at
`0x69AA60` assigns descriptor type 7 and dispatches through the matching
factory slot.

The common cube-array object is exactly 344 bytes. It contains the 168-byte
texture base, a normalized 144-byte descriptor snapshot, and a 32-byte vector
of 480-byte cube states. Each cube is represented as six typed 80-byte
mip-chain faces. Construction validates the normalized cube records, publishes
the first face's shared mip properties, and records the cube count. The Orbis
subclass adds only its texture-view and allocation pointers.

Backend initialization at `0x8E6730` creates a 32-byte Gnm texture descriptor
with texture type 11. Its array-slice count is six times the number of
480-byte cube records, while width, height, format, mip count, tile mode, and
fragment count come from the common descriptor and first face.

After calculating the tiled size and alignment, the function allocates named
GPU storage. It visits every cube, each of its six 80-byte face records, and
every mip in each face. The flattened Gnm array slice is `cube * 6 + face`.
Each CPU image is copied to the tiled offset for that slice and mip before the
descriptor receives its 256-byte base address and resource memory type `16`.

The final 16 bytes hold the Gnm texture descriptor pointer and its allocation.
Destruction at `0x8E6670` defers the allocation through the Orbis render
system, releases the descriptor, clears its object field, and invokes the
common cube-array destructor. The deleting destructor follows at `0x8E66D0`.

The six virtual methods at `0x8E6AB0` through `0x8E6B6F` forward the flattened
cube-array texture view and common sampler state to the shared stage-binding
layer.

Common lifecycle evidence is preserved in
`analysis/exports/render-texture-array-cube.asm` and
`analysis/exports/render-texture-array-cube.c`.
