# Orbis 3D texture backend

`orbis_create_texture_3d` at `0x8D89E0` is virtual slot 25 of the Orbis
render system. It allocates a 408-byte object and passes the supplied texture
descriptor to `orbis_texture_3d_construct` at `0x8E53C0`.

The platform constructor invokes the common 3D texture constructor at
`0x6F5CB0`, replaces the object vtable with the Orbis implementation, and
zeros the final 16 bytes of backend state. The wrapper at `0x6F5C50` assigns
descriptor type 2. The type-to-resource map at `0x6ACF40` identifies type 2
as `RndTexture3DResource`.

Backend initialization at `0x8E54B0` creates a 32-byte Gnm texture descriptor
with texture type 10. It carries the common width, height, depth, mip count,
format, tile mode, and fragment count into the descriptor and calculates its
tiled size and alignment.

The initializer may reuse the allocation pointer at offset 400 from a
compatible texture supplied by the caller. Otherwise it allocates GPU memory
using the texture's alignment field. When CPU mip data is present, every mip
is copied into its tiled destination before the descriptor receives its base
address. Flag bit 1 selects resource memory type `109`; textures without that
flag use type `16`.

The final 16 bytes hold the Gnm texture descriptor pointer and its allocation.
Destruction at `0x8E53F0` defers the allocation through the Orbis render system,
releases the descriptor, clears its object field, and invokes the common 3D
texture destructor. The deleting destructor follows at `0x8E5450`.

The six virtual methods at `0x8E5770` through `0x8E582F` forward the texture
view and common sampler state to the shared vertex, hull, domain, geometry,
pixel, and compute binding functions.
