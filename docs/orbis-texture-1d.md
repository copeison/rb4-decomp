# Orbis 1D texture backend

`orbis_create_texture_1d` at `0x8D8980` is virtual slot 23 of the Orbis
render system. It allocates a 408-byte object and passes the supplied texture
descriptor to `orbis_texture_1d_construct` at `0x8E4F60`.

The platform constructor invokes the common 1D texture constructor at
`0x6F5870`, replaces the object vtable with the Orbis implementation, and
zeros the final 16 bytes of backend state. The wrapper at `0x6F5810` assigns
descriptor type 0 and dispatches through the matching factory slot.

Backend initialization at `0x8E5050` creates a 32-byte Gnm texture descriptor
with texture type 8, the authored width, unit height and depth, and the common
format, mip-count, tile-mode, and fragment settings. It calculates the tiled
size and alignment, allocates named GPU storage, and copies every available
CPU mip into its Gnm tiled destination. The descriptor then receives the
allocation's 256-byte base address and resource memory type `16`.

The final 16 bytes hold the Gnm texture descriptor pointer and its allocation.
Destruction at `0x8E4F90` defers the allocation through the Orbis render system,
releases the descriptor, clears its object field, and invokes the common 1D
texture destructor. The deleting destructor follows at `0x8E4FF0`.

The six virtual methods at `0x8E52D0` through `0x8E538F` forward the texture
view and common sampler state to the shared vertex, hull, domain, geometry,
pixel, and compute binding functions.
