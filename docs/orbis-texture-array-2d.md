# Orbis 2D texture-array backend

`orbis_create_texture_array_2d` at `0x8D8A70` is virtual slot 28 of the Orbis
render system. It allocates a 392-byte object and passes the supplied texture
array descriptor to `orbis_texture_array_2d_construct` at `0x8E5D40`.

The platform constructor invokes the common 2D texture-array constructor at
`0x698160`, replaces the object vtable with the Orbis implementation, and
zeros the final 48 bytes of backend state. The common constructor copies the
descriptor elements into 80-byte texture records and stores their count. The
wrapper at `0x698100` assigns descriptor type 5, which the type-to-resource
map identifies as `RndTextureArray2DResource`.

Backend initialization at `0x8E5E70` branches on the common usage field. A
depth array creates a 52-byte Gnm depth-target descriptor, allocates aligned
depth, optional stencil, and HTILE surfaces, assigns their 256-byte addresses,
and creates a 32-byte shader texture view with resource memory type `109`.

The color path creates a 32-byte Gnm texture descriptor with texture type 13.
Its layer count comes from the number of 80-byte element records. After
allocating aligned tiled storage, it walks every layer and every mip, copying
each CPU image to the offset reported by Gnm. Flag bit 1 optionally adds a
64-byte render-target descriptor. That target owns any additional metadata
allocation recorded in the descriptor. The flag also selects resource memory
type `109`; textures without it use type `16`.

The final 48 bytes contain the texture view, three possible surface
allocations, the optional color target, and the depth target. Destruction at
`0x8E5D80` defers all GPU allocations, including color-target metadata,
releases the descriptors, and invokes the common array destructor. The
deleting destructor follows at `0x8E5E50`.

`analysis/exports/orbis-texture-array-2d-backend.asm` is the authoritative
initializer evidence because Hex-Rays does not currently produce pseudocode
for that function.

The six virtual methods at `0x8E64C0` through `0x8E657F` forward the color or
depth shader texture view and common sampler state to the shared stage-binding
layer.
