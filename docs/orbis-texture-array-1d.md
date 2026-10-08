# Orbis 1D texture-array backend

`orbis_create_texture_array_1d` at `0x8D8A40` is virtual slot 27 of the Orbis
render system. It allocates a 360-byte object and passes the supplied texture
array descriptor to `orbis_texture_array_1d_construct` at `0x8E5870`.

The platform constructor invokes the common 1D texture-array constructor at
`0x696C00`, replaces the object vtable with the Orbis implementation, and
zeros the final 16 bytes of backend state. The wrapper at `0x696BA0` assigns
descriptor type 4, which the type-to-resource map identifies as
`RndTextureArray1DResource`.

Backend initialization at `0x8E5960` creates a 32-byte Gnm texture descriptor
with texture type 12. The descriptor uses the authored width, unit height and
depth, the number of 80-byte array-element records, and the common format,
mip-count, tile-mode, and fragment settings.

After calculating the tiled size and alignment, the function allocates named
GPU storage. It walks every array element and every mip in that element,
queries the corresponding Gnm tiled offset, and copies the CPU image into the
allocation. The descriptor then receives the allocation's 256-byte base
address and resource memory type `16`.

The final 16 bytes hold the Gnm texture descriptor pointer and its allocation.
Destruction at `0x8E58A0` defers the allocation through the Orbis render system,
releases the descriptor, clears its object field, and invokes the common array
destructor. The deleting destructor follows at `0x8E5900`.

The six virtual methods at `0x8E5C50` through `0x8E5D0F` forward the array
texture view and common sampler state to the shared stage-binding layer.
